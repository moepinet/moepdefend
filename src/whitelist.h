#ifndef _INIPARSER_H_
#define _INIPARSER_H_

#include "moepcommon/types.h"

struct whitelist {
	char filename[256];
	struct {
		int count;
		u8 **hwaddr;
	} cell;
	struct {
		int count;
		u8 **hwaddr;
	} sta;
};

int whitelist_load(struct whitelist *wlist);
void whitelist_destroy(struct whitelist *wlist);

#endif//_INIPARSER_H_
