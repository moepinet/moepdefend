#include <unistd.h>
#include <moepcommon/util.h>

#include "myattack.h"
#include "../global.h"
#include "../helper.h"
#include "../frametypes.h"
#include "../radiotap.h"

moep_frame_t
myattack(moep_frame_t frame)
{
	moep_frame_t f;
	struct moep80211_radiotap *rt;
	struct ieee80211_hdr_gen *hdr;
	struct ieee80211_encryption_hdr *payload;
	size_t len;

	(void) payload; // avoid compile time warning (set but not used)

	// We have to return a new frame
	f = moep80211_frame_clone(frame);

	// If we just modify the actual frame, let's clone it
	//
	//f = moep_frame_clone(frame);

	// Retrieve headers
	if (!(rt = moep_frame_radiotap(frame))) {
		LOG(LOG_ERR, "moep_frame_ieee80211_hdr() failed");
		goto fail;
	}
	if (!(hdr = moep_frame_ieee80211_hdr(frame))) {
		LOG(LOG_ERR, "moep_frame_ieee80211_hdr() failed");
		goto fail;
	}

	// The radiotap header has to be (re-)initialized for transmission
	radiotap_set_defaults(rt);

	// If we work on a cloned frame, we may also want to retrieve its
	// payload. In most cases this will be the beginning of another header
	// or the mayload of managemend/control frames.
	// You have to check what kind of frame it is before you can decide how
	// to interpret the payload.
	payload = (struct ieee80211_encryption_hdr *)
				moep_frame_get_payload(f, &len);

	// If this is a new frame, it has no payload yet. Allocate some memory,
	// put something meaningful in it, and set it as our frame's payload.
	//
	//payload = calloc(42, sizeof(u8));
	//...
	//if (!(moep_frame_set_payload(frame, (u8 *)&payload, sizeof(payload)))) {
	//	LOG(LOG_ERR, "moep_frame_set_payload() failed");
	//	goto fail;
	//}

	return f;

	fail:
	moep_frame_destroy(f);
	return NULL;
}



