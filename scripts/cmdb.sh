#!/bin/sh
#
#  Script to generate the initial configurations for the cmdb software
#
#  This includes configuring apache, dhcp and tftp and creating a cmdb.conf file
#  We can also create the initial bind configurations for dnsa
#
#
#  Copyright (C) 2012 - 2013  Iain M Conochie <iain-AT-thargoid.co.uk>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.

#  You should have received a copy of the GNU General Public License along
#  with this program; if not, write to the Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.


#  Check for cmdb user and group; if not create them

create_cmdb_user() {

  getent group | awk -F ':' '{print $1}'|grep cmdb >/dev/null

  if [ $? -eq 0 ]; then
    echo "cmdb group exists. Not adding"
  else
    groupadd -r cmdb
  fi

  id cmdb 2>/dev/null
  if [ $? -eq 0 ]; then
    echo "cmdb user exists. Not adding"
  else
    useradd -g cmdb -d /var/lib/cmdb -s /sbin/nologin -c "CMDB User" -r cmdb
  fi

  if [ ! -d /var/lib/cmdb ]; then
    mkdir /var/lib/cmdb
  fi

  for i in web sql cmdb-bin logs scripts inc
    do mkdir /var/lib/cmdb/${i}
  done
  chown -R cmdb:cmdb /var/lib/cmdb
  chmod -R 770 /var/lib/cmdb

}

get_host_from_user() {

  echo "Please input a host name"
  read HOSTNAME

}

get_domain_from_user() {

  echo "Please input a domain name"
  read DOMAIN

}

get_ip_from_user() {

  echo "Please input the IP address this server will use to build machines"
  read IPADDR
}

parse_command_line() {

  while getopts "h:i:d:s:e:" opt; do
    case $opt in 
      h  ) HOSTNAME=$OPTARG
           ;;
      d  ) DOMAIN=$OPTARG
           ;;
      i  ) IPADDR=$OPTARG
           ;;
      \? ) echo "Usage: $0 [-h hostname] [-d domain] [-i ip address]"
           exit 1
    esac
  done

  if [ -z $HOSTNAME ]; then
    get_host_from_user
  fi
  if [ -z $DOMAIN ]; then
    get_domain_from_user
  fi
  if [ -z $IPADDR ]; then
    get_ip_from_user
  fi
  
}


parse_command_line

echo "host: $HOSTNAME"
echo "domain: $DOMAIN"
echo "IP: $IPADDR"
