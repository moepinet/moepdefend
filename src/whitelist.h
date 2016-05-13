#ifndef _INIPARSER_H_
#define _INIPARSER_H_

#include "moepcommon/types.h"

struct whitelist {
	struct {
		int count;
		u8 **hwaddr;
	} cell;
	struct {
		int count;
		u8 **hwaddr;
	} sta;
};

int iniparser(const char *filename, struct whitelist *wlist);

#endif//_INIPARSER_H_
