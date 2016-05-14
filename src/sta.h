#ifndef _STA_H_
#define _STA_H_

#include <moepcommon/util.h>
#include <moepcommon/list.h>
#include <moepcommon/timeout.h>


struct sta_priv;

struct sta {
	struct list_head list;
	u8 hwaddr[IEEE80211_ALEN];
	u64 numpackets;
	u8 encrypted;
	s8 signal;
	struct sta_priv *priv;
};

typedef struct sta * sta_t;

int sta_delete(sta_t sta);
sta_t sta_find(const struct list_head *sl, const u8 *hwaddr);
sta_t sta_add(struct list_head *sl, const u8 *hwaddr);
int sta_update(sta_t sta);
int sta_inactive(const sta_t sta, struct timespec *ts);

#endif//_STA_H_
