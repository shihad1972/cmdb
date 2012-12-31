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

#define OVECCOUNT 18    /* should be a multiple of 3 */

const char *regexps[10] = {
	"^[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{12}$",
	"^[a-zA-Z0-9][a-zA-Z0-9\\-\\_]*[a-zA-Z0-9]$",
	"^[0-9]+$",
	"^[a-zA-Z0-9]*[a-zA-Z0-9\\-\\_\\ \']*[a-zA-Z0-9]$",
	"^[A-Z0-9]{5,7}[A-Z0-9]$",
	"^[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}$",
	"^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$",
	"^(([a-zA-Z]|[a-zA-Z][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z]|[A-Za-z][A-Za-z0-9\\-]*[A-Za-z0-9])$",
	"^/([a-zA-Z0-9]*)(/([a-zA-Z0-9]+))*$",
	"^[a-zA-Z0-9]+$"
};
int validate_user_input(char *input, int regex_test)
{
	pcre *regexp;
	const char *error;
	int input_length, erroffset, valid;
	int ovector[OVECCOUNT];
	
	input_length = (int)strlen(input);
	
	regexp = pcre_compile(
		regexps[regex_test],
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
		pcre_free(regexp);
		return -1;
	}
	pcre_free(regexp);
	return 1;
}


int add_trailing_slash(char *member)
{
	size_t len;
	int retval;
	
	retval = 0;
	len = strlen(member);
	if ((member[len - 1] != '/') && len < CONF_S) {
		member[len] = '/';
		member[len + 1] = '\0';
	} else if ((member[len - 1] == '/')) {
		retval = NONE;
	} else {
		retval = -1;
	}
	
	return retval;
}
