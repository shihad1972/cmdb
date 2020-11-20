#!/bin/bash
#  Copyright (C) 2020  Iain M Conochie <iain-AT-thargoid.co.uk>
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
#
#  deploy-lspci.sh
#
#  Deploy the lspci.sh monitor script onto a xymon client

VERSION="1.00"

# Check which user  and group we have
if getent passwd hobbit > /dev/null 2>&1; then
    CLIUSER=hobbit
elif getent passwd xymon > /dev/null 2>&1; then
    CLIUSER=xymon
else
    echo "Cannot find client username!"
    exit 1
fi
CLIGRP=`id $CLIUSER | awk '{print $2}' | sed -e s'/gid=[0-9]*//' | sed -e 's/(//' | sed -e 's/)//'`

if [ -f /etc/redhat-release ]; then # Either redhat or centos
    if [ -d /opt/hobbit ]; then
        if [ ! -d /opt/hobbit/client ]; then
            ln -s /opt/hobbit /opt/hobbit/client
        fi
        mv lspci.sh /opt/hobbit/ext/client
        if grep lspci /opt/hobbit/client/etc/clientlaunch.cfg; then
            echo "Not adding to /opt/hobbit/client/etc/clientlaunch.cfg. Already there"
        else
            echo "
[lspci]
        ENVFILE \$HOBBITCLIENTHOME/etc/hobbitclient.cfg
        CMD \$HOBBITCLIENTHOME/ext/lspci.sh
        LOGFILE \$HOBBITCLIENTHOME/logs/lspci.sh
        INTERVAL 6h

" >> /opt/hobbit/client/etc/clientlaunch.cfg
        fi
    elif [ -d /opt/xymon ]; then
        if [ ! -d /opt/xymon/client ]; then
            ln -s /opt/xymon /opt/xymon/client
        fi
        mv lspci.sh /opt/xymon/client/ext
        if grep lspci /opt/xymon/client/etc/clientlaunch.cfg; then
            echo "Not adding to /opt/xymon/client/etc/clientlaunch.cfg. Already there"
        else
            echo "
[lspci]
        ENVFILE \$XYMONCLIENTHOME/etc/xymonclient.cfg
        CMD \$XYMONCLIENTHOME/ext/lspci.sh
        LOGFILE \$XYMONCLIENTLOGS/lspci.log
        INTERVAL 6h

" >> /opt/xymon/client/etc/clientlaunch.cfg
        fi
    else
        echo "Cannot find /opt/hobbit or /opt/xymon"
        exit 1
    fi
elif [ -f /etc/lsb-release ] || [ -f /etc/debian_version ]; then

    if [ -d /usr/lib/hobbit ]; then
        TOPDIR=/usr/lib/hobbit
    elif [ -d /usr/lib/xymon ]; then
        TOPDIR=/usr/lib/xymon
    else
        echo "Cannot find client installation!"
        exit 1
    fi
    mv lspci.sh $TOPDIR/client/ext/
    chmod 755 $TOPDIR/client/ext/lspci.sh
    chown $CLIUSER:$CLIGRP $TOPDIR/client/ext/lspci.sh

    if [ -f /var/run/hobbit/hobbitclient-include.cfg ]; then
        VARFILE=/var/run/hobbit/hobbitclient-include.cfg
    elif [ -f /var/run/xymon/xymonclient-include.cfg ]; then
        VARFILE=/var/run/xymon/xymonclient-include.cfg
    else
        echo "Cannot find client directory under /var!"
        exit 1
    fi

    if [ -d /etc/hobbit ]; then
        ETCDIR=/etc/hobbit
    elif [ -d /etc/xymon ]; then
        ETCDIR=/etc/xymon
    else
        echo "Cannot find client directory under /etc!"
        exit 1
    fi

    if ! grep lspci ${VARFILE}; then
        echo "include	${ETCDIR}/clientlaunch.d/lspci.cfg" >> $VARFILE
    fi

    if [ ! -f ${ETCDIR}/clientlaunch.d/lspci.cfg ]; then
        if [ -d /etc/hobbit ]; then
            echo "
[lspci]
        ENVFILE \$HOBBITCLIENTHOME/etc/hobbitclient.cfg
        CMD \$HOBBITCLIENTHOME/ext/lspci.sh
        LOGFILE \$HOBBITCLIENTHOME/logs/lspci.sh
        INTERVAL 6h

" >>${ETCDIR}/clientlaunch.d/lspci.cfg
        elif [ -d /etc/xymon ]; then
            echo "
[lspci]
	ENVFILE \$XYMONCLIENTHOME/etc/xymonclient.cfg
	CMD \$XYMONCLIENTHOME/ext/lspci.sh
	LOGFILE \$XYMONCLIENTHOME/logs/lspci.sh
	INTERVAL 6h

" >> ${ETCDIR}/clientlaunch.d/lspci.cfg
        else
            echo "Cannot find client directory under /etc"
            echo "And we have already tested for this. Where is it?"
            exit 1
        fi
    fi
fi
