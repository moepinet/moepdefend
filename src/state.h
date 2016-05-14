#ifndef _STATE_LOG_
#define _STATE_LOG_

#include <moepcommon/types.h>
#include <moepcommon/timeout.h>

extern struct list_head cl;

int state_log();
int state_log_cb(timeout_t t, u32 overrun, void *data);

#endif//_STATE_LOG_
