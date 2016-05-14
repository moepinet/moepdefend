#ifndef _LIST_SORT_H_
#define _LIST_SORT_H_

#include <moepcommon/types.h>
#include <moepcommon/list.h>

void
list_sort(void *priv, struct list_head *head,
	int (*cmp)(void *priv, struct list_head *a, struct list_head *b));

#endif//_LIST_SORT_H_
