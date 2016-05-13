#ifndef _FRAMETYPES_H_
#define _FRAMETYPES_H_

/*
 * moep80211/ieee80211_frametypes.h defines a huge struct with dozens of
 * unions. Since we want to work with different payloads without the variable
 * length header, we define some of those unions explicitly here. Member names
 * are the same as used in ieee80211_frametypes.h.
 */

struct ieee80211_beacon {
	u64 timestamp;
	u16 beacon_int;
	u16 capab_info;
	u8 variable[0];
} __attribute__((packed));

struct ieee80211_deauth {
	u8 reason;
	u8 vendor_specific;
} __attribute__((packed));

#endif
