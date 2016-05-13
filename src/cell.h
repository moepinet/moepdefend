#ifndef _CELL_H_
#define _CELL_H_

#include <moepcommon/util.h>
#include <moepcommon/list.h>
#include <moepcommon/timeout.h>

#include "global.h"
#include "sta.h"

#define ESSID_MAX_LEN (32+1)

struct cell_priv;

struct cell {
	struct list_head list;
	struct list_head sl;
	u8 bssid[IEEE80211_ALEN];
	char essid[ESSID_MAX_LEN+1];
	struct cell_priv *priv;
};

typedef struct cell * cell_t;

extern struct list_head cl;


int cell_delete(cell_t cell);
int cell_timeout_cb(timeout_t t, u32 overrun, void *data);
cell_t cell_find(const u8 *bssid);
cell_t cell_add(const u8 *bssid);
int cell_delete(cell_t cell);
int cell_update_timestamp(cell_t cell);
int cell_inactive(const cell_t cell, struct timespec *ts);

#endif//_CELL_H_
