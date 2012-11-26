/* checks.c
 * 
 * Contains the function to validate user input before adding or
 * searching the database. These are basic sanity checks using
 * regex on the input strings. Uses the PCRE library written
 * by Philip Hazel (http://www.pcre.org)
 * 
 * (C) 2012 Iain M. Conochie
 * 
 * Part of the cmdb program
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcre.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"

#define OVECCOUNT 18    /* should be a multiple of 3 */

int validate_user_input(char *input, char *regex_test)
{
	pcre *regexp;
	const char *error;
	int input_length, erroffset, valid;
	int ovector[OVECCOUNT];
	
	input_length = (int)strlen(input);
	
	regexp = pcre_compile(
		regex_test,
		0,
		&error,
		&erroffset,
		NULL);
	
	if (!regexp) {
		printf("PCRE compilation failed!\n");
		return -1;
	}
	
	valid = pcre_exec (
		regexp,
		NULL,
		input,
		input_length,
		0,
		0,
		ovector,
		OVECCOUNT);
	
	if (valid < 0) {
		pcre_free(regexp);
		return valid;
	}
	
	if (valid == 0) {
		valid = OVECCOUNT / 3;
		printf("ovector only has room for %d captured substrings\n", valid - 1);
		return 0;
	}
	
	return 1;
}