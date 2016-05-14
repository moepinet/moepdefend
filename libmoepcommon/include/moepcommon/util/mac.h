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
	return (0x01 & ((unsigned char *)mac)[0]);
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
is_unicast_mac(const void *mac)
{
	return is_mcast_mac(mac)^0x01;
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

static inline char *
mac_ntoa(const void *addr)
{
	static char str[18];

	sprintf(str, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
		((char *)addr)[0], ((char *)addr)[1], ((char *)addr)[2],
		((char *)addr)[3], ((char *)addr)[4], ((char *)addr)[5]);

	return str;
}

static inline char *
mac_ntoa_r(const void *addr)
{
	char *str;

	if (!(str = malloc(18))) {
		errno = ENOMEM;
		return NULL;
	}

	sprintf(str, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
		((char *)addr)[0], ((char *)addr)[1], ((char *)addr)[2],
		((char *)addr)[3], ((char *)addr)[4], ((char *)addr)[5]);

	return str;
}

#endif //_MAC_H
