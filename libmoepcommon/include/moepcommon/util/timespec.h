#ifndef _TIMESPEC_H
#define _TIMESPEC_H

#include <time.h>

#define timespecclear(tvp)      ((tvp)->tv_sec = (tvp)->tv_nsec = 0)

#define timespecisset(tvp)      ((tvp)->tv_sec || (tvp)->tv_nsec)

#define timespeccmp(tvp, uvp, cmp)					\
	(((tvp)->tv_sec == (uvp)->tv_sec) ?				\
	    ((tvp)->tv_nsec cmp (uvp)->tv_nsec) :			\
	    ((tvp)->tv_sec cmp (uvp)->tv_sec))

#define timespecadd(vvp, uvp)						\
do {									\
	(vvp)->tv_sec += (uvp)->tv_sec;					\
	(vvp)->tv_nsec += (uvp)->tv_nsec;				\
	if ((vvp)->tv_nsec >= 1000000000) {				\
		(vvp)->tv_sec++;					\
		(vvp)->tv_nsec -= 1000000000;				\
	}								\
} while (0)

#define timespecsub(vvp, uvp)						\
do {									\
	(vvp)->tv_sec -= (uvp)->tv_sec;					\
	(vvp)->tv_nsec -= (uvp)->tv_nsec;				\
	if ((vvp)->tv_nsec < 0) {					\
		(vvp)->tv_sec--;					\
		(vvp)->tv_nsec += 1000000000;				\
	}								\
} while (0)

#define timespecmax(vvp, uvp, wvp)					\
do {									\
	if (timespeccmp(uvp, wvp, >)) {					\
		(vvp)->tv_sec = (uvp)->tv_sec;				\
		(vvp)->tv_nsec = (uvp)->tv_nsec;			\
	}								\
	else {								\
		(vvp)->tv_sec = (wvp)->tv_sec;				\
		(vvp)->tv_nsec = (wvp)->tv_nsec;			\
	}								\
} while (0)

#define timespecmset(tvp, msec)						\
do {									\
	(tvp)->tv_sec  = msec/1000;					\
	(tvp)->tv_nsec = (msec-((tvp)->tv_sec*1000))*1000000;		\
} while (0)

#define timespecuset(tvp, usec)						\
do {									\
	(tvp)->tv_sec  = usec/1000000;					\
	(tvp)->tv_nsec = (usec-((tvp)->tv_sec*1000000))*1000;		\
} while (0)

#endif//_TIMESPEC_H
