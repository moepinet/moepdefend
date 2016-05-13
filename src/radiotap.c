#include <string.h>

#include "global.h"
#include "radiotap.h"
#include "cfg.h"

extern struct cfg cfg;

void
radiotap_set_defaults(struct moep80211_radiotap *rt)
{
	memset(rt, 0, sizeof (*rt));

	rt->hdr.it_present = cfg.radio.rt.it_present;
	rt->rate = cfg.radio.rt.rate;
	rt->mcs.known = cfg.radio.rt.mcs.known;
	rt->mcs.flags = cfg.radio.rt.mcs.flags;
	rt->mcs.mcs = cfg.radio.rt.mcs.mcs;

	rt->hdr.it_present |= BIT(IEEE80211_RADIOTAP_TX_FLAGS);
	rt->tx_flags = IEEE80211_RADIOTAP_F_TX_NOACK;
}
