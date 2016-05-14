#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#ifndef BIT
#define BIT(nr)		(1UL << (nr))
#endif

#define CELL_TIMEOUT	60	// [s]
#define STA_TIMEOUT	30	// [s]
#define LOG_INTERVAL	500	// [ms]

#define DEFAULT_LOGFILE		"/dev/shm/moepdefend"
#define DEFAULT_WHITELIST	"./whitelist.conf"

#endif//_GLOBAL_H_
