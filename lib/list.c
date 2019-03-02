/*
 *
 *  alisacmdb: Alisatech Configuration Management Database library
 *  Copyright (C) 2015 Iain M Conochie <iain-AT-thargoid.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *  
 *  data.c
 *
 *  Contains the functions for data types various AIL_ data types
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ailsacmdb.h>

// Linked List functions

void
ailsa_list_init(AILLIST *list, void (*destroy)(void *data))
{
	list->total = 0;
	list->destroy = destroy;
	list->head = NULL;
	list->tail = NULL;
}

void
ailsa_list_destroy(AILLIST *list)
{
	void *data;
	int retval = 0;

	if (!(list))
		return;
	while (list->total > 0) {
		retval = ailsa_list_remove(list, list->tail, (void **)&data);
		if (retval == 0 && list->destroy != NULL) {
			list->destroy(data);
		}
	}
	memset(list, 0, sizeof(AILLIST));
}

int
ailsa_list_ins_next(AILLIST *list, AILELEM *element, void *data)
{
	size_t size = sizeof(AILELEM);
	AILELEM *new;

	if (!(element) && list->total != 0)
		return -1;
	new = ailsa_calloc(size, "new in ailsa_list_ins_next");
	new->data = data;
	if (list->total == 0) {
		list->head = new;
		list->tail = new;
		new->prev = NULL;
		new->next = NULL;
	} else {
		new->next = element->next;
		new->prev = element;
		if (element->next == NULL)
			list->tail = new;
		else
			element->next->prev = new;
		element->next = new;
	}
	list->total++;
	return 0;
}

int
ailsa_list_ins_prev(AILLIST *list, AILELEM *element, void *data)
{
	size_t size = sizeof(AILELEM);
	AILELEM *new;

	if (!(element) && list->total != 0)
		return -1;
	new = ailsa_calloc(size, "new in ailsa_list_ins_prev");
	new->data = data;
	if (list->total == 0) {
		list->head = new;
		list->tail = new;
		new->prev = NULL;
		new->next = NULL;
	} else {
		new->next = element;
		new->prev = element->prev;
		if (element->prev == NULL)
			list->head = new;
		else
			element->prev->next = new;
		element->prev = new;
	}
	list->total++;
	return 0;
}

int
ailsa_list_remove(AILLIST *list, AILELEM *element, void **data)
{
	if (!(element) || list->total == 0)
		return -1;
	*data = element->data;
	if (element == list->head) {
		list->head = element->next;
		if (!(list->head))
			list->tail = NULL;
		else
			element->next->prev = NULL;
	} else {
		element->prev->next = element->next;
		if (!(element->next))
			list->tail = element->prev;
		else
			element->next->prev = element->prev;
	}
	my_free(element);
	list->total--;
	return 0;
}

