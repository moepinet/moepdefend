#ifndef _INIPARSER_H_
#define _INIPARSER_H_

#include <stdio.h>

#include "moepcommon/types.h"
#include "moepcommon/list.h"

struct whitelist {
	char filename[256];
	struct list_head cell;
	struct list_head sta;
};

int whitelist_load(struct whitelist *wlist);
void whitelist_destroy(struct whitelist *wlist);
void whitelist_print(FILE *fd, const struct whitelist *wlist);
int whitelist_check_cell(const u8 *hwaddr);
int whitelist_check_sta(const u8 *hwaddr);

#endif//_INIPARSER_H_
