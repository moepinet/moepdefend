#ifndef _HELPER_H_
#define _HELPER_H_

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <moepcommon/types.h>
#include <moepcommon/util.h>
#include <moep80211/modules/ieee80211.h>

#include "frametypes.h"

int get_bssid(u8 *buffer, const struct ieee80211_hdr_gen *hdr);
int get_sta_hwaddr(u8 *buffer, const struct ieee80211_hdr_gen *hdr);
int get_essid(char *buffer, size_t maxlen, const struct ieee80211_beacon * bcn, ssize_t len);

#endif
