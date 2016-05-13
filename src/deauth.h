#ifndef _DEAUTH_H_
#define _DEAUTH_H_

#include <moep80211/types.h>
#include <moep80211/system.h>
#include <moep80211/modules/ieee80211.h>

moep_frame_t deauth(const u8 *victim, const u8 *bssid);

#endif
