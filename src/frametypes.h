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

struct ieee80211_encryption_hdr {
	uint8_t tsc1;
	uint8_t wep_seed;
	uint8_t tsc0;
	uint8_t rsvd:5;
	uint8_t ext_iv:1;
	uint8_t key_id:2;
} __attribute__((packed));

#endif
