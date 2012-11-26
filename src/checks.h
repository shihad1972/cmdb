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

char uuid_regex[] = "^[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{12}$";
char customer_name_regex[] = "^[a-zA-Z0-9]*[a-zA-Z0-9\\-\\_\\ \']*[a-zA-Z0-9]$";
char name_regex[] = "^[a-zA-Z0-9][a-zA-Z0-9\\-\\_]*[a-zA-Z0-9]$";
char id_regex[] = "^[0-9]+$";
char coid_regex[] = "^[A-Z0-9]{5,7}[A-Z0-9]$";
char mac_regex[] = "^[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}$";
char ip_regex[] = "^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$";

int validate_user_input(char *input, char *test);

#endif