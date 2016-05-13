#ifndef _HEXDUMP_H
#define _HEXDUMP_H

#include <stddef.h>
#include <stdio.h>

static inline char *
hexdump2(const void *buffer, ssize_t len)
{
	static char str[65536];
	unsigned int i;
	int p = 0;

	for (i = 0; i < len; i++) {
		if ((i % 8 == 0) && (i != 0)) {
			p += sprintf(str + p, "  ");
		}
		if (i % 16 == 0) {
			if (i == 0) {
				p += sprintf(str + p, ">> %.4x:  ", i);
			} else {
				fprintf(stderr,"%s\n",str);
				p = 0;
				p += sprintf(str + p, ">> %.4x:  ", i);
			}
		}
		p += sprintf(str + p, "%c ", ((unsigned char *) buffer)[i]);

		if ((sizeof(str) - p) < 64)
			break;
	}

	return str;
}

static inline void
hexdump(const void *buffer, ssize_t len) {
	char *str = hexdump2(buffer, len);
	fprintf(stderr, "%s\n", str);
}

#endif
