#ifndef _HELPER_H_
#define _HELPER_H_

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <moepcommon/types.h>
#include <moepcommon/util.h>
#include <moep80211/modules/ieee80211.h>

#include "frametypes.h"

u8 * get_bssid(struct ieee80211_hdr_gen *hdr);
u8 * get_sta_hwaddr(const struct ieee80211_hdr_gen *hdr);
char * get_essid(const struct ieee80211_beacon * bcn, ssize_t len);

#endif
