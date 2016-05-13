#ifndef _TIMEOUT_H
#define _TIMEOUT_H

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include "moepcommon/types.h"
#include "moepcommon/util.h"


/*
 * Flags controlling how timouts are being set. If flags=0, timeout_settime()
 * unconditionally sets the new timeout value. TIMEOUT_FLAG_SHORTEN indicates
 * that a new timeout is considered only if the timeout is currently
 * deactivated or the new timeout value is shorter than the remaining timeout.
 * TIMEOUT_FLAG_INACTIVE inidicates that the new timeout is considered only if
 * the current timeout is deactivated and ignored otherwise.
 */
#define TIMEOUT_FLAG_SHORTEN	0x01
#define TIMEOUT_FLAG_INACTIVE	0x02

struct timeout;
typedef struct timeout * timeout_t;

/*
 * Prototype for the callback being executed when a timeout occurs. The
 * callback gets a pointer to the respective timeout, e.g. to rearm or disable
 * the timeput, u32 indicating the number of timeouts eccured between the
 * last timeout and the currently handled timeout, e.g. any value larger than 0
 * indicates that the timeout expired but could not be handled - probably due
 * to too short timeout values, and pointer to private data (if present) the
 * callback may use.
 */
typedef int (*timeout_cb_t)(timeout_t, u32, void *);

/*
 * Internal timeout struct that shall never be accessed directly. Use the
 * typedef below instead.
 */
struct timeout {
	timer_t timerid;	// internal timer id
	struct sigevent sevp;	// eventfd for notification
	timeout_cb_t cb;	// callback when the timeout times out
	void *data;		// private data for the callback
};

/*
 * Create a new timeout. Returns 0 on success or -1 on error with errno set.
 * @clockid: Indicates the clockid to use. Possible values are CLOCK_MONOTONIC
 * (probably the one you want), CLOCK_REALTIME, and the corresponding _COARSE
 * variants.
 * @timeoutid: Pointer to a timeout_t typedef. Can be used after successfull
 * return.
 * @cb: Callback that is registered to be executed whenever the timeout times
 * out. In case of a non-recurring timeout this callback may be responsible for
 * rearming the timeout. The callback must not block.
 * @data: Private data for the callback.
 */
static inline int
timeout_create(clockid_t clockid, timeout_t *timeoutid, timeout_cb_t cb,
								void *data)
{
	timeout_t t;

	if (!(t = calloc(sizeof(*t), 1)))
		DIE("calloc() failed: %s", strerror(errno));

	t->cb = cb;
	t->data = data;
	t->sevp.sigev_notify = SIGEV_SIGNAL;
	t->sevp.sigev_signo = SIGRTMIN;
	t->sevp.sigev_value.sival_ptr = t;

	if (0 > timer_create(clockid, &t->sevp, &t->timerid)) {
		LOG(LOG_ERR, "timer_create() failed: %s", strerror(errno));
		free(t);
		return -1;
	}

	*timeoutid = t;

	return 0;
}

/*
 * Deletes a timeout and the associated timer. If the timer cannot be deleted,
 * memory is not freed. Returns 0 on succes and -1 on error with errno set.
 */
static inline int
timeout_delete(timeout_t t)
{
	int ret;

	ret = timer_delete(t->timerid);
	if (!ret)
		free(t);

	return ret;
}

/*
 * Disables a timeout. Returns 0 on success and -1 on error with errno set.
 */
static inline int
timeout_clear(timeout_t t)
{
	const struct itimerspec its = {{0,0},{0,0}};
	return timer_settime(t->timerid, 0, &its, NULL);
}

/*
 * Returns the remaining time until the timeout expires in cur_value. Returns 0
 * on success and -1 on error with errno set.
 */
static inline int
timeout_gettime(timeout_t t, struct itimerspec *cur_value)
{
	return timer_gettime(t->timerid, cur_value);
}

/*
 * Test whether or not the timeout is currentyl active. Returns 0 (inactive) or
 * 1 (active).
 * //FIXME function may fail if timeout is invalid.
 */
static inline int
timeout_active(timeout_t t)
{
	struct itimerspec its;

	if (0 > timeout_gettime(t, &its))
		return 0;

	if (timespecisset(&its.it_value))
		return 1;

	return 0;
}

/*
 * Sets a new timeout value and interval. The behavior depends on the value of
 * flags (see defintion of the flags above).
 * @t: Timeout to be modified.
 * @flags: Flags controlling the behavior of timeout_settime().
 * @new_value: itimersepc specifying the new timeout value and interval. You
 * may want to use timeout_msec() to create an appropriate new_value.
 */
static inline int
timeout_settime(timeout_t t, int flags, const struct itimerspec *new_value)
{
	const struct itimerspec its_zero = {{0,0},{0,1}};
	struct itimerspec its;

	if (!new_value)
		return timeout_clear(t);

	if (flags & TIMEOUT_FLAG_SHORTEN) {
		if (0 > timeout_gettime(t, &its))
			return -1;

		if (timespecisset(&its.it_value) &&
		    timespeccmp(&its.it_value, &new_value->it_value, <))
			return 0;
	}
	if (flags & TIMEOUT_FLAG_INACTIVE) {
		if (timeout_active(t))
			return 0;
	}

	if (!timespecisset(&new_value->it_value)) {
		return timer_settime(t->timerid, 0, &its_zero, NULL);
	}

	return timer_settime(t->timerid, 0, new_value, NULL);
}

/*
 * Used to execute the callback associated with a timeout.
 */
static inline int
timeout_exec(timeout_t t, u32 overrun)
{
	return t->cb(t, overrun, t->data);;
}

/*
 * Helper function to create a struct itimerspec given a tuple of
 * (<value>,<interval>) interpreted as msec. Note that the pointer returned is
 * statically allocated and is invalidated when timeout_msec is called again.
 */
static inline struct itimerspec *
timeout_msec(s64 value, s64 interval)
{
	static struct itimerspec its;
	timespecmset(&its.it_value, value);
	timespecmset(&its.it_interval, interval);
	return &its;
}

/*
 * Same as timeout_msec but values are given in usec instead of msec.
 */
static inline struct itimerspec *
timeout_usec(s64 value, s64 interval)
{
	static struct itimerspec its;
	timespecuset(&its.it_value, value);
	timespecuset(&its.it_interval, interval);
	return &its;
}

#endif //_TIMEOUT_H
