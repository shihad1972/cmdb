#!/bin/bash
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

# Commands to use

APACTL=`which apache2ctl`
APTG=`which apt-get`
DPKG=`which dpkg`
YUM=`which yum`
RPM=`which rpm`
SERV=`which service`

# Files and directories

DHCPF="/etc/dhcp/dhcpd.hosts"
DHCPD="/etc/dhcp/"
BIND="/etc/bind/"

# Options
DB="sqlite"
HAVE_DNSA="yes"
SQL="${PWD}"

DEBMIR="http://mirrors.melbourne.co.uk/debian/dists/"
DEBBASE="/current/images/netboot/debian-installer/"
DEBINST="/main/installer-"
DEBDIST="wheezy"
DEBARCH="amd64 i386"
DEBFILES="linux initrd.gz"

CENTMIR="http://mirrors.melbourne.co.uk/centos/"
CENTBASE="/images/pxeboot/"
CENTVER="5 6"
CENTARCH="i386 x86_64"
CENTFILE="vmlinuz initrd.img"

if [ -d /var/lib/tftpboot/ ]; then
  TFTP="/var/lib/tftpboot/"
  echo "Found $TFTP"
elif [ -d /srv/tftp/ ]; then 
  TFTP="/srv/tftp/"
  echo "Found $TFTP"
else
  unset TFTP
  echo "No tftp directory found"
fi


###############################################################################
#
# Functions Start
#
###############################################################################

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
  echo "Please input the fedora remote mirror you wish to use"
  read FEDMIR
}

get_mirrors() {
  get_cent_mirror
  get_deb_mirror
  get_fed_mirror
  get_ubun_mirror
}

parse_command_line() {

  while getopts "h:i:d:s:e:n" opt; do
    case $opt in 
      h  ) HOSTNAME=$OPTARG
           ;;
      d  ) DOMAIN=$OPTARG
           ;;
      i  ) IPADDR=$OPTARG
           ;;
      n  ) unset HAVE_DNSA
           ;;
      \? ) echo "Usage: $0 [-h hostname] [-d domain] [-i ip address] [ -n ]"
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

  if [ $HAVE_DNSA ]; then
    echo "Installing and configuring bind. If this is not what you want"
    echo "then quit this script and run with the -n option"
    echo "Hit enter to continue"
    read
  else
    echo "Not configuring bind; detected -n option"
  fi
  
}

###############################################################################
#
# Applications Configuration
#
###############################################################################

create_apache_config() {

  echo "We shall create a web alias for the host ${HOSTNAME}.${DOMAIN}"
  echo "The web site will be available under http://${HOSTNAME}.${DOMAIN}/cmdb/"
  echo "This is where we shall store the build files."
  echo "They will be stored in /var/lib/cmdb/web/"
  echo " "
  echo "You can also put post-installation scripts into /var/lib/cmdb/scripts"
  echo " "
  echo "Creating config in ${APACNF}"
  cat >${APACNF}cmdb.conf<<EOF
#
# This is the connfiguration file for the cmdb web portal used
# by the cmdb build system, Muppett
Alias /cmdb/ "/var/lib/cmdb/web/"
<Directory "/var/lib/cmdb/web/">
    Options Indexes FollowSymLinks Includes MultiViews
    Order allow,deny
    Allow from all
</Directory>

ScriptAlias /cmdb-bin/ "/var/lib/cmdb/cgi-bin/"
<Directory "/var/lib/cmdb/cgi-bin/">
    AllowOverride None
    Options ExecCGI Includes
    Order allow,deny
    Allow from all
</Directory>

EOF

}

create_bind_config() {

  if [ ! -d $BIND ]; then
    echo "Bind config directory is not $BIND"
    echo "Please set the real bind config directory at the top of this script"
    return 0
  fi
  echo "Creating bind 9 config in ${BIND}"
  cat >>${BIND}/named.conf<<EOF

include "/etc/bind/dnsa.conf";
include "/etc/bind/dnsa-rev.conf";

EOF

  touch ${BIND}/dnsa.conf ${BIND}/dnsa-rev.conf
  chown cmdb:cmdb ${BIND}/dnsa*
  chmod 664 ${BIND}/dnsa*

}

create_dhcp_config() {

  if [ ! -d $DHCPD ]; then
    echo "Cannot find directory ${DHCPD}! for dhcpd configuration"
    exit 5
  fi
  cd $DHCPD
  mv dhcpd.conf dhcpd.old
  cat > dhcpd.conf<<EOF

  allow booting;

  allow bootp;
                                                                                                                             
  ddns-update-style none;

include "/etc/dhcp/dhcpd.networks";
include "/etc/dhcp/dhcpd.hosts";
EOF
  touch dhcpd.networks dhcpd.hosts
  chmod 664 dhcpd.networks dhcpd.hosts
  chown cmdb:cmdb dhcpd.networks dhcpd.hosts

}

