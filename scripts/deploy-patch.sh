#!/bin/sh
###########################################
# 
# Deploy the hobbit-patch script to monitor
# OS updates
# 
# (C) 2009 Iain Conochie
#
###########################################

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
        mv hobbit-patch.sh /opt/hobbit/client/ext
        if grep patches /opt/hobbit/client/etc/clientlaunch.cfg; then
            echo "Not adding to /opt/hobbit/client/etc/clientlaunch.cfg. Already there"
        else
            echo "
[patches]
        ENVFILE \$HOBBITCLIENTHOME/etc/hobbitclient.cfg
        CMD \$HOBBITCLIENTHOME/ext/hobbit-patch.sh
        LOGFILE \$HOBBITCLIENTHOME/logs/hobbit-patch.sh
        INTERVAL 6h

" >> /opt/hobbit/client/etc/clientlaunch.cfg
        fi
    elif [ -d /opt/xymon ]; then
        if [ ! -d /opt/xymon/client ]; then
            ln -s /opt/xymon /opt/xymon/client
        fi
        mv hobbit-patch.sh /opt/xymon/client/ext
        if grep patches /opt/xymon/client/etc/clientlaunch.cfg; then
            echo "Not adding to /opt/xymon/client/etc/clientlaunch.cfg. Already there"
        else
            echo "
[patches]
        ENVFILE \$XYMONCLIENTHOME/etc/xymonclient.cfg
        CMD \$XYMONCLIENTHOME/ext/hobbit-patch.sh
        LOGFILE \$XYMONCLIENTLOGS/hobbit-patch.sh
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
    mv hobbit-patch.sh $TOPDIR/client/ext/
    chmod 755 $TOPDIR/client/ext/hobbit-patch.sh
    chown $CLIUSER:$CLIGRP $TOPDIR/client/ext/hobbit-patch.sh

    if ! grep $CLIUSER /etc/sudoers; then

        echo "$CLIUSER  ALL=(ALL) NOPASSWD: /usr/bin/apt-get" >> /etc/sudoers

    fi

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

    if ! grep patches ${VARFILE}; then
        echo "include	${ETCDIR}/clientlaunch.d/patches.cfg" >> $VARFILE
    fi

    if [ ! -f ${ETCDIR}/clientlaunch.d/patches.cfg ]; then
        if [ -d /etc/hobbit ]; then
            echo "
[patches]
        ENVFILE \$HOBBITCLIENTHOME/etc/hobbitclient.cfg
        CMD \$HOBBITCLIENTHOME/ext/hobbit-patch.sh
        LOGFILE \$HOBBITCLIENTHOME/logs/hobbit-patch.sh
        INTERVAL 6h

" >>${ETCDIR}/clientlaunch.d/patches.cfg
        elif [ -d /etc/xymon ]; then
            echo "
[patches]
	ENVFILE \$XYMONCLIENTHOME/etc/xymonclient.cfg
	CMD \$XYMONCLIENTHOME/ext/hobbit-patch.sh
	LOGFILE \$XYMONCLIENTHOME/logs/hobbit-patch.sh
	INTERVAL 6h

" >> ${ETCDIR}/clientlaunch.d/patches.cfg
        else
            echo "Cannot find client directory under /etc"
            echo "And we have already tested for this. Where is it?"
            exit 1
        fi
    fi
fi
