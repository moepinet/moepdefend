/*
 This file is part of moep80211
 (C) 2011-2013 Stephan M. Guenther (and other contributing authors)

 moep80211 is free software; you can redistribute it and/or modify it under the
 terms of the GNU General Public License as published by the Free Software
 Foundation; either version 3, or (at your option) any later version.

 moep80211 is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with
 moep80211; see the file COPYING.  If not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#define _GNU_SOURCE

#include <argp.h>
#include <sys/eventfd.h>
#include <sys/signalfd.h>

#include <moep80211/system.h>
#include <moep80211/modules/ieee80211.h>

#include <moepcommon/timeout.h>

#include "global.h"
#include "daemonize.h"
#include "cell.h"
#include "frametypes.h"
#include "whitelist.h"


static int _run = 1;
static int sfd = -1;

const char *argp_program_version = "defender 0.1";
const char *argp_program_bug_address = "<moepi@moepi.net>";

static char args_doc[] = "IF FREQ";

static int send_beacon(timeout_t t, u32 overrun, void *data);
static int rad_tx(moep_frame_t f);

/*
 * Argument parsing
 * ---------------------------------------------------------------------------
 */
static char doc[] =
"defender - the moep80211 network defender\n\n"
"  IF                         Use the radio interface with name IF\n"
"  FREQ                       Use the frequency FREQ [in MHz] for the radio\n"
"                             interface\n";

enum fix_args {
	FIX_ARG_IF   = 0,
	FIX_ARG_FREQ = 1,
	FIX_ARG_CNT
};

static struct argp_option options[] = {
	{
		.name	= "whitelist",
		.key	= 'w',
		.arg	= "WLIST",
		.flags	= 0,
		.doc	= "Load whitelist WLIST"
	},
	{NULL}
};

static error_t
parse_opt(int key, char *arg, struct argp_state *state);

static struct argp argp = {
	.options	= options,
	.parser		= parse_opt,
	.args_doc	= args_doc,
	.doc		= doc
};

static struct cfg {
	int daemon;
	struct {
		u64	freq0;
		u64	freq1;
		u64	moep_chan_width;
		struct {
			u32	it_present;
			u8	rate;
			struct {
				u8	known;
				u8	flags;
				u8	mcs;
			} mcs;
		} rt;
	} wlan;
	struct {
		char	*name;
		size_t	mtu;
		moep_dev_t dev;
		int	tx_rdy;
		int	rx_rdy;
	} rad;
	struct whitelist whitelist;
} cfg;

