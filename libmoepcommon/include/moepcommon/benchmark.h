#ifndef __BENCHMARK_H_
#define __BENCHMARK_H_

#include "moepcommon/types.h"
#include "moepcommon/util.h"

struct timebench {
	struct timespec start;
	struct timespec stop;
	struct timespec last;
	struct timespec now;
};

#define TIMEBENCH_INIT(name) \
	static struct timebench name = {{0,0},{0,0},{0,0},{0,0}}

static inline void
timebench_start(struct timebench *tb)
{
	clock_gettime(CLOCK_MONOTONIC, &tb->start);
}

static inline u64
timebench_stop(struct timebench *tb)
{
	u64 usec;

	clock_gettime(CLOCK_MONOTONIC, &tb->stop);
	timespecsub(&tb->stop, &tb->start);

	usec = tb->stop.tv_sec*1000000 + tb->stop.tv_nsec/1000;
	return usec;
}

static inline u64
timebench_sample(struct timebench *tb)
{
	struct timespec tmp;
	u64 usec;

	clock_gettime(CLOCK_REALTIME, &tmp);
	tb->now = tmp;
	timespecsub(&tmp, &tb->last);
	tb->last = tb->now;

	usec = tmp.tv_sec*1000000 + tmp.tv_nsec/1000;
	return usec;
}

#endif
