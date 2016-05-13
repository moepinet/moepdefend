#ifndef _ASSERTION_H
#define _ASSERTION_H

#include <errno.h>

#define assert(expr)							\
({									\
	typeof (expr) __x__ = expr;					\
	if (errno) {							\
		DIE(#expr" failed, return value is %lld: %s",		\
			(long long)__x__, strerror(errno));		\
	}								\
})

#endif//_ASSERTION_H
