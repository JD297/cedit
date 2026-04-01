/**
BSD 2-Clause License

Copyright (c) 2025-2026, JD297
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef JD297_LIST_H
#define JD297_LIST_H

#include <assert.h>
#include <stddef.h>

#include <jd297/string.h>

typedef struct list_node_t {
    str_t *value;
    struct list_node_t *next;
    struct list_node_t *prev;
} list_node_t;

typedef struct {
    list_node_t *begin;
    list_node_t *end;

    size_t size;
} list_t;

extern int list_create(list_t *list);

extern void list_free(list_t *list);

extern int list_clear(list_t *list);

extern int list_empty(list_t *list);

extern list_node_t *list_insert(list_t *list, list_node_t *pos, str_t *value);

extern list_node_t *list_erase(list_t *list, list_node_t *pos);

extern size_t list_distance(list_node_t *first, list_node_t *last);

extern void list_advance(list_node_t **it, long n);

#define list_begin(list) (list)->begin
#define list_end(list) (list)->end
#define list_next(node) (node)->next
#define list_prev(node) (node)->prev
#define list_size(list) (list)->size

#endif

#ifdef JD297_LIST_IMPLEMENTATION

#include <stdlib.h>

extern int list_create(list_t *list)
{
	list->end = NULL;

	list->begin = calloc(1, sizeof(list_node_t));

	if (list->begin == NULL) {
		return -1;
	}
	
	list->end = list->begin;

	list_size(list) = 0;

	return 0;
}

extern int list_empty(list_t *list)
{
	return list->begin == list->end;
}

extern list_node_t *list_insert(list_t *list, list_node_t *pos, str_t *value)
{
	list_node_t *node = calloc(1, sizeof(list_node_t));
	
	if (node == NULL) {
		return NULL;
	}

	if (pos == list->begin) {
		list->begin = node;
	} else {
		pos->prev->next = node;
	}

	node->value = value;

	node->prev = pos->prev;
	node->next = pos;
	pos->prev = node;

	++list_size(list);

	return node;
}

extern list_node_t *list_erase(list_t *list, list_node_t *pos)
{
	list_node_t *next;
	
	if (list->end == pos->next) {
			next = list->end;
	} else {
		pos->next->prev = pos->prev;
		next = pos->prev->next = pos->next;
	}
	
	if (list->begin == pos) {
		list->begin = next;
	}
	
	free(pos);

	--list_size(list);

	return next;
}

extern void list_free(list_t *list)
{
	list_node_t *it;

	for (it = list->begin; it != NULL; ) {
		list_node_t *next = it->next;

		free(it);

		it = next;
	}
}

extern int list_clear(list_t *list)
{
	list_free(list);
	
	return list_create(list);
}

extern size_t list_distance(list_node_t *first, list_node_t *last)
{
	list_node_t *it;
	size_t d = 0;

	for (it = first; it != last; it = list_next(it), ++d);

	return d;
}

extern void list_advance(list_node_t **it, long n)
{
	while (n != 0) {
		if (n > 0) { /* positive */
			assert(list_next(*it) != NULL);

			*it = list_next(*it);
			--n;
		} else { /* negative */
			assert(list_prev(*it) != NULL);

			*it = list_prev(*it);
			++n;
		}
	}
}

#endif