static error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
	struct cfg *cfg = state->input;
	long long int freq;
	char *endptr = NULL;

	switch (key) {
	case 'w':
		strncpy(cfg->whitelist.filename, arg,
					sizeof(cfg->whitelist.filename));
		break;
	case ARGP_KEY_ARG:
		switch (state->arg_num) {
		case FIX_ARG_IF:
			cfg->rad.name = arg;
			break;
		case FIX_ARG_FREQ:
			freq = strtoll(arg, &endptr, 0);
			if (freq < 0)
				argp_failure(state, 1, errno,
					"Invalid frequency: %lld", freq);
			cfg->wlan.freq0 = freq;
			cfg->wlan.freq1 += freq;
			break;
		default:
			argp_usage(state);
		}
		break;
	case ARGP_KEY_END:
		if (state->arg_num < FIX_ARG_CNT)
			argp_usage(state);
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

/*
 * ---------------------------------------------------------------------------
 * /Argument parsing
 */


static void
signal_handler(int sig)
{
	LOG(LOG_INFO, "received signal %d", sig);

	switch (sig)
	{
	case SIGINT:
	case SIGTERM:
		_run = 0;
		break;

	default:
		LOG(LOG_WARNING,"signal_handler(): unknown signal %d", sig);
		break;
	}
}


moep_frame_t create_rad_frame()
{
	return moep_dev_frame_create(cfg.rad.dev);
}

static int
log_status(timeout_t t, u32 overrun, void *data)
{
	(void) data;
	(void) t;
	(void) overrun;
	cell_t cell;
	sta_t sta;
	struct timespec inactive;
	FILE *file;

	if (!(file = fopen(DEFAULT_LOGFILE, "w"))) {
		LOG(LOG_ERR, "fopen() failed: %s", strerror(errno));
		return -1;
	}

	list_for_each_entry(cell, &cl, list) {
		if (cell_inactive(cell, &inactive)) {
			LOG(LOG_ERR, "cell_inactive() failed: %s",
				strerror(errno));
			continue;
		}
		fprintf(file, "Cell %s [inactive since %lds]\n",
			ether_ntoa((const struct ether_addr *)cell->bssid),
			inactive.tv_sec);
		fprintf(file, "  ESSID: %s\n", cell->essid);
		list_for_each_entry(sta, &cell->sl, list) {
			if (sta_inactive(sta, &inactive)) {
				LOG(LOG_ERR, "sta_inactive() failed: %s",
					strerror(errno));
				continue;
			}
			fprintf(file, "  STA %s [inactive since %lds]\n",
				ether_ntoa((const struct ether_addr *)sta->hwaddr),
				inactive.tv_sec);
		}
	}

	fclose(file);
	return 0;
}

static void
ieee80211_frame_init_l1hdr(moep_frame_t frame)
{
	struct moep80211_radiotap *rt;

	rt = moep_frame_radiotap(frame);

	rt->hdr.it_present = cfg.wlan.rt.it_present;
	rt->rate = cfg.wlan.rt.rate;
	rt->mcs.known = cfg.wlan.rt.mcs.known;
	rt->mcs.flags = cfg.wlan.rt.mcs.flags;
	rt->mcs.mcs = cfg.wlan.rt.mcs.mcs;

	rt->hdr.it_present |= BIT(IEEE80211_RADIOTAP_TX_FLAGS);
	rt->tx_flags = IEEE80211_RADIOTAP_F_TX_NOACK;
}

static void
ieee80211_frame_init_l2hdr(moep_frame_t frame)
{
	struct ieee80211_hdr_gen *hdr;

	hdr = moep_frame_ieee80211_hdr(frame);
	hdr->frame_control =
		htole16(IEEE80211_FTYPE_DATA | IEEE80211_STYPE_DATA);
}

int
rad_tx(moep_frame_t f)
{
	int ret;

	ieee80211_frame_init_l1hdr(f);
	ieee80211_frame_init_l2hdr(f);

	if (0 > (ret = moep_dev_tx(cfg.rad.dev, f)))
		LOG(LOG_ERR, "moep80211_tx() failed: %s", strerror(errno));

	return ret;
}

static int
run()
{
	int ret, maxfd;
	timeout_t logt;
	struct signalfd_siginfo siginfo;
	fd_set rfds, rfd;
	sigset_t sigset, oldset, blockset, emptyset;

	sigemptyset(&blockset);
	sigaddset(&blockset, SIGRTMIN);
	sigaddset(&blockset, SIGRTMIN+1);
	if (0 > sigprocmask(SIG_BLOCK, &blockset, NULL)) {
		moep_dev_close(cfg.rad.dev);
		DIE("sigprocmask() failed: %s", strerror(errno));
	}
	if (0 > (sfd = signalfd(-1, &blockset, SFD_CLOEXEC | SFD_NONBLOCK)))
		DIE("signalfd() failed: %s", strerror(errno));

	sigfillset(&sigset);
	if (0 > sigprocmask(SIG_SETMASK, &sigset, &oldset)) {
		moep_dev_close(cfg.rad.dev);
		DIE("sigprocmask() failed: %s", strerror(errno));
	}

	if (0 > timeout_create(CLOCK_MONOTONIC, &logt, log_status, NULL))
		DIE("timeout_create() failed: %s", strerror(errno));
	timeout_settime(logt, 0, timeout_msec(LOG_INTERVAL,LOG_INTERVAL));

	sigemptyset(&emptyset);
	sigaddset(&emptyset, SIGRTMIN);

	FD_ZERO(&rfds);
	FD_SET(sfd, &rfds);
	maxfd = sfd;

	LOG(LOG_INFO,"defender startup complete");
	while (_run) {
		rfd = rfds;

		ret = moep_select(maxfd+1, &rfd, NULL, NULL, NULL, &oldset);

		if (0 > ret) {
			if (errno == EINTR)
				continue;
			DIE("pselect() failed: %s", strerror(errno));
		}

		if (!FD_ISSET(sfd, &rfd))
			continue;

		for (;;) {
			ret = read(sfd, &siginfo, sizeof(siginfo));
			if (0 > ret) {
				if (errno == EAGAIN || errno == EWOULDBLOCK)
					break;
			}
			if (0 > ret)
				DIE("read() failed: %s", strerror(errno));

			if (SIGRTMIN+1 == (int)siginfo.ssi_signo)
				continue;

			if (SIGRTMIN != (int)siginfo.ssi_signo
				|| SI_TIMER != siginfo.ssi_code) {
				signal_handler(siginfo.ssi_signo);
				continue;
			}
			ret = timeout_exec((void *)siginfo.ssi_ptr,
						siginfo.ssi_overrun);
			if (0 > ret) {
				LOG(LOG_ERR, "timeout_exec() failed: %d", ret);
				continue;
			}
		}
	}

	timeout_delete(logt);

	sigprocmask(SIG_SETMASK, &oldset, NULL);
	return _run;
}

static u8 *
get_transmitter(struct ieee80211_hdr_gen *hdr)
{
	return hdr->addr2;
}

static u8 *
get_receiver(struct ieee80211_hdr_gen *hdr)
{
	return hdr->addr1;
}

static u8 *
get_bssid(struct ieee80211_hdr_gen *hdr)
{
	int fromds, tods;

	fromds = hdr->frame_control & IEEE80211_FCTL_FROMDS;
	tods = hdr->frame_control & IEEE80211_FCTL_TODS;

	if (fromds & tods)
		return NULL;
	if (fromds)
		return hdr->addr2;
	if (tods)
		return hdr->addr1;
	return hdr->addr3;
}

static char *
get_essid(const struct ieee80211_beacon * bcn, ssize_t len)
{
	size_t elen;
	int ptr;
	static char essid[IEEE80211_MAX_SSID_LEN+1];

	memset(essid, 0, sizeof(essid));

	if (!bcn) {
		LOG(LOG_ERR, "moep_frame_get_payload() failed: %s",
			strerror(errno));
		return NULL;
	}

	for (ptr=0; ptr<len-2; ptr+=bcn->variable[ptr+1]) {
		if (bcn->variable[ptr] != WLAN_EID_SSID)
			continue;
		elen = min((int)bcn->variable[ptr+1]+1, IEEE80211_MAX_SSID_LEN+1);
		if (elen > 0)
			snprintf(essid, elen, "%s", &bcn->variable[ptr+2]);
		break;
	}

	return essid;
}

static u8 *
get_sta_hwaddr(const struct ieee80211_hdr_gen *hdr)
{
	static u8 hwaddr[IEEE80211_ALEN];

	if (ieee80211_has_tods(hdr->frame_control))
		memcpy(hwaddr, hdr->addr2, IEEE80211_ALEN);
	else if (ieee80211_has_fromds(hdr->frame_control))
		memcpy(hwaddr, hdr->addr1, IEEE80211_ALEN);
	else
		return NULL;

	if (!is_unicast_mac(hwaddr))
		return NULL;

	return hwaddr;
}

static void
radh(moep_dev_t dev, moep_frame_t frame)
{
	(void)dev;
	struct ieee80211_hdr_gen *hdr;
	size_t len;
	u8 *payload, *hwaddr;
	char *essid;
	u8 *transmitter, *receiver, *bssid;
	cell_t cell;
	sta_t sta;

	if (!(hdr = moep_frame_ieee80211_hdr(frame))) {
		LOG(LOG_ERR, "moep_frame_ieee80211_hdr() failed");
		goto end;
	}
	if (!(payload = moep_frame_get_payload(frame, &len))) {
		LOG(LOG_ERR, "moep_frame_get_payload() failed");
		goto end;
	}

	transmitter = get_transmitter(hdr);
	receiver = get_receiver(hdr);
	bssid = get_bssid(hdr);

	if (!is_unicast_mac(bssid))
		goto end;

	if (!bssid)
		goto end;

	if (!(cell = cell_find(bssid)))
		cell = cell_add(bssid);
	cell_update_timestamp(cell);

	if (ieee80211_is_beacon(hdr->frame_control)) {
		essid = get_essid((const struct ieee80211_beacon *)payload, len);
		if (!essid)
			goto end;
		cell_update_essid(cell, essid);
	}
	else if (ieee80211_is_data(hdr->frame_control)) {
		hwaddr = get_sta_hwaddr(hdr);
		if (!hwaddr)
			goto end;
		if (!(sta = sta_find(&cell->sl, hwaddr)))
			sta = sta_add(&cell->sl, hwaddr);
		sta_update(sta);
	}

	end:
	moep_frame_destroy(frame);
	return;
}

void
cfg_init()
{
	memset(&cfg, 0, sizeof(cfg));

	cfg.daemon		= 0;

	cfg.rad.name		= "wlan0";
	cfg.rad.mtu		= 1500;

	cfg.wlan.freq0		= 5180;
	cfg.wlan.freq1		= 0;
	cfg.wlan.moep_chan_width= MOEP80211_CHAN_WIDTH_20;
	cfg.wlan.rt.it_present	= BIT(IEEE80211_RADIOTAP_MCS)
				| BIT(IEEE80211_RADIOTAP_TX_FLAGS);
	cfg.wlan.rt.mcs.known	= IEEE80211_RADIOTAP_MCS_HAVE_MCS
				| IEEE80211_RADIOTAP_MCS_HAVE_BW;
	cfg.wlan.rt.mcs.mcs	= 0;
	cfg.wlan.rt.mcs.flags	= IEEE80211_RADIOTAP_MCS_BW_20;

	strncpy(cfg.whitelist.filename, DEFAULT_WHITELIST,
		sizeof(cfg.whitelist.filename));
}

static int
check_timer_resoluton()
{
	struct timespec ts;
	u64 res;

	clock_getres(CLOCK_MONOTONIC, &ts);
	res = ts.tv_sec*1000*1000 + ts.tv_nsec/1000;

	if (!res) {
		LOG(LOG_INFO, "timer resultion is %lu nsec [OK]", ts.tv_nsec);
		return 0;
	}

	LOG(LOG_WARNING, "timer resultion is %ld usec which may "\
		"cause problem - fix your timers", res);
	return -1;
}


int
main(int argc, char **argv)
{
	int ret;

	(void) signal(SIGTERM, signal_handler);
	(void) signal(SIGINT, signal_handler);

	LOG(LOG_INFO,"defender starting...");

	(void) check_timer_resoluton();
	cfg_init();

	argp_parse(&argp, argc, argv, 0, 0, &cfg);

	if (cfg.daemon) {
		daemonize();
	}
	else {
		openlog("moep80211ncm", LOG_PID | LOG_PERROR, LOG_USER);
		setlogmask(LOG_UPTO(LOG_DEBUG));
	}

	if (!(cfg.rad.dev = moep_dev_ieee80211_open(cfg.rad.name,
					cfg.wlan.freq0,
					cfg.wlan.moep_chan_width,
					cfg.wlan.freq1, 0,
					cfg.rad.mtu))) {
		LOG(LOG_ERR,"moep80211_rad_open() failed: %s",
		    strerror(errno));
		return -1;
	}

	moep_dev_set_rx_handler(cfg.rad.dev, radh);
	cfg.rad.rx_rdy = eventfd(1, EFD_SEMAPHORE | EFD_CLOEXEC);
	moep_dev_set_rx_event(cfg.rad.dev, cfg.rad.rx_rdy);

	whitelist_load(&cfg.whitelist);
	ret = run();
	whitelist_destroy(&cfg.whitelist);

	moep_dev_close(cfg.rad.dev);

	return ret;
}

