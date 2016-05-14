#include "helper.h"

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

	for (ptr=0; ptr<len-2; ptr+=bcn->variable[ptr+1]) {
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
