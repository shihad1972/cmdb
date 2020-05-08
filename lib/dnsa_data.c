/*
 * 
 *  dnsa: DNS Administration
 *  Copyright (C) 2012 - 2014  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  dnsacom.c:
 *  Contains functions to deal with command line arguments and also
 *  to read the values from the configuration file.
 *
 *  Part of the DNSA program
 * 
 * 
 */
#include <config.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmdb.h"
#include "dnsa_data.h"

void
init_dnsa_struct(dnsa_s *dnsa)
{
	memset(dnsa, 0, sizeof(dnsa_s));
}

void
dnsa_init_config_values(ailsa_cmdb_s *dc)
{
	memset(dc, 0, sizeof(ailsa_cmdb_s));
	dc->file = strdup("/var/lib/cmdb/cmdb.sql");
	dc->dbtype = strdup("sqlite");
	dc->db = strdup("bind");
	dc->user = strdup("root");
	dc->dir = strdup("/var/named/");
	sprintf(dc->bind, "/var/named/");
	sprintf(dc->dnsa, "dnsa.conf");
	sprintf(dc->rev, "dnsa-rev.conf");
	sprintf(dc->rndc, "/usr/sbin/rndc");
	sprintf(dc->chkz, "/usr/sbin/named-checkzone");
	sprintf(dc->chkc, "/usr/sbin/named-checkconf");
}

void
init_zone_struct(zone_info_s *zone)
{
	memset(zone, 0, sizeof(zone_info_s));
	snprintf(zone->name, COMM_S, "NULL");
	snprintf(zone->pri_dns, COMM_S, "NULL");
	snprintf(zone->sec_dns, COMM_S, "NULL");
	snprintf(zone->valid, COMM_S, "NULL");
	snprintf(zone->updated, COMM_S, "NULL");
	snprintf(zone->web_ip, COMM_S, "NULL");
	snprintf(zone->ftp_ip, COMM_S, "NULL");
	snprintf(zone->mail_ip, COMM_S, "NULL");
	snprintf(zone->type, COMM_S, "NULL");
	snprintf(zone->master, COMM_S, "NULL");
}

void
init_rev_zone_struct(rev_zone_info_s *rev)
{
	memset(rev, 0, sizeof(rev_zone_info_s));
	snprintf(rev->net_range, COMM_S, "NULL");
	snprintf(rev->net_start, COMM_S, "NULL");
	snprintf(rev->net_finish, COMM_S, "NULL");
	snprintf(rev->pri_dns, COMM_S, "NULL");
	snprintf(rev->sec_dns, COMM_S, "NULL");
	snprintf(rev->updated, COMM_S, "NULL");
	snprintf(rev->valid, COMM_S, "NULL");
	snprintf(rev->hostmaster, COMM_S, "NULL");
	snprintf(rev->type, COMM_S, "NULL");
	snprintf(rev->master, COMM_S, "NULL");
}

void
init_glue_zone_struct(glue_zone_info_s *glu)
{
	memset(glu, 0, sizeof(glue_zone_info_s));
	snprintf(glu->name, COMM_S, "NULL");
	snprintf(glu->pri_ns, COMM_S, "NULL");
	snprintf(glu->sec_ns, COMM_S, "none");
	snprintf(glu->pri_dns, COMM_S, "NULL");
	snprintf(glu->sec_dns, COMM_S, "none");
}

void
init_record_struct(record_row_s *record)
{
	memset(record, 0, sizeof(record_row_s));
	snprintf(record->dest, COMM_S, "NULL");
	snprintf(record->host, COMM_S, "NULL");
	snprintf(record->type, COMM_S, "NULL");
	snprintf(record->valid, COMM_S, "NULL");
}

void
init_rev_record_struct(rev_record_row_s *rev)
{
	memset(rev, 0, sizeof(rev_record_row_s));
	snprintf(rev->host, COMM_S, "NULL");
	snprintf(rev->dest, COMM_S, "NULL");
	snprintf(rev->valid, COMM_S, "NULL");
}

void
init_preferred_a_struct(preferred_a_s *prefer)
{
	memset(prefer, 0, sizeof(preferred_a_s));
	snprintf(prefer->ip, COMM_S, "NULL");
	snprintf(prefer->fqdn, COMM_S, "NULL");
}

void
dnsa_clean_list(dnsa_s *dnsa)
{
	if (dnsa->zones)
		dnsa_clean_zones(dnsa->zones);
	if (dnsa->rev_zones)
		dnsa_clean_rev_zones(dnsa->rev_zones);
	if (dnsa->records)
		dnsa_clean_records(dnsa->records);
	if (dnsa->rev_records)
		dnsa_clean_rev_records(dnsa->rev_records);
	if (dnsa->prefer)
		dnsa_clean_prefer(dnsa->prefer);
	if (dnsa->glue)
		dnsa_clean_glue(dnsa->glue);
	free(dnsa);
}

void
dnsa_clean_zones(zone_info_s *list)
{
	zone_info_s *zone, *next;

	if (list)
		zone = list;
	else
		return;
	if (zone->next)
		next = zone->next;
	else
		next = NULL;
	while (zone) {
		free(zone);
		if (next)
			zone = next;
		else
			return;
		if (zone->next)
			next = zone->next;
		else
			next = NULL;
	}
}

void
dnsa_clean_rev_zones(rev_zone_info_s *list)
{
	rev_zone_info_s *zone, *next;

	if (list)
		zone = list;
	else
		return;
	if (zone->next)
		next = zone->next;
	else
		next = NULL;
	while (zone) {
		free(zone);
		if (next)
			zone = next;
		else
			return;
		if (zone->next)
			next = zone->next;
		else
			next = NULL;
	}
}

void
dnsa_clean_records(record_row_s *list)
{
	record_row_s *rec, *next;

	if (list)
		rec = list;
	else
		return;
	if (rec->next)
		next = rec->next;
	else
		next = NULL;
	while (rec) {
		free(rec);
		if (next)
			rec = next;
		else
			return;
		if (rec->next)
			next = rec->next;
		else
			next = NULL;
	}
}

void
dnsa_clean_rev_records(rev_record_row_s *list)
{
	rev_record_row_s *rec, *next;

	if (list)
		rec = list;
	else
		return;
	if (rec->next)
		next = rec->next;
	else
		next = NULL;
	while (rec) {
		free(rec);
		if (next)
			rec = next;
		else
			return;
		if (rec->next)
			next = rec->next;
		else
			next = NULL;
	}
}

void
dnsa_clean_prefer(preferred_a_s *list)
{
	preferred_a_s *prefer, *next;

	if (list)
		prefer = list;
	else
		return;
	if (prefer->next)
		next = prefer->next;
	else
		next = NULL;
	while (prefer) {
		free(prefer);
		if (next)
			prefer = next;
		else
			return;
		if (prefer->next)
			next = prefer->next;
		else
			next = NULL;
	}
}

void
dnsa_clean_glue(glue_zone_info_s *list)
{
	glue_zone_info_s *glu, *next;

	if (list)
		glu = list;
	else
		return;
	if (glu->next)
		next = glu->next;
	else
		next = NULL;
	while (glu) {
		free(glu);
		if (next)
			glu = next;
		else
			return;
		if (glu->next)
			next = glu->next;
		else
			next = NULL;
	}
}