create_tftp_config() {

  if [ ! -d $TFTP ]; then
    echo "$TFTP is not a directory! Exiting.."
    exit 4
  fi

  cd $TFTP
  mkdir pxelinux.cfg
  chmod 664 pxelinux.cfg
  chown cmdb:cmdb pxelinux.cfg
  if echo $CLIENT | grep debian >/dev/null 2>&1; then
    cp /usr/lib/syslinux/pxelinux.0 .
  fi
  echo "Retrieving debian boot files..."
  for i in $DEBDIST
    do for j in $DEBARCH
      do for k in $DEBFILES
        do if echo $k | grep linu >/dev/null 2>&1; then
          wget ${DEBMIR}${i}${DEBINST}${j}${DEBBASE}${j}/${k} -O vmlinuz-debian-${i}-${j}.img \
>/dev/null 2>&1 && echo "Got vmlinuz-debian-${i}-${j}.img"
        elif echo $k | grep initrd >/dev/null 2>&1; then
          wget ${DEBMIR}${i}${DEBINST}${j}${DEBBASE}${j}/${k} -O initrd-debian-${i}-${j}.img \
>/dev/null 2>&1 && echo "Got initrd-debian-${i}-${j}.img"
        fi
      done
    done
  done

  echo "Retrieving centos boot files..."
  for i in $CENTVER
    do for j in $CENTARCH
      do for k in $CENTFILE
        do if echo $k | grep linu >/dev/null 2>&1; then
          wget ${CENTMIR}${i}/os/${j}${CENTBASE}${k} -O vmlinuz-centos-${i}-${j}.img \
>/dev/null 2>&1 && echo "Got vmlinuz-centos-${i}-${j}.img"
        elif echo $k | grep initrd >/dev/null 2>&1; then
          wget ${CENTMIR}${i}/os/${j}${CENTBASE}${k} -O initrd-centos-${i}-${j}.img \
>/dev/null 2>&1 && echo "Got initrd-centos-${i}-${j}.img"
        fi
      done
    done
  done

}

###############################################################################
#
# End of Applications Configuration
#
###############################################################################

###############################################################################
#
# OS Varient configurations
#
###############################################################################

debian_base() {

  CLIENT="debian"
  APACNF="/etc/apache2/conf.d/"
  if [ -z "$APACTL" ]; then
    echo "Installing apache2 package"
    $APTG install apache2 apache2.2-bin libapache2-mod-php5 -y > /dev/null 2>&1
  fi
  echo "Adding www-data to cmdb group"
  echo "If this is not your apache user you will have to do this manually"
  usermod -a -G cmdb www-data

  if [ ! -d "$DHCPD" ]; then
    echo "Installing isc-dhcp-server package"
    $APTG install isc-dhcp-server -y > /dev/null 2>&1
  fi

  if [ ! -d "$TFTP" ]; then
    echo "Installing tftpd-hpa and syslinux packages"
    echo "Please use /srv/tftp for the directory"
    $APTG install tftpd-hpa syslinux -y > /dev/null 2>&1
    TFTP=/srv/tftp
  fi

  if [ $HAVE_DNSA ]; then
    if [ ! -d "$BIND" ]; then
      echo "Installing bind9 package"
      $APTG install bind9 bind9-host -y > /dev/null 2>&1
    fi
  fi

}

redhat_base() {

  CHKCON=`which chkconfig`

  if [ -z "$CHKCON" ]; then
    echo "Cannot find chkconfig. Do you have /sbin in your path?"
    echo "Alternatively install it via yum and run this script again"
    exit 3
  fi

  APACNF="/etc/httpd/conf.d/"

  if [ -z "$APACTL" ]; then
    echo "Installing httpd package"
    $YUM install httpd php5 -y > /dev/null 2>&1
  fi
  echo "Adding apache to cmdb group"
  echo "If this is not your apache user you will have to do this manually"
  usermod -a -G cmdb apache

  if [ ! -d "$DHCPD" ]; then
    echo "Installing dhcp package"
    $YUM install dhcp -y > /dev/null 2>&1
  fi

# Need to check if syslinux is installed cos we need the pxelinux.0 file
# from it

  if [ ! -d "$TFTP" ]; then
    echo "Installing atftp and syslinux package"
    $YUM install atftp syslinux -y > /dev/null 2>&1
    echo "Setting tftp to start and restarting xinetd"
    $CHKCON tftp on
    $SERV xinetd restart
    TFTP="/tftpboot"
  fi

  if [ $HAVE_DNSA ]; then
    if [ ! -d "$BIND" ]; then
      echo "Installing bind package"
      $YUM install bind -y > /dev/null 2>&1
    fi
  fi
}

###############################################################################
#
# End of OS Varient configurations
#
###############################################################################

create_database() {
  if echo $PWD | grep scripts; then > /dev/null 2>&1
    echo "Hopefully you are in the scripts directory off the main tree"
    SQL="${SQL}../sql"
  else
    echo "Assuming you are in the main source directory"
    SQL="${SQL}/sql"
  fi

}

###############################################################################
#
# Functions End
#
###############################################################################

# Need to be root

if [[ $EUID -ne 0 ]]; then
   echo "You must run this script as root" 1>&2
   exit 1
fi

parse_command_line

create_cmdb_user


# Check for OS type

if which apt-get >/dev/null 2>&1; then
  debian_base
elif which yum >/dev/null 2>&1; then
  redhat_base
else
  echo "No yum or apt-get?? What OS are you running?"
  exit 2
fi

create_apache_config

if [ $HAVE_DNSA ]; then
  create_bind_config
fi

create_tftp_config

create_dhcp_config

create_database
