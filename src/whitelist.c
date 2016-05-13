#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ether.h>

#include "moepcommon/util.h"

#include "whitelist.h"
#include "global.h"

enum {
	section_bssid	= 0,
	section_hwaddr	= 1,
	section_count	= 2,
};

static char * sectionname[] = {
	"[cell]",
	"[sta]",
};

struct la {
	struct list_head list;
	u8 hwaddr[IEEE80211_ALEN];
};

static int
find_section(FILE *file)
{
	char *line;
	size_t n, len;
	int i;

	for (line=NULL, n=0; 0<getline(&line, &n, file);) {
		for (i=0; i<section_count; i++) {
			len = strlen(sectionname[i]);
			if (0 == strncmp(line, sectionname[i], min(len, n))) {
				free(line);
				return i;
			}
		}
	}

	free(line);
	return -1;
}

static int
parse_section(FILE *file, struct list_head *l)
{
	char *line;
	size_t n;
	int count;
	struct ether_addr *ptr;
	long pos;
	struct la * elem;

	pos = ftell(file);

	count = 0;
	for (line=NULL, n=0; 0<getline(&line, &n, file); line=NULL, n=0) {
		if (line[0] == '[') {
			fseek (file, pos, SEEK_SET);
			return count;
		}
		pos += strlen(line);
		if (!(ptr = ether_aton(line)))
			continue;

		elem = calloc(1, sizeof(struct la));
		memcpy(elem->hwaddr, ptr, IEEE80211_ALEN);
		list_add(&elem->list, l);
		count++;
	}

	return count;
}

int
whitelist_load(struct whitelist *wlist)
{
	FILE *file;
	int section;

	if (!(file = fopen(wlist->filename, "r")))
		return -1;

	INIT_LIST_HEAD(&wlist->cell);
	INIT_LIST_HEAD(&wlist->sta);

	section = find_section(file);
	while (section >= 0) {
		switch (section) {
			case section_bssid:
				parse_section(file, &wlist->cell);
			break;

			case section_hwaddr:
				parse_section(file, &wlist->sta);
			break;

			default:
			break;
		}
		section = find_section(file);
	}

	fclose(file);

	return 0;
}

void
whitelist_destroy(struct whitelist *wlist)
{
	int i;
	struct la *cur, *tmp;

	list_for_each_entry_safe(cur, tmp, &wlist->cell, list) {
		list_del(&cur->list);
		free(cur);
	}
	list_for_each_entry_safe(cur, tmp, &wlist->sta, list) {
		list_del(&cur->list);
		free(cur);
	}

	return;
}

void
whitelist_print(FILE *file, const struct whitelist *wlist)
{
	struct la *cur;

	fprintf(file, "Whitelisted cells by BSSID\n");
	list_for_each_entry(cur, &wlist->cell, list) {
		fprintf(file, "  %s\n", ether_ntoa(
			(const struct ether_addr *)cur->hwaddr));
	}

	fprintf(file, "Whitelisted stations by hwaddr\n");
	list_for_each_entry(cur, &wlist->cell, list) {
		fprintf(file, "  %s\n", ether_ntoa(
			(const struct ether_addr *)cur->hwaddr));
	}
}

