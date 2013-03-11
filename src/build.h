/* 
 *
 *  cbc: Create Build Configuration
 *  Copyright (C) 2012 - 2013  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  build.h
 * 
 *  Header file for the build.c file.
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */

#ifndef __CBC_BUILD_H__
# define __CBC_BUILD_H__

int
display_build_config(cbc_config_t *cbt, cbc_comm_line_t *cml);

int
cbc_get_server(cbc_comm_line_t *cml, cbc_t *cbc, cbc_t *details);

int
cbc_get_build_details(cbc_t *cbc, cbc_t *details);

void
print_build_config(cbc_t *details);

#endif /* __CBC_BUILD_H__ */