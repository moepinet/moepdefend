#ifndef _MATHS_H
#define _MATHS_H

/* Provides abs() */
#include <stdlib.h>

/*
 * include/linux/kernel.h
 */
#ifndef max
#define max(x,y) ({			\
	typeof(x) _max1 = (x);		\
	typeof(y) _max2 = (y);		\
	(void) (&_max1 == &_max2);	\
	_max1 > _max2 ? _max1 : _max2; })
#endif

#ifndef min
#define min(x,y) ({			\
	typeof(x) _min1 = (x);		\
	typeof(y) _min2 = (y);		\
	(void) (&_min1 == &_min2);	\
	_min1 < _min2 ? _min1 : _min2; })
#endif

#ifndef max_t
#define max_t(type, x, y) ({		\
	type __max1 = (x);		\
	type __max2 = (y);		\
	__max1 > __max2 ? __max1: __max2; })
#endif

#ifndef min_t
#define min_t(type, x, y) ({		\
	type __min1 = (x);		\
	type __min2 = (y);		\
	__min1 < __min2 ? __min1: __min2; })
#endif

#ifndef mult_frac
#define mult_frac(x, numer, denom) ({			\
	typeof(x) quot = (x) / (denom);			\
	typeof(x) rem  = (x) % (denom);			\
	(quot * (numer)) + ((rem * (numer)) / (denom)); })
#endif


/*
 * some additonal math functions
 */
#ifndef delta
#define delta(x,y,m) ({					\
	((x) - (y) + (m) + 1) % ((m) + 1); })
#endif


#endif//_MATHS_H
