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

#include "../config.h"
#ifdef HAVE_LIBPCRE
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <pcre.h>
# include "cmdb.h"

# define OVECCOUNT 18    /* should be a multiple of 3 */

const char *regexps[18] = {
	"^[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{12}$",
	"^[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9]$",
	"^[0-9]+$",
	"^[a-zA-Z0-9]*[a-zA-Z0-9\\-\\_\\ \']*[a-zA-Z0-9]$",
	"^[A-Z0-9]{4,14}[A-Z0-9]$",
	"^[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}$",
	"^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$",
	"^(([a-zA-Z]|[a-zA-Z][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z]|[A-Za-z][A-Za-z0-9\\-]*[A-Za-z0-9])\\.*$",
	"^/([a-zA-Z0-9]*)(/([a-zA-Z0-9]+))*$",
	"^[a-zA-Z0-9]+$",
	"^[a-zA-Z0-9][a-zA-Z0-9\\ \\,\\.\\-\\_]*[a-zA-Z0-9]$",
	"^[a-z][a-z0-9]*$",
	"^[0-9]*\\ [TGM]B$",
	"^[0-9]*\\.[0-9]*$",
	"^[A-Z][A-Z]?[1-9][0-9]?\\ ?[1-9][A-Z]{2}$",
	"^([a-z]{1,8}\\:\\/{2})?(([a-z0-9]*([a-z0-9][\\-\\.])?[a-z0-9])+/?)*$",
	"^\\+?[0-9]*[0-9\\ ]*[0-9]$",
	"^[a-zA-Z0-9]+[a-zA-Z0-9\\.\\_\\-]*@[a-zA-Z0-9]+[a-zA-Z0-9\\.\\-]+[a-zA-Z]*$"
};
int
validate_user_input(char *input, int regex_test)
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

#endif /* HAVE_LIBPCRE */
