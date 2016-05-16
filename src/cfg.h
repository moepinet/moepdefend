#ifndef _CFG_H_
#define _CFG_H_

#include <stdlib.h>

#include <moepcommon/types.h>
#include <moep80211/system.h>
#include <moep80211/dev.h>

#include "whitelist.h"

struct cfgradio {
	u64	freq0;
	u64	freq1;
	u64	moep_chan_width;
	struct {
		u32	it_present;
		u8	rate;
		struct {
			u8 known;
			u8 flags;
			u8 mcs;
		} mcs;
	} rt;
};

struct cfgdev {
	char	*name;
	size_t	mtu;
	moep_dev_t dev;
	int	tx_rdy;
	int	rx_rdy;
};

struct cfg {
	int daemon;
	int defmode;
	struct cfgradio radio;
	struct cfgdev rad;
	struct whitelist whitelist;
};

#endif//_CFG_H_
