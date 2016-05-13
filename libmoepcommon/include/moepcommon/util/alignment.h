#ifndef _ALIGNMENT_H
#define _ALIGNMENT_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static inline size_t
aligned_length(size_t len, size_t alignment) {
	return (((len+alignment-1)/alignment)*alignment);
}

static inline void *
aligned_calloc(size_t len, size_t alignment)  {
	void *ptr;

	if (posix_memalign(&ptr, alignment, len))
		return NULL;
	memset(ptr, 0, len);

	return ptr;
}

#endif//_ALIGNMENT_H
