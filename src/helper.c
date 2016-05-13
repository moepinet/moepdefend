#include "helper.h"


u8 *
get_bssid(struct ieee80211_hdr_gen *hdr)
{
	int fromds, tods;
	u8 *ret = NULL;

	fromds = hdr->frame_control & IEEE80211_FCTL_FROMDS;
	tods = hdr->frame_control & IEEE80211_FCTL_TODS;

	if (fromds & tods)
		ret = NULL;
	else if (tods)
		ret = hdr->addr1;
	else if (fromds)
		ret = hdr->addr2;
	else
		ret = hdr->addr3;

	if (ret) {
		if (!is_unicast_mac(ret))
			ret = NULL;
	}

	return ret;
}

char *
get_essid(const struct ieee80211_beacon * bcn, ssize_t len)
{
	size_t elen;
	int ptr;
	static char essid[IEEE80211_MAX_SSID_LEN+1];

	memset(essid, 0, sizeof(essid));

	if (!bcn) {
		LOG(LOG_ERR, "moep_frame_get_payload() failed: %s",
			strerror(errno));
		return NULL;
	}

	for (ptr=0; ptr<len-2; ptr+=bcn->variable[ptr+1]) {
		if (bcn->variable[ptr] != WLAN_EID_SSID)
			continue;
		elen = min((int)bcn->variable[ptr+1]+1, IEEE80211_MAX_SSID_LEN+1);
		if (elen > 0)
			snprintf(essid, elen, "%s", &bcn->variable[ptr+2]);
		break;
	}

	return essid;
}

u8 *
get_sta_hwaddr(const struct ieee80211_hdr_gen *hdr)
{
	static u8 hwaddr[IEEE80211_ALEN];

	if (ieee80211_has_tods(hdr->frame_control))
		memcpy(hwaddr, hdr->addr2, IEEE80211_ALEN);
	else if (ieee80211_has_fromds(hdr->frame_control))
		memcpy(hwaddr, hdr->addr1, IEEE80211_ALEN);
	else
		return NULL;

	if (!is_unicast_mac(hwaddr))
		return NULL;

	return hwaddr;
}
