/*
 This file is part of moep80211
 (C) 2011-2016 Stephan M. Guenther (and other contributing authors)

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

#include "args.h"
#include "attack.h"
#include "cell.h"
#include "cfg.h"
#include "daemonize.h"
#include "deauth.h"
#include "defender.h"
#include "frametypes.h"
#include "global.h"
#include "helper.h"
#include "state.h"
#include "whitelist.h"


static volatile int _run = 1;
static int sfd = -1;
static int do_attack = 0;

extern struct argp argp;
extern struct cfg cfg;

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
	case SIGUSR1:
		do_attack = !do_attack;
		LOG(LOG_INFO, "attacking switched %s", do_attack ? "on" : "off");
		break;

	default:
		LOG(LOG_WARNING,"signal_handler(): unknown signal %d", sig);
		break;
	}
}

int
rad_tx(moep_frame_t f)
{
	int ret;

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

	if (0 > timeout_create(CLOCK_MONOTONIC, &logt, state_log_cb, NULL))
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



static void
radh(moep_dev_t dev, moep_frame_t frame)
{
	(void)dev;
	struct ieee80211_hdr_gen *hdr;
	struct moep80211_radiotap *rt;
	size_t len;
	u8 *payload = NULL;
	u8 hwaddr[] = {[0 ... IEEE80211_ALEN-1] = 0};
	u8 bssid[] = {[0 ... IEEE80211_ALEN-1] = 0};
	char essid[IEEE80211_MAX_SSID_LEN+1];
	int ret;
	cell_t cell;
	sta_t sta;

	if (!(rt = moep_frame_radiotap(frame))) {
		LOG(LOG_ERR, "moep_frame_radiotap() failed");
		goto end;
	}
	if (!(rt->hdr.it_present & BIT(IEEE80211_RADIOTAP_RX_FLAGS)))
		goto end; // driver echo frame
	if (!(hdr = moep_frame_ieee80211_hdr(frame))) {
		LOG(LOG_ERR, "moep_frame_ieee80211_hdr() failed");
		goto end;
	}
	if (!(payload = moep_frame_get_payload(frame, &len))) {
		LOG(LOG_ERR, "moep_frame_get_payload() failed");
		goto end;
	}

	if (0 > get_bssid(bssid, hdr)) {
		goto end;
	}

	if (!(cell = cell_find(bssid)))
		cell = cell_add(bssid);
	cell_update_timestamp(cell);

	if (ieee80211_is_beacon(hdr->frame_control)) {
		ret = get_essid(essid, sizeof(essid),
				(const struct ieee80211_beacon *)payload, len);
		if (0 > ret)
			goto end;
		cell_update_essid(cell, essid);

		ret = get_encryption(
				(const struct ieee80211_beacon *)payload, len);
		if (0 > ret)
			goto end;
		cell->ciphers = ret;
		cell->signal = rt->signal;

	}
	else if (ieee80211_is_data(hdr->frame_control)) {
		if (0 > get_sta_hwaddr(hwaddr, hdr))
			goto end;
		if (!(sta = sta_find(&cell->sl, hwaddr)))
			sta = sta_add(&cell->sl, hwaddr);
		sta_update(sta);
		if (ieee80211_has_protected(hdr->frame_control))
			sta->encrypted = 1;
		sta->signal = rt->signal;
	}

	if (whitelist_check(&cfg.whitelist.cell, bssid))
		goto end;
	if (whitelist_check(&cfg.whitelist.sta, hwaddr))
		goto end;

	if (do_attack)
		attack(frame);

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

	cfg.radio.freq0		= 5180;
	cfg.radio.freq1		= 0;
	cfg.radio.moep_chan_width= MOEP80211_CHAN_WIDTH_20;
	cfg.radio.rt.it_present	= BIT(IEEE80211_RADIOTAP_MCS)
				| BIT(IEEE80211_RADIOTAP_TX_FLAGS);
	cfg.radio.rt.mcs.known	= IEEE80211_RADIOTAP_MCS_HAVE_MCS
				| IEEE80211_RADIOTAP_MCS_HAVE_BW;
	cfg.radio.rt.mcs.mcs	= 0;
	cfg.radio.rt.mcs.flags	= IEEE80211_RADIOTAP_MCS_BW_20;

	strncpy(cfg.whitelist.filename, DEFAULT_WHITELIST,
		sizeof(cfg.whitelist.filename));
}

static int
check_timer_resolution()
{
	struct timespec ts;
	u64 res;

	clock_getres(CLOCK_MONOTONIC, &ts);
	res = ts.tv_sec*1000*1000 + ts.tv_nsec/1000;

	if (!res) {
		LOG(LOG_INFO, "timer resolution is %lu nsec [OK]", ts.tv_nsec);
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
	(void) signal(SIGUSR1, signal_handler);

	LOG(LOG_INFO,"defender starting...");

	(void) check_timer_resolution();
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
					cfg.radio.freq0,
					cfg.radio.moep_chan_width,
					cfg.radio.freq1, 0,
					cfg.rad.mtu))) {
		LOG(LOG_ERR,"moep80211_rad_open() failed: %s",
		    strerror(errno));
		return -1;
	}

	moep_dev_set_rx_handler(cfg.rad.dev, radh);
	cfg.rad.rx_rdy = eventfd(1, EFD_SEMAPHORE | EFD_CLOEXEC);
	moep_dev_set_rx_event(cfg.rad.dev, cfg.rad.rx_rdy);

	whitelist_load(&cfg.whitelist);
	whitelist_print(stdout, &cfg.whitelist);

	ret = run();
	whitelist_destroy(&cfg.whitelist);

	moep_dev_close(cfg.rad.dev);

	return ret;
}

