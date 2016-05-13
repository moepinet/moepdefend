#ifndef _MAC_H
#define _MAC_H

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include <netinet/ether.h>

#define MAC_ALEN 6

static inline int
maccmp(const void *x, const void *y)
{
	return strncmp((const char *)x, (const char *)y, MAC_ALEN);
}

static inline int
is_mcast_mac(const void *mac) {
	// IPv4: 01-00-5E-00-00-00 to 01-00-5E-7F-FF-FF
	// IPv6: 33-33-00-00-00-00 to 33-33-FF-FF-FF-FF
	if ((*((uint32_t *) mac) & htole32(0x80ffffff)) == htole32(0x005e0001))
		return 1;
	if (*((uint16_t *) mac) == 0x3333)
		return 1;
	return 0;
}

static inline int
is_bcast_mac(const void *mac)
{
	if (*((uint32_t *) mac) != 0xffffffff)
		return 0;
	if (*((uint16_t *) (mac+4)) != 0xffff)
		return 0;
	return 1;
}

static inline int
is_local_mac(const void *mac, const void *local)
{
	return !memcmp(mac,local,MAC_ALEN);
}

static inline int
is_zero_mac(const void *mac)
{
	unsigned long long int zero = 0;
	return !memcmp(mac,&zero,MAC_ALEN);
}

#endif //_MAC_H
