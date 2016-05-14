#include <moepcommon/util.h>
#include <moepcommon/list.h>
#include <moepcommon/timeout.h>

#include "cell.h"
#include "sta.h"
#include "global.h"
#include "list_sort.h"

LIST_HEAD(cl);

struct cell_priv {
	timeout_t timeout;
	u8 bssid[IEEE80211_ALEN];
	char essid[IEEE80211_MAX_SSID_LEN+1];
};

static int
timeout_cb(timeout_t t, u32 overrun, void *data)
{
	(void) overrun;
	(void) t;
	cell_delete(data);
	return 0;
}

cell_t
cell_find(const u8 *bssid)
{
	struct cell *cur;

	list_for_each_entry(cur, &cl, list) {
		if (0 == memcmp(bssid, cur->bssid, IEEE80211_ALEN))
			return cur;
	}

	return NULL;
}

cell_t
cell_add(const u8 *bssid)
{
	struct cell *cell = cell_find(bssid);

	if (cell) {
		errno = EEXIST;
		return NULL;
	}

	cell = calloc(1, sizeof(struct cell));
	cell->priv = calloc(1, sizeof(struct cell_priv));

	INIT_LIST_HEAD(&cell->sl);
	memcpy(cell->bssid, bssid, IEEE80211_ALEN);

	if (0 > timeout_create(CLOCK_MONOTONIC, &cell->priv->timeout, timeout_cb, cell))
		DIE("timeout_create() failed: %s", strerror(errno));
	timeout_settime(cell->priv->timeout, 0, timeout_msec(CELL_TIMEOUT*1000,0));

	list_add(&cell->list, &cl);

	LOG(LOG_ERR, "cell: new cell at %s",
		ether_ntoa((const struct ether_addr *)cell->bssid));

	return cell;
}

int
cell_delete(cell_t cell)
{
	sta_t cur, tmp;
	list_del(&cell->list);

	list_for_each_entry_safe(cur, tmp, &cell->sl, list)
		sta_delete(cur);

	LOG(LOG_ERR, "cell: deleted cell %s (timeout)",
		ether_ntoa((const struct ether_addr *)cell->bssid));

	timeout_delete(cell->priv->timeout);
	free(cell);

	return 0;
}

int
cell_update_timestamp(cell_t cell)
{
	timeout_settime(cell->priv->timeout, 0, timeout_msec(CELL_TIMEOUT*1000,0));
	cell->numpackets++;
	return 0;
}

int
cell_update_essid(cell_t cell, const char *essid)
{
	if (strlen(essid) > IEEE80211_MAX_SSID_LEN)
		return -1;

	strncpy(cell->essid, essid, IEEE80211_MAX_SSID_LEN);

	return 0;
}

int
cell_inactive(const cell_t cell, struct timespec *ts)
{
	struct itimerspec remaining;
	int ret;

	if (0 > (ret = timeout_gettime(cell->priv->timeout, &remaining)))
		return ret;

	ts->tv_sec = CELL_TIMEOUT;
	ts->tv_nsec = 0;
	timespecsub(ts, &remaining.it_value);

	return 0;
}


static int cmp(void *priv, struct list_head *a, struct list_head *b)
{
	struct cell * ca = container_of(a, struct cell, list);
	struct cell * cb = container_of(b, struct cell, list);

	if (ca->numpackets > cb->numpackets)
		return 1;
	return 0;
}

void
cell_sort()
{
	list_sort(NULL, &cl, cmp);
}
