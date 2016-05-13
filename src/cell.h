#ifndef __CELL_H
#define __CELL_H

#include <moepcommon/util.h>
#include <moepcommon/list.h>
#include <moepcommon/timeout.h>

#include "global.h"
#include "sta.h"

#define ESSID_MAX_LEN (32+1)

struct cell;
typedef struct cell * cell_t;

struct cell {
	struct list_head list;
	struct list_head sl;
	struct {
		timeout_t timeout;
	} task;
	u8 bssid[IEEE80211_ALEN];
	char essid[ESSID_MAX_LEN];
};

static int cell_del(const u8 *bssid);

static LIST_HEAD(cl);

static int
cell_delete(timeout_t t, u32 overrun, void *data)
{
	(void) overrun;
	(void) t;
	struct cell *cell = data;

	cell_del(cell->bssid);
	return 0;
}

static cell_t
cell_find(const u8 *bssid)
{
	struct cell *cur;

	list_for_each_entry(cur, &cl, list) {
		if (0 == memcmp(bssid, cur->bssid, IEEE80211_ALEN))
			return cur;
	}

	return NULL;
}

static cell_t
cell_add(const u8 *bssid)
{
	struct cell *cell = cell_find(bssid);

	if (cell) {
		errno = EEXIST;
		return NULL;
	}

	cell = calloc(1, sizeof(struct cell));
	INIT_LIST_HEAD(&cell->sl);
	memcpy(cell->bssid, bssid, IEEE80211_ALEN);

	if (0 > timeout_create(CLOCK_MONOTONIC, &cell->task.timeout, cell_delete, cell))
		DIE("timeout_create() failed: %s", strerror(errno));
	timeout_settime(cell->task.timeout, 0, timeout_msec(CELL_TIMEOUT*1000,0));

	list_add(&cell->list, &cl);

	LOG(LOG_ERR, "cell: new cell at %s",
		ether_ntoa((const struct ether_addr *)cell->bssid));

	return cell;
}

static int
cell_del(const u8 *bssid)
{
	struct cell *cell;

	if (!(cell = cell_find(bssid))) {
		errno = ENODEV;
		return -1;
	}

	list_del(&cell->list);

	LOG(LOG_ERR, "cell: deleted cell %s (timeout)",
		ether_ntoa((const struct ether_addr *)cell->bssid));

	timeout_delete(cell->task.timeout);
	free(cell);

	return 0;
}

static int
cell_update_timestamp(cell_t cell)
{
	timeout_settime(cell->task.timeout, 0, timeout_msec(CELL_TIMEOUT*1000,0));
	return 0;
}

static int
cell_inactive(const cell_t cell, struct timespec *ts)
{
	struct itimerspec remaining;
	int ret;

	if (0 > (ret = timeout_gettime(cell->task.timeout, &remaining)))
		return ret;

	ts->tv_sec = CELL_TIMEOUT;
	ts->tv_nsec = 0;
	timespecsub(ts, &remaining.it_value);

	return 0;
}

#endif
