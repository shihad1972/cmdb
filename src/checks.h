/* checks.h
 * 
 * These are the regex patterns to search against to validate the user input
 * 
 * Uses the PCRE library to do the regex checks.
 * 
 * PCRE library written by Philip Hazel
 * 
 * (C) 2012 Iain M. Conochie
 */
#ifndef __CHECKS_H__
#define __CHECKS_H__

enum {			/* regex search codes */
	UUID_REGEX = 0,
	NAME_REGEX,
	ID_REGEX,
	CUSTOMER_REGEX,
	COID_REGEX,
	MAC_REGEX,
	IP_REGEX,
	DOMAIN_REGEX,
	PATH_REGEX,
	FS_REGEX
};

int
validate_user_input(char *input, int test);

int
add_trailing_slash(char *member);

#endif