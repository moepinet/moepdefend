#include "state.h"
#include "helper.h"
#include "cell.h"
#include "sta.h"

static char *
cipher_str(u32 ciphers)
{
	static char str[256];
	size_t n;

	memset(str, 0, sizeof(str));

	if (ciphers & BIT(CIPHER_SUITE_WEP40)) {
		n = snprintf(str, sizeof(str)-n, " %s",
				cipher_suite_string[CIPHER_SUITE_WEP40]);
	}
	if (ciphers & BIT(CIPHER_SUITE_WEP104)) {
		n = snprintf(str, sizeof(str)-n, " %s",
				cipher_suite_string[CIPHER_SUITE_WEP104]);
	}
	if (ciphers & BIT(CIPHER_SUITE_CCMP)) {
		n = snprintf(str, sizeof(str)-n, " %s",
				cipher_suite_string[CIPHER_SUITE_CCMP]);
	}
	if (ciphers & BIT(CIPHER_SUITE_TKIP)) {
		n = snprintf(str, sizeof(str)-n, " %s",
				cipher_suite_string[CIPHER_SUITE_TKIP]);
	}

	return str;
}

int
state_log()
{
	cell_t cell;
	sta_t sta;
	struct timespec inactive;
	FILE *file;
	int x;

	cell_sort();

	if (!(file = fopen(DEFAULT_LOGFILE, "w"))) {
		LOG(LOG_ERR, "fopen() failed: %s", strerror(errno));
		return -1;
	}

	list_for_each_entry_reverse(cell, &cl, list) {
		if (cell_inactive(cell, &inactive)) {
			LOG(LOG_ERR, "cell_inactive() failed: %s",
				strerror(errno));
			continue;
		}

		x = fprintf(file, "Cell %s %s%s",
			mac_ntoa((const struct ether_addr *)cell->bssid),
			cell->essid, cipher_str(cell->ciphers));
		fprintf(file, "%*lds ", 62-x, inactive.tv_sec);
		fprintf(file, "%*lu\n", 7, cell->numpackets);

		list_for_each_entry(sta, &cell->sl, list) {
			if (sta_inactive(sta, &inactive)) {
				LOG(LOG_ERR, "sta_inactive() failed: %s",
					strerror(errno));
				continue;
			}
			x = fprintf(file, "  STA %s", mac_ntoa(
				(const struct ether_addr *)sta->hwaddr));
			fprintf(file, "%*s[%lds,%lu,%d]\n", 25-x, "",
				inactive.tv_sec,sta->numpackets,
				sta->encrypted);
		}
		fprintf(file, "\n");
	}

	fclose(file);
	return 0;
}

int
state_log_cb(timeout_t t, u32 overrun, void *data)
{
	(void) t;
	(void) overrun;
	(void) data;

	return state_log();
}
