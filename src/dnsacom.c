/* commline.c:
 * 
 * Contains functions to deal with command line arguments and also
 * to read the values from the configuration file.
 *
 * Part of the DNSA  program
 * 
 * (C) Iain M Conochie 2012
 * 
 */

#include "dnsa.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int parse_command_line(int argc, char **argv, comm_line_t *comp)
{
	int i, retval;
	
	retval = 0;

	comp->action = NONE;
	comp->type = NONE;
	strncpy(comp->domain, "NULL", CONF_S);
	strncpy(comp->config, "/etc/dnsa/dnsa.conf", CONF_S);
	
	for (i = 1; i < argc; i++) {
		if ((strncmp(argv[i], "-d", COMM_S) == 0)) {
			comp->action = DISPLAY_ZONE;
		} else if ((strncmp(argv[i], "-w", COMM_S) == 0)) {
			comp->action = WRITE_ZONE;
		} else if ((strncmp(argv[i], "-c", COMM_S) == 0)) {
			comp->action = CONFIGURE_ZONE;
			strncpy(comp->domain, "none", CONF_S);
		} else if ((strncmp(argv[i], "-l", COMM_S) == 0)) {
			comp->action = LIST_ZONES;
			strncpy(comp->domain, "all", CONF_S);
		} else if ((strncmp(argv[i], "-f", COMM_S) == 0)) {
			comp->type = FORWARD_ZONE;
		} else if ((strncmp(argv[i], "-r", COMM_S) == 0)) {
			comp->type = REVERSE_ZONE;
		} else if ((strncmp(argv[i], "-n", COMM_S) == 0)) {
			i++;
			if (i >= argc) 
				retval = -1;
			else
				strncpy(comp->domain, argv[i], CONF_S);
		} else {
			retval = -1;
		}
	}
	
	if (comp->action == NONE)
		retval = -1;
	else if (comp->type == NONE)
		retval = -1;
	else if ((strncmp(comp->domain, "NULL", CONF_S) == 0))
		retval = -1;
	
	return retval;
}

int parse_config_file(dnsa_config_t *dc, char *config)
{
	FILE *cnf;	/* File handle for config file */
	size_t len;
	int retval;
	unsigned long int portno;

	char buff[CONF_S] = "";
	char port[CONF_S] = "";

	dc->port = 3306;
	dc->cliflag = 0;

	if (!(cnf = fopen(config, "r"))) {
		fprintf(stderr, "Cannot open config file %s\n", config);
		fprintf(stderr, "Using default values\n");
		retval = -1;
	} else {
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "PASS=%s", dc->pass);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "HOST=%s", dc->host);	
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "USER=%s", dc->user);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "DB=%s", dc->db);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "DIR=%s", dc->dir);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "BIND=%s", dc->bind);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "REV=%s", dc->rev);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "DNSA=%s", dc->dnsa);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "RNDC=%s", dc->rndc);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "CHKZ=%s", dc->chkz);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "CHKC=%s", dc->chkc);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "SOCKET=%s", dc->socket);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "PORT=%s", port);
		}
		retval = 0;
		fclose(cnf);
	}
	
	/* We need to check the value of portnop before we convert to int.
	 * Obviously we cannot have a port > 65535
	 */
	portno = strtoul(port, NULL, 10);
	if (portno > 65535) {
		retval = -1;
	} else {
		dc->port = (unsigned int) portno;
	}
	
	/* The next 2 values need to be checked for a trailing /
	 * If there is not one then add it
	 */
	sprintf(buff, "%s", dc->dir);
	len = strlen(buff);
	if (buff[len - 1] != '/') {
		buff[len] = '/';
		buff[len + 1] = '\0';
		sprintf(dc->dir, "%s", buff);
	}
	sprintf(buff, "%s", dc->bind);
	len = strlen(buff);
	if (buff[len - 1] != '/') {
		buff[len] = '/';
		buff[len + 1] = '\0';
		sprintf(dc->bind, "%s", buff);
	}
	
	return retval;
}

void init_config_values(dnsa_config_t *dc)
{
	char *buff;
	buff = dc->socket;
	sprintf(dc->db, "bind");
	sprintf(dc->user, "root");
	sprintf(dc->host, "localhost");
	sprintf(dc->pass, "%s", "");
	sprintf(dc->dir, "/var/named/");
	sprintf(dc->bind, "/var/named/");
	sprintf(dc->dnsa, "dnsa.conf");
	sprintf(dc->rev, "dnsa-rev.conf");
	sprintf(dc->rndc, "/usr/sbin/rndc");
	sprintf(dc->chkz, "/usr/sbin/named-checkzone");
	sprintf(dc->chkc, "/usr/sbin/named-checkconf");
	sprintf(buff, "%s", "");
}
