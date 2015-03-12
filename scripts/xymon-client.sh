#!/bin/sh
#
#
# Script to install the hobbit-client
#
# (C) 2009 - 2013 Iain Conochie
#
# Needs to be run as root!!
#
# SYNOPSIS:
# hobbit-client host url xymon-server-ip
#

HOST=$1
URL=$2
IP=$3

# Version of the client.
VER=4.3.18

[ -n $HOST ] || echo "No host set"
[ -n $URL ] || echo "No url set"
[ -n $VER ] || echo "No version set"

# Check for xymon group; if none create
getent group xymon > /dev/null || groupadd -g 1984 xymon
#RETVAL=$?
#[ $RETVAL -eq 0 ] || groupadd -g 1984 xymon

# Check for xymon user; if none create
getent passwd xymon > /dev/null || useradd -u 1984 -g 1984 -d /opt/xymon -m -s /bin/bash xymon
#RETVAL=$?
#[ $RETVAL -eq 0 ] || useradd -u 1984 -g 1984 -d /opt/xymon -m -s /bin/bash xymon

if [ -d /target ]; then
  echo "Running from within debian install. No Go!"
  exit 1
fi

cd /root
wget ${URL}/xymon-${VER}.tar.gz
tar xf xymon-${VER}.tar.gz
cd xymon-${VER}

if [ -x /usr/bin/yum ]; then
  /usr/bin/yum -y install gcc make
elif [ -x /usr/bin/apt-get ]; then
  /usr/bin/apt-get install -y build_essential
else
  echo "Cannot find yum or apt-get. If no gcc and make this will fail"
fi

cat > Makefile <<EOF
# Toplevel Makefile for Xymon
BUILDTOPDIR=`pwd`
CLIENTONLY = yes
LOCALCLIENT = no

# configure settings for Xymon
#
# Toplevel dir
XYMONTOPDIR = /opt/xymon/client

# Xymon settings follows
#
# Username running Xymon
XYMONUSER = xymon
# Xymon server IP-address
XYMONHOSTIP = $IP
# Large File Support settings
LFSDEF = -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64
LIBRTDEF = -lrt

include build/Makefile.Linux


#
# Add local CFLAGS etc. settings here

include build/Makefile.rules
EOF

make > /dev/null
make install > /dev/null

cat > /etc/default/xymon-client <<EOF

# Configure the Xymon client settings.

# You MUST set the list of Xymon servers that this
# client reports to.
# It is good to use IP-adresses here instead of DNS
# names - DNS might not work if there's a problem.
#
# E.g. (a single Hobbit server)
#   HOBBITSERVERS="192.168.1.1"
# or (multiple servers)
#   HOBBITSERVERS="10.0.0.1 192.168.1.1"

XYMONSERVERS="$IP"

# The defaults usually suffice for the rest of this file,
# but you can tweak the hostname that the client reports
# data with, and the OS name used (typically needed only on
# RHEL or RHAS servers).

CLIENTHOSTNAME="$HOST"
# CLIENTOS="rhel3"

EOF

cat rpm/xymon-client.init | sed -e 's/DAEMON=\/usr\/lib\/xymon\/client\/runclient.sh/DAEMON=\/opt\/xymon\/client\/runclient.sh/' > /etc/init.d/xymon-client

chmod 755 /etc/init.d/xymon-client
chmod 644 /etc/default/xymon-client

if [ `which chkconfig` ]; then
    chkconfig --add xymon-client
    service xymon-client start
else
    update-rc.d xymon-client defaults
    /etc/init.d/xymon-client start
fi

cd /root

rm -rvf /usr/local/src/xymon*
#rm -rvf xymon*

