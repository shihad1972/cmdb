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
 *  hash.c
 *
 *  Contains the functions for hash functions and data types
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

// Hash tables functions

// Hash a character string key
unsigned int
ailsa_hash(const void *key)
{
	const char	*ptr;
	unsigned int	val, tmp;

	val = 0;
	ptr = key;
	while (*ptr != '\0') {
		val = (val << 4) + (unsigned)(*ptr);
		if ((tmp = (val & 0xf0000000))) {
			val = val ^ (tmp >> 24);
			val = val ^ tmp;
		}
		ptr++;
	}
	return val;
}

int
ailsa_hash_init(AILHASH *htbl, unsigned int buckets,
		unsigned int (*h)(const void *key),
		int (*match)(const void *key1, const void *key2),
		void (*destroy)(void *data))
{
	unsigned int i;

	htbl->table = ailsa_calloc(buckets * sizeof(AILLIST), "htbl->table in ailsa_hash_init");
	htbl->buckets = buckets;
	for (i = 0; i < buckets; i++)
		ailsa_list_init(&htbl->table[i], destroy);
	htbl->h = h;
	htbl->match = match;
	htbl->destroy = destroy;
	htbl->size = 0;
	return 0;
}

void
ailsa_hash_destroy(AILHASH *htbl)
{
	unsigned int i;

	for (i = 0; i < htbl->buckets; i++)
		ailsa_list_destroy(&htbl->table[i]);
	my_free(htbl->table);
	memset(htbl, 0, sizeof(AILHASH));
}

int
ailsa_hash_insert(AILHASH *htbl, void *data, const char *key)
{
	unsigned int	bucket;
	int		retval;

	if (ailsa_hash_lookup(htbl, &data, key) == 0)
		return 1;
	bucket = htbl->h(key) % htbl->buckets;
	if ((retval = ailsa_list_ins_next(&htbl->table[bucket], NULL, data)) == 0)
		htbl->size++;
	return retval;
}

int
ailsa_hash_remove(AILHASH *htbl, void **data, const char *key)
{
	AILELEM 	*em, *prev;
	unsigned int	bucket;

	bucket = htbl->h(key) % htbl->buckets;
	prev = NULL;
	for (em = htbl->table[bucket].head; em != NULL; em = em->next) {
		if (htbl->match(*data, em->data)) {
			if (ailsa_list_remove(&htbl->table[bucket], prev, data) == 0) {
				htbl->size--;
				return 0;
			} else {
				return 1;
			}
		}
		prev = em;
	}
	return -1;
}

int
ailsa_hash_lookup(AILHASH *htbl, void **data, const char *key)
{
	AILELEM 	*em;
	unsigned int	bucket;

	bucket = htbl->h(key) % htbl->buckets;
	for (em = htbl->table[bucket].head; em != NULL; em = em->next) {
		if (htbl->match(*data, em->data)) {
			*data = em->data;
			return 0;
		}
	}
	return -1;
}

