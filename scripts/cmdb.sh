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

  id cmdb >/dev/null 2>&1
  if [ $? -eq 0 ]; then
    echo "cmdb user exists. Not adding"
  else
    useradd -g cmdb -d /var/lib/cmdb -s /sbin/nologin -c "CMDB User" -r cmdb
  fi

  if [ ! -d /var/lib/cmdb ]; then
    mkdir /var/lib/cmdb
  fi

  for i in web sql cmdb-bin logs scripts inc
    do if [ ! -d /var/lib/cmdb/${i} ]; then
      mkdir /var/lib/cmdb/${i}
     fi
  done
  chown -R cmdb:cmdb /var/lib/cmdb
  chmod -R 770 /var/lib/cmdb
}

# Get Hostname

get_host() {
  echo "Please input a host name"
  read HOSTNAME
}

# Get domain name

get_domain() {
  echo "Please input a domain name"
  read DOMAIN
}

# Get IP address

get_ip() {
  echo "Please input the IP address this server will use to build machines"
  read IPADDR
}

# Get Debian Mirror URL

get_deb_mirror() {
  echo "Please input the debian remote mirror you wish to use"
  read DEBMIR
}

# Get CentOS Mirror URL

get_cent_mirror() {
  echo "Please input the centos remote mirror you wish to use"
  read CENTMIR
}

# Get Ubuntu Mirror URL

get_ubun_mirror() {
  echo "Please input the ubuntu remote mirror you wish to use"
  read UBUMIR
}

# Get Fedora Mirror URL

get_fed_mirror() {
  echo "Please input the ubuntu remote mirror you wish to use"
  read FEDMIR
}

get_mirrors() {
  get_cent_mirror
  get_deb_mirror
  get_fed_mirror
  get_ubun_mirror
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
    get_host
  fi

  if [ -z $DOMAIN ]; then
    get_domain
  fi

  if [ -z $IPADDR ]; then
    get_ip
  fi
  
}

create_cmdb_user

parse_command_line

echo "We shall create a web alias for the host ${HOSTNAME}.${DOMAIN}"
echo "The web site will be available under http://${HOSTNAME}.${DOMAIN}/cmdb/"
echo "This is where we shall store the build files."
echo "They will be stored in /var/lib/cmdb/web/"
echo " "
echo "You can also put post-installation scripts into /var/lib/cmdb/scripts"
echo " "

get_mirrors
# Check for OS type

if which apt-get >/dev/null 2>&1; then
  debian_base
elif which yum >/dev/null 2>&1; then
  redhat_base
else
  echo "No yum or apt-get?? What OS are you running?"
  exit 1
fi


