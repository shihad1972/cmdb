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

APACTL=`which apache2ctl 2>/dev/null`
APTG=`which apt-get 2>/dev/null`
DPKG=`which dpkg 2>/dev/null`
YUM=`which yum 2>/dev/null`
RPM=`which rpm 2>/dev/null`
SERV=`which service 2>/dev/null`
SQLITE=`which sqlite3 2>/dev/null`
MYSQL=`which mysql 2>/dev/null`

# Files and directories

DHCPF="/etc/dhcp/dhcpd.hosts"
DHCPD="/etc/dhcp/"
MIRROR="mirrors.melbourne.co.uk"

# Options
HAVE_DNSA="yes"
SQL="/var/lib/cmdb/sql"
SQLFILE="${SQL}/cmdb.sql"
DB="sqlite"
DBNAME="cmdb"

DEBBASE="/current/images/netboot/debian-installer/"
DEBINST="/main/installer-"
DEBDIST="wheezy"
DEBARCH="amd64 i386"
DEBFILES="linux initrd.gz"

CENTBASE="/images/pxeboot/"
CENTVER="5 6"
CENTARCH="i386 x86_64"
CENTFILE="vmlinuz initrd.img"

UBUBASE="/current/images/netboot/ubuntu-installer/"
UBUINST="/main/installer-"
UBUDIST="precise quantal raring"
UBUARCH="amd64 i386"
UBUFILE="linux initrd.gz"

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

if [ -d /etc/bind ]; then
  BIND="/etc/bind/"
  echo "Found ${BIND}"
elif [ -d /var/named ]; then
  BIND="/var/named"
  echo "Found ${BIND}"
