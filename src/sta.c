#include <moepcommon/util.h>
#include <moepcommon/list.h>
#include <moepcommon/timeout.h>

#include "sta.h"
#include "global.h"

struct sta_priv {
	timeout_t timeout;
};

static int
timeout_cb(timeout_t t, u32 overrun, void *data)
{
	(void) overrun;
	(void) t;
	sta_delete(data);
	return 0;
}

int
sta_delete(sta_t sta)
{
	list_del(&sta->list);

	LOG(LOG_INFO, "sta: deleted cell %s (timeout)",
		ether_ntoa((const struct ether_addr *)sta->hwaddr));

	timeout_delete(sta->priv->timeout);
	free(sta->priv);
	free(sta);

	return 0;
}

sta_t
sta_find(const struct list_head *sl, const u8 *hwaddr)
{
	struct sta *cur;

	list_for_each_entry(cur, sl, list) {
		if (0 == memcmp(hwaddr, cur->hwaddr, IEEE80211_ALEN))
			return cur;
	}

	return NULL;
}

sta_t
sta_add(struct list_head *sl, const u8 *hwaddr)
{
	sta_t sta = sta_find(sl, hwaddr);

	if (sta) {
		errno = EEXIST;
		return NULL;
	}

	sta = calloc(1, sizeof(struct sta));
	sta->priv = calloc(1, sizeof(struct sta_priv));
	memcpy(sta->hwaddr, hwaddr, IEEE80211_ALEN);

	if (0 > timeout_create(CLOCK_MONOTONIC, &sta->priv->timeout, timeout_cb, sta))
		DIE("timeout_create() failed: %s", strerror(errno));
	timeout_settime(sta->priv->timeout, 0, timeout_msec(STA_TIMEOUT*1000,0));

	list_add(&sta->list, sl);

	LOG(LOG_ERR, "sta: new station at %s",
		ether_ntoa((const struct ether_addr *)sta->hwaddr));

	return sta;
}

int
sta_update(sta_t sta)
{
	timeout_settime(sta->priv->timeout, 0, timeout_msec(STA_TIMEOUT*1000,0));
	sta->numpackets++;
	return 0;
}

int
sta_inactive(const sta_t sta, struct timespec *ts)
{
	struct itimerspec remaining;
	int ret;

	if (0 > (ret = timeout_gettime(sta->priv->timeout, &remaining)))
		return ret;

	ts->tv_sec = STA_TIMEOUT;
	ts->tv_nsec = 0;
	timespecsub(ts, &remaining.it_value);

	return 0;
}

