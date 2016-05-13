#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "whitelist.h"
#include "global.h"
#include "moepcommon/util.h"

enum {
	section_bssid	= 0,
	section_hwaddr	= 1,
	section_count	= 2,
};

static char * sectionname[] = {
	"[cell]",
	"[sta]",
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
			if (strncmp(line, sectionname[i], min(len, n))) {
				free(line);
				return i;
			}
		}
	}

	free(line);
	return -1;
}

static int
parse_section(FILE *file, u8 **hwaddr)
{
	char *line;
	size_t n;
	int count;
	struct ether_addr *ptr;

	hwaddr = NULL;

	count = 0;
	for (line=NULL, n=0; 0<getline(&line, &n, file); line=NULL, n=0) {
		if (line[0] == '[') {
			fseek (file, -n, SEEK_CUR);
			return count-1;
		}
		if (!(ptr = ether_aton(line)))
			continue;

		hwaddr = realloc(hwaddr, (count+1)*sizeof(hwaddr));
		hwaddr[count] = malloc(IEEE80211_ALEN);
		memcpy(hwaddr[count], ptr, IEEE80211_ALEN);
		fprintf(stderr, "%s\n", ether_ntoa(
			(const struct ether_addr *)hwaddr[count]));
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

	section = find_section(file);
	while (section >= 0) {
		switch (section) {
			case section_bssid:
				wlist->cell.count = parse_section(file,
					wlist->cell.hwaddr);
			break;

			case section_hwaddr:
				wlist->sta.count = parse_section(file,
					wlist->sta.hwaddr);
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

	for (i=0; i<wlist->cell.count; i++)
		free (wlist->cell.hwaddr[i]);
	for (i=0; i<wlist->sta.count; i++)
		free (wlist->sta.hwaddr[i]);

	free (wlist->cell.hwaddr);
	free (wlist->sta.hwaddr);

	return;
}

void
whitelist_print(const struct whitelist *wlist)
{
	for (i=0; i<wlist->cell.count; i++)
		free (wlist->cell.hwaddr[i]);
	for (i=0; i<wlist->sta.count; i++)
		free (wlist->sta.hwaddr[i]);
}

