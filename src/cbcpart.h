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
 *  cbcpart.h
 * 
 *  Header file for data and functions for cbcpart program
 * 
 *  part of the cbcpart program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */

#ifndef __CBCPART_H__
# define __CBCPART_H__
# include "cmdb.h"
# include "cbc_data.h"

typedef struct cbcpart_comm_line_s {
	char scheme[CONF_S];
	short int action;
	short int lvm;
} cbcpart_comm_line_s;


#endif /* __CBCPART_H__ */

