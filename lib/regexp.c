/*
 *  (C) 2015 Iain M. Conochie
 *  Part of the cmdb suite of programs
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
 * 
 *  Part of the cmdb program
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <syslog.h>
#include <regex.h>
#include <ailsacmdb.h>
#include "cmdb.h"

const char *regexps[] = {
	"^[a-fA-F0-9]{8}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{4}-[a-fA-F0-9]{12}$",
	"^[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9]$",
	"^[0-9]+$",
	"^[a-zA-Z0-9]*[a-zA-Z0-9\\_\\ \\'\\.\\-]*[a-zA-Z0-9]$",
	"^[A-Z0-9]{4,14}[A-Z0-9]$",
	"^[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}:[a-f0-9]{2}$",
	"^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$",
	"^(([a-zA-Z]|[a-zA-Z][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z]|[A-Za-z][A-Za-z0-9\\-]*[A-Za-z0-9])\\.*$",
	"^/([a-zA-Z0-9]*)(/([a-zA-Z0-9]+))*$",
	"^[a-zA-Z0-9]*[a-zA-Z0-9\\_]*[a-zA-Z0-9]$",
	"^[a-zA-Z0-9]+$",
	"^[a-zA-Z0-9][a-zA-Z0-9\\ \\,\\.\\_\\-]*[a-zA-Z0-9]$",
	"^[a-z][a-z0-9]*$",
	"^[0-9]*\\ [TGM]B$",
	"^[0-9]*\\.?[0-9]*$",
	"^[A-Z][A-Z]?[1-9][0-9]?\\ ?[1-9][A-Z]{2}$",
	"^([a-z]{1,8}\\:\\/{2})?(([a-z0-9]*([a-z0-9][\\-\\.])?[a-z0-9])+/?)*$",
	"^\\+?[0-9]*[0-9\\ ]*[0-9]$",
	"^[a-zA-Z0-9]+[a-zA-Z0-9\\.\\_\\-]*@[a-zA-Z0-9]+[a-zA-Z0-9\\.\\-]+[a-zA-Z]*$",
	"^[a-zA-Z0-9][a-zA-Z0-9\\ \\,\\.\\-\\_\\:]*[a-zA-Z0-9]$",
	"^cn\\=[a-zA-Z0-9]*(\\,dc\\=([a-zA-Z]|[a-zA-Z][a-zA-Z0-9\\-]*[a-zA-Z0-9]))*$",
	"^dc\\=([a-zA-Z]|[a-zA-Z][a-zA-Z0-9\\-]*[a-zA-Z0-9])(\\,dc\\=([a-zA-Z]|[a-zA-Z][a-zA-Z0-9\\-]*[a-zA-Z0-9]))*$",
	"^[a-zA-Z]*[a-zA-Z0-9]*[\\-/]*[a-zA-Z0-9]*$",
	"^[a-zA-Z]+[a-zA-Z0-9]*[\\-]?[a-zA-Z0-9]*/[a-zA-Z0-9]+([\\_\\-]?[a-zA-Z0-9])*$",
	"^[a-zA-Z0-9]+$",
	"^[a-zA-Z0-9%]+[a-zA-Z0-9\\ \\,\\@\\.]+$"
};

int
ailsa_validate_input(char *input, int test)
{
	regex_t re;
	regmatch_t pos[1];
	const char *re_test = regexps[test];
	char *errstr = NULL;
	size_t len;
	int fl = REG_EXTENDED;
	int retval;

	if ((retval = regcomp(&re, re_test, fl)) != 0) {
		len = regerror(retval, &re, NULL, 0);
		errstr = ailsa_calloc(len, "errstr in ailsa_validate_input");
		(void) regerror(retval, &re, errstr, len);
		syslog(LOG_ALERT, "Regex compile failed: %s", errstr);
		free(errstr);
		return retval;
	}
	if ((retval = regexec(&re, input, 1, pos, 0)) != 0)
		retval = -1;
	regfree(&re);
	return retval;
}

int
ailsa_validate_string(const char *input, const char *re_test)
{
	regex_t re;
	regmatch_t pos[1];
	char *errstr = NULL;
	size_t len;
	int fl = REG_EXTENDED;
	int retval;

	if ((retval = regcomp(&re, re_test, fl)) != 0) {
		len = regerror(retval, &re, NULL, 0);
		errstr = ailsa_calloc(len, "errstr in ailsa_validate_string");
		(void) regerror(retval, &re, errstr, len);
		syslog(LOG_ALERT, "Regex compile failed: %s", errstr);
		free(errstr);
		return retval;
	}
	if ((retval = regexec(&re, input, 1, pos, 0)) != 0)
		retval = -1;
	regfree(&re);
	return retval;
}

const char *
ailsa_regex_error(int error)
{
	switch(error) {
	case UUID_REGEX_ERROR:
		return "UUID failed validation";
		break;
	case NAME_REGEX_ERROR:
		return "Name failed validation";
		break;
	case ID_REGEX_ERROR:
		return "id failed validation";
		break;
	case CUSTOMER_REGEX_ERROR:
		return "Customer name failed validation";
		break;
	case COID_REGEX_ERROR:
		return "COID failed validation";
		break;
	case MAC_REGEX_ERROR:
		return "MAC failed validation";
		break;
	case IP_REGEX_ERROR:
		return "IP failed validation";
		break;
	case DOMAIN_REGEX_ERROR:
		return "Domain name failed validation";
		break;
	case FS_REGEX_ERROR:
		return "File system failed validation";
		break;
	default:
		return "Unknown regex failed validation";
		break;
	}
	return "Should never get here! ailsa_regex_error failed";
}
