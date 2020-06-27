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
ailsa_list_clean(AILLIST *list)
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
}

void
ailsa_list_destroy(AILLIST *list)
{

	if (!(list))
		return;
	ailsa_list_clean(list);
	memset(list, 0, sizeof(AILLIST));
	return;
}

void
ailsa_list_full_clean(AILLIST *l)
{
	ailsa_list_destroy(l);
	my_free(l);
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
ailsa_list_insert(AILLIST *list, void *data)
{
	AILELEM *tmp;
	int retval = 0;
	if (!(list) || !(data))
		return -1;
	if (list->total == 0) {
		retval = ailsa_list_ins_next(list, NULL, data);
	} else {
		AILELEM *new = ailsa_calloc(sizeof(AILELEM), "new in ailsa_list_insert");
		new->data = data;
		tmp = (AILELEM *)list->tail;
		tmp->next = new;
		new->prev = list->tail;
		new->next = NULL;
		list->tail = new;
		list->total++;
	}
	return retval;
}

int
ailsa_list_remove(AILLIST *list, AILELEM *element, void **data)
{
	if (!(element) || list->total == 0)
		return -1;
	int retval = 0;

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
	list->total--;
	my_free(element);
	return retval;
}

int
ailsa_list_pop_element(AILLIST *list, AILELEM *element)
{
	AILELEM *e, *f;
	if (!(element) || list->total == 0)
		return -1;
	if (element == list->head) {
		list->head = NULL;
	} else if (element == list->tail) {
		list->tail = list->tail->prev;
		list->tail->next = NULL;

	} else {
		e = list->head;
		while (e)
			if (e->next == element)
				break;
		f = list->tail;
		while (f)
			if (f->prev == element)
				break;
		f->prev = e;
		e->next = f;
	}
	list->total--;
	return 0;
}

void
ailsa_clean_element(AILLIST *list, AILELEM *e)
{
	if (!(list) || !(e))
		return;
	list->destroy(e->data);
	my_free(e);
}

AILELEM *
ailsa_list_get_element(AILLIST *list, size_t number)
{
	if (!(list) || (number == 0))
		return NULL;
	if (number > list->total)
		return NULL;
	AILELEM *element = list->head;
	size_t i;

	for (i = 1; i < number; i++) {
		element = element->next;
	}
	return element;
}

AILELEM *
ailsa_move_down_list(AILELEM *element, size_t number)
{
	if (!(element))
		return NULL;
	size_t i;
	AILELEM *e = element;
	for (i = 0; i < number; i++) {
		if (e)
			e = e->next;
	}
	return e;
}

AILELEM *
ailsa_replace_element(AILLIST *list, AILELEM *element, size_t number)
{
	if (!(list) || !(element))
		return NULL;
	size_t i;
	AILELEM *e = list->head;
	if (number == 0) {
		list->head = element;
		element->next = e->next;
		e->next->prev = element;
	} else if (number == list->total) {
		e = list->tail;
		list->tail = element;
		element->prev = e;
		e->prev->next = element;
	} else {
		for (i = 0; i < number; i++) {
			if (e)
				e = e->next;
		}
		e->next->prev = element;
		element->next = e->next;
		e->prev->next = element;
		element->prev = e->prev;
	}
	return e;
}
