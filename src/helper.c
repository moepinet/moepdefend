#include "helper.h"
#include "global.h"

int
get_bssid(u8 *buffer, const struct ieee80211_hdr_gen *hdr)
{
	int fromds, tods;

	fromds = ieee80211_has_fromds(hdr->frame_control);
	tods = ieee80211_has_tods(hdr->frame_control);

	if (ieee80211_is_beacon(hdr->frame_control)) {
		memcpy(buffer, hdr->addr2, IEEE80211_ALEN);
	}
	else if (!ieee80211_is_data(hdr->frame_control)) {
		return -1;
	}
	else if (fromds & tods) {
		return -1; // unspecified
	}
	else if (fromds) {
		memcpy(buffer, hdr->addr2, IEEE80211_ALEN);
	}
	else if (tods) {
		memcpy(buffer, hdr->addr1, IEEE80211_ALEN);
	}
	else {
		return -1; // ad-hoc
	}

	return 0;
}

int
get_essid(char *buffer, size_t maxlen, const struct ieee80211_beacon * bcn,
								ssize_t len)
{
	size_t elen;
	int ptr;

	memset(buffer, 0, maxlen);

	for (ptr=0; ptr<len-2; ptr+=bcn->variable[ptr+1]+2) {
		if (bcn->variable[ptr] != WLAN_EID_SSID)
			continue;
		elen = min((size_t)bcn->variable[ptr+1]+1, maxlen);
		if (elen > 0)
			snprintf(buffer, elen, "%s", &bcn->variable[ptr+2]);
		return 0;
	}

	return -1;
}

int
get_sta_hwaddr(u8 *buffer, const struct ieee80211_hdr_gen *hdr)
{
	int fromds, tods;

	fromds = ieee80211_has_fromds(hdr->frame_control);
	tods = ieee80211_has_tods(hdr->frame_control);

	if (!ieee80211_is_data(hdr->frame_control)) {
		return -1;
	}
	else if (!(tods | fromds)) {
		return -1; //both are STAs (adhoc)
	}
	else if (fromds) {
		if (is_mcast_mac(hdr->addr1))
			return -1;
		memcpy(buffer, hdr->addr1, IEEE80211_ALEN);
	}
	else if (tods) {
		if (is_mcast_mac(hdr->addr2))
			return -1;
		memcpy(buffer, hdr->addr2, IEEE80211_ALEN);
	}
	else {
		return -1; //unspecified
	}

	return 0;
}

struct cipher {
	u8 oui[3];
	u8 type;
} __attribute__((packed));

struct wlan_eid_rsn {
	u8 eid;
	u8 len;
	struct cipher group_cipher;
	u16 rsn_version;
	u16 pairwise_cipher_count;
	u8 variable[0];

} __attribute__((packed));

int
get_encryption(const struct ieee80211_beacon *bcn, size_t len)
{
	size_t tag_len = 0;
	struct wlan_eid_rsn rsn;
	static struct cipher pairwise_cipher[16];
	int ptr, i;
	int mask = 0;

	for (ptr=0; ptr<len-2; ptr+=bcn->variable[ptr+1]+2) {
		if (bcn->variable[ptr] != WLAN_EID_RSN)
			continue;
		tag_len = bcn->variable[ptr+1];
		break;
	}

	if (0 == tag_len)
		return -1;


	memcpy(&rsn, bcn->variable+ptr, sizeof(rsn));
	memcpy(pairwise_cipher, bcn->variable+ptr+sizeof(rsn),
		min(rsn.pairwise_cipher_count,16)*sizeof(struct cipher));

	for (i=0; i<min(rsn.pairwise_cipher_count,2); i++)
		mask |= BIT(pairwise_cipher[i].type);

	return mask;
}