else
  unset BIND
  echo "No bind directory found"
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
  chmod g+s /var/lib/cmdb/*
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


###############################################################################
#
# Applications Configuration
#
###############################################################################

create_apache_config() {

  if [ -f ${APACNF}cmdb.conf ]; then
    echo "Apache already configured. Skipping"
    return
  fi

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

Alias /cmdb/scripts/ "/var/lib/cmdb/scripts/"
<Directory "/var/lib/cmdb/scripts/">
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
  if [ -f dhcpd.conf ]; then
    mv dhcpd.conf dhcpd.old
  fi
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
  if [ ! -d pxelinux.cfg ]; then
    mkdir pxelinux.cfg
  fi
  chmod 664 pxelinux.cfg
  chown cmdb:cmdb pxelinux.cfg
  if [ ! -f pxelinux.0 ]; then
    if echo $CLIENT | grep debian >/dev/null 2>&1; then
      cp /usr/lib/syslinux/pxelinux.0 .
    fi
  fi
  echo "Retrieving debian boot files..."
  for i in $DEBDIST
    do for j in $DEBARCH
      do for k in $DEBFILES
        do if echo $k | grep linu >/dev/null 2>&1; then
          if [ ! -f vmlinuz-debian-${i}-${j} ]; then
            wget ${DEBMIRR}${i}${DEBINST}${j}${DEBBASE}${j}/${k} -O vmlinuz-debian-${i}-${j} \
>/dev/null 2>&1 && echo "Got vmlinuz-debian-${i}-${j}"
          else
            echo "Skipping vmlinuz-debian-${i}-${j}. Exists"
          fi
        elif echo $k | grep initrd >/dev/null 2>&1; then
          if [ ! -f initrd-debian-${i}-${j}.img ]; then
            wget ${DEBMIRR}${i}${DEBINST}${j}${DEBBASE}${j}/${k} -O initrd-debian-${i}-${j}.img \
>/dev/null 2>&1 && echo "Got initrd-debian-${i}-${j}.img"
          else
            echo "Skipping initrd-debian-${i}-${j}.img. Exists"
          fi
        fi
      done
    done
  done

  echo "Retrieving centos boot files..."
  for i in $CENTVER
    do for j in $CENTARCH
      do for k in $CENTFILE
        do if echo $k | grep linu >/dev/null 2>&1; then
          if [ ! -f vmlinuz-centos-${i}-${j} ]; then
            wget ${CENTMIRR}${i}/os/${j}${CENTBASE}${k} -O vmlinuz-centos-${i}-${j} \
>/dev/null 2>&1 && echo "Got vmlinuz-centos-${i}-${j}"
          else
            echo "Skipping vmlinuz-centos-${i}-${j}. Exists"
          fi
        elif echo $k | grep initrd >/dev/null 2>&1; then
          if [ ! -f initrd-centos-${i}-${j}.img ]; then
            wget ${CENTMIRR}${i}/os/${j}${CENTBASE}${k} -O initrd-centos-${i}-${j}.img \
>/dev/null 2>&1 && echo "Got initrd-centos-${i}-${j}.img"
          else
            echo "Skipping initrd-centos-${i}-${j}.img. Exists"
          fi
        fi
      done
    done
  done

  echo "Retrieving ubuntu boot files..."
  for i in $UBUDIST
    do for j in $UBUARCH
      do for k in $UBUFILE
        do if echo $k | grep linu > /dev/null 2>&1; then
          if [ ! -f vmlinuz-ubuntu-${i}-${j} ]; then
            wget ${UBUMIRR}${i}${UBUINST}${j}${UBUBASE}${j}/${k} -O vmlinuz-ubuntu-${i}-${j} \
>/dev/null 2>&1 && echo "Got vmlinuz-ubuntu-${i}-${j}"
          else
            echo "Skipping vmlinuz-ubuntu-${i}-${j}. Exists"
          fi
        elif echo $k | grep initrd > /dev/null 2>&1; then
          if [ ! -f initrd-ubuntu-${i}-${j}.img ]; then
            wget ${UBUMIRR}${i}${UBUINST}${j}${UBUBASE}${j}/${k} -O initrd-ubuntu-${i}-${j}.img \
>/dev/null 2>&1 && echo "Got initrd-ubuntu-${i}-${j}.img"
          else
            echo "Skipping initrd-ubuntu-${i}-${j}.img. Exists"
          fi
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
  if ! id www-data | grep cmdb > /dev/null 2>&1; then
    echo "Adding www-data to cmdb group"
    echo "If this is not your apache user you will have to do this manually"
    usermod -a -G cmdb www-data
  else
    echo "www-data already in cmdb group"
  fi

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

  if echo $DB | grep sqlite > /dev/null 2>&1; then
    if [ -z $SQLITE ]; then
      echo "Installing sqlite3 command"
      apt-get install -y sqlite3 > /dev/null 2>&1
    fi
  elif echo $DB | grep mysql; then
    if [ -z $MYSQL ]; then
      echo "Install mysql command"
      apt-get install -y mysql-client > /dev/null 2>&1
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

  if ! id apache | grep cmdb > /dev/null 2>&1; then
    echo "Adding apache to cmdb group"
    echo "If this is not your apache user you will have to do this manually"
    usermod -a -G cmdb apache
  else
    echo "apache already in cmdb group"
  fi

  if [ ! -d "$DHCPD" ]; then
    echo "Installing dhcp package"
    $YUM install dhcp -y > /dev/null 2>&1
  fi

# Need to check if syslinux is installed cos we need the pxelinux.0 file
# from it

  if [ ! -d "$TFTP" ]; then
    echo "Installing tftp-server and syslinux package"
    $YUM install tftp-server syslinux -y > /dev/null 2>&1
    echo "Setting tftp to start and restarting xinetd"
    $CHKCON xinetd on
    $SERV xinetd restart
    TFTP="/var/lib/tftpboot"
  fi

  if [ $HAVE_DNSA ]; then
    if [ ! -d "$BIND" ]; then
      echo "Installing bind package"
      $YUM install bind -y > /dev/null 2>&1
      BIND="/var/named"
    fi
  fi

  if echo $DB | grep sqlite; then
    if [ -z $SQLITE ]; then
      echo "Installing sqlite3 command"
      yum install sqlite -y> /dev/null 2>&1
    fi
  elif echo $DB | grep mysql; then
    if [ -z $MYSQL ]; then
      echo "Install mysql command"
      yum install mysql -y> /dev/null 2>&1
    fi
  fi
}

###############################################################################
#
# End of OS Varient configurations
#
###############################################################################

create_database() {

  if echo $DB | grep sqlite > /dev/null 2>&1; then
    if [ -z $SQLITE ]; then
      echo "No sqlite3 command. Exiting"
      exit 6
    fi
    SQLBASE=$SQL/all-tables-sqlite.sql
    $SQLITE -init $SQLBASE $SQLFILE <<STOP
.quit
STOP
    if [ -z $DEBMIR ]; then
      $SQLITE $SQLFILE <<STOP
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES ("debian", "preseed", "url", "http://${HOSTNAME}.${DOMAIN}/cmdb/", "$MIRROR", "auto=true priority=critical vga=788");
.quit
STOP
    else
      $SQLITE $SQLFILE <<STOP
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES ("debian", "preseed", "url", "http://${HOSTNAME}.${DOMAIN}/cmdb/", "$DEBMIR", "auto=true priority=critical vga=788");
.quit
STOP
    fi
    if [ -z $UBUMIR ]; then
      $SQLITE $SQLFILE <<STOP
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES ("ubuntu", "preseed", "url", "http://${HOSTNAME}.${DOMAIN}/cmdb/", "$MIRROR", "auto=true priority=critical vga=788");
.quit
STOP
    else
      $SQLITE $SQLFILE <<STOP
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES ("ubuntu", "preseed", "url", "http://${HOSTNAME}.${DOMAIN}/cmdb/", "$UBUMIR", "auto=true priority=critical vga=788");
.quit
STOP
    fi
    if [ -z $CENTMIR ]; then
      $SQLITE $SQLFILE <<STOP
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES ("centos", "kickstart", "ks", "http://${HOSTNAME}.${DOMAIN}/cmdb/", "$MIRROR", "ksdevice=eth0 console=tty0 ramdisk_size=8192");
.quit
STOP
    else
      $SQLITE $SQLFILE <<STOP
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES ("centos", "kickstart", "ks", "http://${HOSTNAME}.${DOMAIN}/cmdb/", "$CENTMIR", "ksdevice=eth0 console=tty0 ramdisk_size=8192");
.quit
STOP
    fi
    cat ${SQL}/initial.sql | $SQLITE $SQLFILE
    chown cmdb:cmdb $SQLFILE
  elif echo $DB | grep mysql > /dev/null 2>&1; then
    SQLBASE=$SQL/all-tables-mysql.sql
    echo "Please enter the name of the mysql host"
    read MYSQLHOST
    echo "Please enter the root password for $MYSQLHOST"
    read MYSQLPASS
    if $MYSQL -h $MYSQLHOST -p${MYSQLPASS} -u root -e "SHOW DATABASES" > /dev/null 2>&1; then
      echo "Connection successful"
    else
      echo "Cannot connect to mysql host. Are you sure root has access to ${MYSQLHOST}?"
      exit 8
    fi
    echo "Creating db $DBNAME..."
    $MYSQL -h $MYSQLHOST -p${MYSQLPASS} -u root -e "CREATE DATABASE $DBNAME"
    echo "Adding initial entries to DB $DBNAME"
    cat $SQLBASE | $MYSQL -h $MYSQLHOST -p${MYSQLPASS} -u root $DBNAME -e
    if [ -z $DEBMIR ]; then
      $MYSQL -u root -p${MYSQLPASS} -h $MYSQLHOST $DBNAME -e <<STOP
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES ("debian", "preseed", "url", "http://${HOSTNAME}.${DOMAIN}/cmdb/", "$MIRROR", "auto=true priority=critical vga=788");
STOP
    else
      $MYSQL -u root -p${MYSQLPASS} -h $MYSQLHOST $DBNAME -e <<STOP
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES ("debian", "preseed", "url", "http://${HOSTNAME}.${DOMAIN}/cmdb/", "$DEBMIR", "auto=true priority=critical vga=788");
STOP
    fi
    if [ -z $UBUMIR ]; then
      $MYSQL -u root -p${MYSQLPASS} -h $MYSQLHOST $DBNAME -e <<STOP
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES ("ubuntu", "preseed", "url", "http://${HOSTNAME}.${DOMAIN}/cmdb/", "$MIRROR", "auto=true priority=critical vga=788");
STOP
    else
      $MYSQL -u root -p${MYSQLPASS} -h $MYSQLHOST $DBNAME -e <<STOP
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES ("ubuntu", "preseed", "url", "http://${HOSTNAME}.${DOMAIN}/cmdb/", "$UBUMIR", "auto=true priority=critical vga=788");
STOP
    fi
    if [ -z $CENTMIR ]; then
      $MYSQL -u root -p${MYSQLPASS} -h $MYSQLHOST $DBNAME -e <<STOP
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES ("centos", "kickstart", "ks", "http://${HOSTNAME}.${DOMAIN}/cmdb/", "$MIRROR", "ksdevice=eth0 console=tty0 ramdisk_size=8192");
STOP
    else
      $MYSQL -u root -p${MYSQLPASS} -h $MYSQLHOST $DBNAME -e <<STOP
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES ("centos", "kickstart", "ks", "http://${HOSTNAME}.${DOMAIN}/cmdb/", "$CENTMIR", "ksdevice=eth0 console=tty0 ramdisk_size=8192");
STOP
    fi
    cat ${SQL}/initial.sql | mysql -h ${MYSQLHOST} -p${MYSQLPASS} $DBNAME -e
  else
    echo "Unknown database type $SQL"
    exit 6
  fi
}

create_config()
{
  if [ ! -d /etc/dnsa ]; then
    echo "Creating directory /etc/dnsa"
    mkdir /etc/dnsa
  else
    echo "/etc/dnsa exists. Continuing"
  fi
  
  if [ ! -f /etc/dnsa/dnsa.conf ]; then
    cat >/etc/dnsa/dnsa.conf <<FINISH
## DB Driver
DBTYPE=$DB

## SQLITE PATH
FILE=$SQLFILE

## MYSQL DB Connectivity
DB=$DBNAME		# Database
USER=cmdb		# DB user
PASS=			# DB pass
HOST=$MYSQLHOST		# DB host
PORT=3306

## DNSA Settings

DIR=/etc/bind/db/		# BIND data directory for zone files
BIND=/etc/bind/			# BIND configuration directory
DNSA=dnsa.conf			# DNSA configuration filename for bind
REV=dnsa-rev.conf		# Reverse zone configuration file for bind
RNDC=/usr/sbin/rndc		# Path to rndc command
CHKZ=/usr/sbin/named-checkzone	# Path to checkzone command
CHKC=/usr/sbin/named-checkconf	# Path to checkconf command
REFRESH=28800
RETRY=7200
EXPIRE=1209600
TTL=86400
PRIDNS=
SECDNS=
HOSTMASTER=
PRINS=
SECNS=

## CBC settings
TMPDIR=/tmp/cmdb
TFTPDIR=/srv/tftp/
PXE=pxelinux.cfg
TOPLEVELOS=/usr/local/zips
PRESEED=preseed/
KICKSTART=ks/
DHCPCONF=/etc/dhcp/dhcpd.hosts

FINISH
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


if [ ! -f ${SQL}/initial.sql ]; then
  echo "Cannot find SQL initialisation file $SQL"
  echo "Please run this script from the top level of the cmdb source directory"
  exit 7
fi


while getopts "b:d:h:i:nm:" opt; do
  case $opt in 
    b  ) DB=$OPTARG
         echo "Setting DB"
         ;;
    d  ) DOMAIN=$OPTARG
         echo "Setting Domain"
         ;;
    h  ) HOSTNAME=$OPTARG
         echo "Setting Hostname"
         ;;
    i  ) IPADDR=$OPTARG
         echo "Setting IP address"
         ;;
    n  ) unset HAVE_DNSA
         ;;
    m  ) MIRROR=$OPTARG
         echo "Setting mirror"
         ;;
    \? ) echo "Usage: $0 -h hostname -d domain -i ip address -b dbtype -m mirror [ -n ]"
         exit 1
  esac
done

if [ -z $HOSTNAME ]; then
  get_host
else
  echo "Using $HOSTNAME as hostname: Change? (Y/N)"
  read answer
  if [ $answer != "y" && $answer != 'Y' ]; then
    get_host
  fi
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

CENTMIRR="http://${MIRROR}/centos/"
DEBMIRR="http://${MIRROR}/debian/dists/"
UBUMIRR="http://${MIRROR}/ubuntu/dists/"

create_cmdb_user


# Check for OS type

if [ ! -z $APTG ]; then
  debian_base
elif [ ! -z $YUM ]; then
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

create_config
