#ifndef _STA_H_
#define _STA_H_

#include <moepcommon/util.h>
#include <moepcommon/list.h>
#include <moepcommon/timeout.h>

struct sta;
typedef struct sta * sta_t;

struct sta {
	struct list_head list;
	struct {
		timeout_t timeout;
	} task;
	u8 hwaddr[IEEE80211_ALEN];
};

static int sta_del(const u8 *hwaddr);

static int
sta_delete(timeout_t t, u32 overrun, void *data)
{
	(void) overrun;
	(void) t;
	struct sta *sta = data;

	sta_del(sta->hwaddr);
	return 0;
}

static sta_t
sta_find(const struct list_head *sl, const u8 *hwaddr)
{
	struct sta *cur;

	list_for_each_entry(cur, sl, list) {
		if (0 == memcmp(hwaddr, cur->hwaddr, IEEE80211_ALEN))
			return cur;
	}

	return NULL;
}

static sta_t
sta_add(struct list_head *sl, const u8 *hwaddr)
{
	struct sta *sta = sta_find(sl, hwaddr);

	if (sta) {
		errno = EEXIST;
		return NULL;
	}

	sta = calloc(1, sizeof(struct sta));
	memcpy(sta->hwaddr, hwaddr, IEEE80211_ALEN);

	if (0 > timeout_create(CLOCK_MONOTONIC, &sta->task.timeout, sta_delete, sta))
		DIE("timeout_create() failed: %s", strerror(errno));
	timeout_settime(sta->task.timeout, 0, timeout_msec(STA_TIMEOUT*1000,0));

	list_add(&sta->list, sl);

	LOG(LOG_ERR, "sta: new station at %s",
		ether_ntoa((const struct ether_addr *)sta->hwaddr));

	return sta;
}

static int
sta_update(sta_t sta)
{
	timeout_settime(sta->task.timeout, 0, timeout_msec(STA_TIMEOUT*1000,0));
	return 0;
}

static int
sta_inactive(const sta_t sta, struct timespec *ts)
{
	struct itimerspec remaining;
	int ret;

	if (0 > (ret = timeout_Gettime(sta->task.timeout, &remaining)))
		return ret;

	ts->tv_sec = STA_TIMEOUT;
	ts->tv_nsec = 0;
	timespecsub(ts, &remaining.it_value);

	return 0;
}

#endif//_STA_H_
