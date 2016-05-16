#include <string.h>
#include <argp.h>

#include <moep80211/system.h>
#include <moep80211/modules/ieee80211.h>

#include "global.h"
#include "args.h"
#include "cfg.h"

const char *argp_program_version = "defender 0.1";
const char *argp_program_bug_address = "<moepi@moepi.net>";

static char args_doc[] = "IF FREQ";

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
//	{
//		.name   = "daemonize",
//		.key	= 'd',
//		.arg    = "DAEMON",
//		.flags  = 0,
//		.doc	= "daemonize"
//	},
	{
		.name   = "defmode",
		.key	= 'm',
		.arg    = "DEFMODE",
		.flags  = 0,
		.doc	= "defense on"
	},
	{
		.name	= "whitelist",
		.key	= 'w',
		.arg	= "WLIST",
		.flags	= 0,
		.doc	= "load whitelist WLIST"
	},
	{
		.name   = "rate",
		.key	= 'r',
		.arg    = "RATE | MCS",
		.flags  = 0,
		.doc	= "set legacy RATE [r*500kbit/s] or MCS index"
	},
	{
		.name   = "ht",
		.key	= 'h',
		.arg    = "HT",
		.flags  = 0,
		.doc	= "set HT channel width"
	},
	{
		.name   = "gi",
		.key	= 'g',
		.arg    = "GI",
		.flags  = 0,
		.doc	= "set GI"
	},
	{NULL}
};

struct argp argp = {
	.options	= options,
	.parser		= parse_opt,
	.args_doc	= args_doc,
	.doc		= doc
};

struct cfg cfg;

error_t
parse_opt(int key, char *arg, struct argp_state *state)
{
	struct cfg *cfg = state->input;
	long long int freq;
	char *endptr = NULL;

	switch (key) {
	case 'd':
		cfg->daemon = 1;
		break;
	case 'm':
		cfg->defmode = atoi(arg);
		if (cfg->defmode < 0 || cfg->defmode > ATTACK_COUNT)
			argp_failure(state, 1, errno,
				"Invalid attack mode: %d", cfg->defmode);
		break;
	case 'w':
		strncpy(cfg->whitelist.filename, arg,
					sizeof(cfg->whitelist.filename));
		break;
	case 'r':
		if (cfg->radio.rt.it_present & BIT(IEEE80211_RADIOTAP_MCS)) {
			cfg->radio.rt.mcs.known |=
				IEEE80211_RADIOTAP_MCS_HAVE_MCS;
			cfg->radio.rt.mcs.mcs = atoi(arg);
		}
		else {
			cfg->radio.rt.it_present |=
				BIT(IEEE80211_RADIOTAP_RATE);
			cfg->radio.rt.rate = atoi(arg);
		}
		break;
	case 'h':
		if (cfg->radio.rt.it_present & BIT(IEEE80211_RADIOTAP_RATE)) {
			cfg->radio.rt.it_present &=
				~BIT(IEEE80211_RADIOTAP_RATE);
			cfg->radio.rt.mcs.known |=
				IEEE80211_RADIOTAP_MCS_HAVE_MCS;
			cfg->radio.rt.mcs.mcs = cfg->radio.rt.rate;
			cfg->radio.rt.rate = 0;
		}
		cfg->radio.rt.it_present |= BIT(IEEE80211_RADIOTAP_MCS);
		cfg->radio.rt.mcs.known |= IEEE80211_RADIOTAP_MCS_HAVE_BW;
		if (0 == strncasecmp(arg, "ht20", strlen(arg))) {
			cfg->radio.rt.mcs.flags |= IEEE80211_RADIOTAP_MCS_BW_20;
			cfg->radio.moep_chan_width = MOEP80211_CHAN_WIDTH_20;
			break;
		}

		if (strlen(arg) != strlen("ht40*"))
			argp_failure(state, 1, errno,
					"Invalid HT bandwidth: %s", arg);

		if (0 == strncasecmp(arg, "ht40+", strlen(arg))) {
			cfg->radio.rt.mcs.flags |= IEEE80211_RADIOTAP_MCS_BW_40;
			cfg->radio.moep_chan_width = MOEP80211_CHAN_WIDTH_40;
			cfg->radio.freq1 += 10;
			break;
		}
		else if (0 == strncasecmp(arg, "ht40-", strlen(arg))) {
			cfg->radio.rt.mcs.flags |= IEEE80211_RADIOTAP_MCS_BW_40;
			cfg->radio.moep_chan_width = MOEP80211_CHAN_WIDTH_40;
			cfg->radio.freq1 -= 10;
			break;
		}

		argp_failure(state, 1, errno, "Invalid HT bandwidth: %s", arg);
		break;
	case 'g':
		if (cfg->radio.rt.it_present & BIT(IEEE80211_RADIOTAP_RATE)) {
			cfg->radio.rt.it_present &=
				~BIT(IEEE80211_RADIOTAP_RATE);
			cfg->radio.rt.mcs.known |=
				IEEE80211_RADIOTAP_MCS_HAVE_MCS;
			cfg->radio.rt.mcs.mcs = cfg->radio.rt.rate;
			cfg->radio.rt.rate = 0;
		}
		cfg->radio.rt.it_present |= BIT(IEEE80211_RADIOTAP_MCS);
		cfg->radio.rt.mcs.known |= IEEE80211_RADIOTAP_MCS_HAVE_GI;
		if (atoi(arg) == 400)
			cfg->radio.rt.mcs.flags |= IEEE80211_RADIOTAP_MCS_SGI;
		else if (atoi(arg) != 800)
			argp_failure(state, 1, errno, "Invalid GI: %s", arg);
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
			cfg->radio.freq0 = freq;
			cfg->radio.freq1 += freq;
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
