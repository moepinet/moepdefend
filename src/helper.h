#ifndef _HELPER_H_
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <moepcommon/types.h>
#include <moepcommon/util.h>
#include <moep80211/modules/ieee80211.h>

#include "frametypes.h"


#define CIPHER_SUITE_WEP40	1
#define CIPHER_SUITE_TKIP	2
#define CIPHER_SUITE_CCMP	4
#define CIPHER_SUITE_WEP104	5

extern const char * cipher_suite_string[7];

int get_bssid(u8 *buffer, const struct ieee80211_hdr_gen *hdr);
int get_sta_hwaddr(u8 *buffer, const struct ieee80211_hdr_gen *hdr);
int get_essid(char *buffer, size_t maxlen, const struct ieee80211_beacon * bcn, ssize_t len);
int get_encryption(const struct ieee80211_beacon *bcn, size_t len);

#endif
