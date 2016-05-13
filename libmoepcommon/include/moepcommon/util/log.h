#ifndef __LOG_H_
#define __LOG_H_

#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"


//#define MOEP80211_LOG_USE_SYSLOG
#define MOEP80211_LOG_LEVEL	LOG_DEBUG

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#ifdef MOEP80211_LOG_USE_SYSLOG
#define LOG(loglevel, ...)						\
do {									\
syslog(loglevel,__FILE__":"TOSTRING(__LINE__)": "__VA_ARGS__);		\
} while (0)
#else
#define LOG(loglevel, ...)						\
do {									\
if (loglevel <= MOEP80211_LOG_LEVEL) {					\
	fprintf(stderr, __FILE__":"TOSTRING(__LINE__)": "__VA_ARGS__);	\
	fprintf(stderr, "\n");						\
}									\
} while (0)
#endif

#ifdef MOEP80211_LOG_USE_SYSLOG
#define DIE(...)							\
do {									\
syslog(LOG_ERR,__FILE__":"TOSTRING(__LINE__)": "__VA_ARGS__);		\
exit(-1);								\
} while (0)
#else
#define DIE(...)							\
do {									\
fprintf(stderr, __FILE__":"TOSTRING(__LINE__)": "__VA_ARGS__);		\
fprintf(stderr, "\n");							\
exit(-1);								\
} while (0)
#endif

#endif // __LOG_H_
