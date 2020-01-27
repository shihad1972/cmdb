/*
 *
 *  mksp : make storgae pool
 *  Copyright (C) 2018 - 2020  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  mksp.c
 *
 *  main and command line parsing functions for mksp
 *
 */

#include <config.h>
#include <configmake.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif // HAVE_GETOPT_H
#ifdef HAVE_REGEX_H
# include <regex.h>
#endif // HAVE_REGEX_H
#ifdef HAVE_WORDEXP_H
# include <wordexp.h>
#endif // HAVE_WORDEXP_H
#include <ailsacmdb.h>
#include "virtual.h"

int
main(int argc, char *argv[])
{
	int retval = 0;
	ailsa_mkvm_s *vm = ailsa_calloc(sizeof(ailsa_mkvm_s), "vm in main");

	parse_mkvm_config(vm);
	if ((retval = parse_mksp_command_line(argc, argv, vm)) != 0)
		goto cleanup;
	switch (vm->action) {
	case AILSA_ADD:
		retval = mksp_create_storage_pool(vm);
		break;
	case AILSA_HELP:
		display_mksp_usage();
		break;
	case AILSA_VERSION:
		display_version(argv[0]);
		break;
	}
	cleanup:
		ailsa_show_error(retval);
		if (retval > 0)
			display_mksp_usage();
		ailsa_clean_mkvm((void *)vm);
		return retval;
}
