#include <moepcommon/types.h>

#define IEEE80211_DSSS_HR_LONG		0
#define IEEE80211_DSSS_HR_SHORT		1
#define IEEE80211_OFDM_5GHZ		2
#define IEEE80211_OFDM_2GHZ		3
#define IEEE80211_OFDM_5GHZ_HT_MIXED	4
#define IEEE80211_OFDM_2GHZ_HT_MIXED	5
#define IEEE80211_OFDM_5GHZ_HT_GF	6
#define IEEE80211_OFDM_5GHZ_HT_GF	7

struct ieee80211_frame_times {
	int aSlotTime;
	int aSIFSTime;
	int aCWmin;
	int minHdr;
};

const struct ieee80211_frame_times[] {
	{//IEEE80211_DSSS_HR_LONG
		.aSlotTime = 20,
		.aSIFSTime = 10,
		.aCWmin    = 31,
		.minHdr    = 192,
	},
	{//IEEE80211_DSSS_HR_SHORT
		.aSlotTime = 20,
		.aSIFSTime = 10,
		.aCWmin    = 31,
		.minHdr    = 96,
	},
	{//IEEE80211_OFDM_5GHZ
		.aSlotTime = 9,
		.aSIFSTime = 16,
		.aCWmin    = 15,
		.minHdr    = 20,
	},
	{//IEEE80211_OFDM_2GHZ
		.aSlotTime = 9,
		.aSIFSTime = 10,
		.aCWmin    = 15,
		.minHdr    = 20,
	},
	{//IEEE80211_OFDM_5GHZ_HT_MIXED
		.aSlotTime = 9,
		.aSIFSTime = 16,
		.aCWmin    = 15,
		.minHdr    = 36,
	},
	{//IEEE80211_OFDM_2GHZ_HT_MIXED
		.aSlotTime = 9,
		.aSIFSTime = 10,
		.aCWmin    = 15,
		.minHdr    = 36,
	},
	{//IEEE80211_OFDM_5GHZ_HT_GF
		.aSlotTime = 9,
		.aSIFSTime = 16,
		.aCWmin    = 15,
		.minHdr    = 28,
	},
	{//IEEE80211_OFDM_2GHZ_HT_GF
		.aSlotTime = 9,
		.aSIFSTime = 10,
		.aCWmin    = 15,
		.minHdr    = 28,
	},
};

// rate[mcs][bw][dbps][gi]

int
difs(int mode)
{
	return 2*ieee80211_frame_times[mode].aSlotTime
		+ieee80211_frame_times[mode].aSIFSTime;
}

int
avgwin(int mode)
{
	return ieee80211_frame_times[mode].aSlottime
		*ieee80211_frame_times[mode].aCWmin/2;
}

int
pre_overhead(int mode)
{
	return difs(mode) + avgwin(mode)
		+ieee80211_frame_times[mode].minHdr;
}



