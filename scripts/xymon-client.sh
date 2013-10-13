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
VER=4.3.12

[ -n $HOST ] || echo "No host set"
[ -n $URL ] || echo "No url set"
[ -n $VER ] || echo "No version set"

getent group xymon > /dev/null
RETVAL=$?
[ $RETVAL -eq 0 ] || groupadd -g 1984 xymon

getent passwd xymon > /dev/null
RETVAL=$?
[ $RETVAL -eq 0 ] || useradd -u 1984 -g 1984 -d /opt/xymon -m -s /bin/bash xymon

if [ -d /target ]; then
  echo "Running from within debian install. No Go!"
  exit 1
fi

cd /root
wget ${URL}/xymon-${VER}.tar.gz
tar xf xymon-${VER}.tar.gz
cd xymon-${VER}

if [ -x /usr/bin/yum ]; then
  /usr/bin/yum -y install gcc
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
XYMONTOPDIR = /opt/xymon
# Server home dir for etc/, www/
XYMONHOME = /opt/xymon/client

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

cat > /etc/init.d/xymon-client <<EOF
#!/bin/sh
#----------------------------------------------------------------------------#
# Xymon client init.d script.                                                #
#                                                                            #
# This invokes xymonlaunch, which in turn runs the Xymon client and any      #
# extensions configured.                                                     #
#                                                                            #
# Copyright (C) 2005-2010 Henrik Storner <henrik@hswn.dk>                    #
# "status" section (C) Scott Smith 2006                                      #
#                                                                            #
# Integration of runclient.sh and xymon-client.initd by Japheth Cleaver 2011 #
#                                                                            #
# This program is released under the GNU General Public License (GPL),       #
# version 2. See the file "COPYING" for details.                             #
#                                                                            #
#----------------------------------------------------------------------------#
#
# chkconfig: 2345 80 20
# description: Xymon is a network monitoring tool that can monitor hosts \\
#               and services. The client reports local system statistics \\
#               (cpu, memory, disk, etc) to Xymon server.
#
# processname: xymonlaunch
# pidfile: /var/run/xymon/xymonlaunch.pid
# config: /etc/xymon-client/xymonclient.cfg
# config: /etc/xymon-client/clientlaunch.cfg autoreload
#
### BEGIN INIT INFO  
# Provides: xymon-client
# Default-Start: 2 3 4 5
# Default-Stop: 0 1 6
# Required-Start: \$local_fs \$network
# Required-Stop: \$local_fs \$network
# Short-Description: start and stop the xymon client
# Description: Xymon is a network monitoring tool that can monitor hosts
#               and services. The client reports local system statistics
#               (cpu, memory, disk, etc) to Xymon server.
### END INIT INFO

NAME=xymon-client
DESC=xymon-client
prog=xymonlaunch
user=xymon

# Source function library.
. /etc/rc.d/init.d/functions || exit 1


# Potentially edited at install-time
XYMONCLIENTHOME="/opt/xymon/client"
DAEMON="/opt/xymon/client/bin/xymonlaunch"
PATH="\$PATH"


# Default settings for this client
MACHINEDOTS="\`uname -n\`"                         # This system's hostname
SERVEROSTYPE="\`uname -s | tr '[A-Z/]' '[a-z_]'\`" # This system's operating system in lowercase
CONFIGCLASS=""                                   # This system's config class


# Source config file
if [ -f /etc/default/xymon-client ] ; then
        . /etc/default/xymon-client
else
        echo "Installation failure - missing /etc/default/xymon-client"
        exit 1
fi

# Check for overrides from /etc/sysconfig/xymon-config
[ -n "\$CLIENTHOSTNAME" ]        && MACHINEDOTS="\$CLIENTHOSTNAME"
[ -n "\$CLIENTOS" ]              && SERVEROSTYPE="\$CLIENTOS"
[ -n "\$CLIENTCLASS" ]           && CONFIGCLASS="\$CLIENTCLASS"


MACHINE="\`echo \$MACHINEDOTS | sed -e 's/\./,/g'\`"
XYMONOSSCRIPT="xymonclient-\${SERVEROSTYPE}.sh"
XYMONLAUNCHOPTS="\$XYMONLAUNCHOPTS"

export MACHINE MACHINEDOTS SERVEROSTYPE XYMONOSSCRIPT XYMONLAUNCHOPTS XYMONCLIENTHOME CONFIGCLASS

# Values used in the remainder of the initscript
envfile="/opt/xymon/client/etc/xymonclient.cfg"
configfile="/opt/xymon/client/etc/clientlaunch.cfg"
logfile="/var/log/xymon/xymonlaunch.log"
pidfile="/var/run/xymon/xymonlaunch.pid"


# Check to make sure our pidfile's directory exists
rundir="\`dirname \$pidfile\`"
[ -d "\$rundir" ] || install -d -o "\$user" -g "\$user" "\$rundir" || exit 1


checkruntime() {

    # Prevent duplicate
    test x"\$XYMSRV" = x"\$XYMONSERVERS" && XYMONSERVERS=""

    # Keep xymonconfig from complaining about non-existant env var
    test -z "\$XYMSRV" && XYMSRV=""

    # Do we have servers to report back to?
    if [ x"\$NAME" = x"xymon-client" -a -z "\$XYMSRV" -a -z "\$XYMONSERVERS" ]; then
        echo "Please configure XYMONSERVERS in /etc/default/xymon-client"
        failure "Please configure XYMONSERVERS in /etc/default/xymon-client"
        exit 1
    fi

    # Empty XYMONSERVERS/XYMSRV is okay for xymond since we
    # add at least the local \$XYMONSERVERIP in xymonserver.cfg

}

# 
# See how we were called.
# 
RETVAL=0

start() {
        # Check if it is already running
        if [ ! -e /var/lock/subsys/\$prog ]; then

            echo -n $"Starting \$NAME: "
            ulimit -S -c 0 >/dev/null 2>&1      # No core dumps
            checkruntime

            SUCMD=su
            [ -x /sbin/runuser ] && SUCMD=/sbin/runuser

            \$SUCMD -m -s /bin/dash "\$user" -c \\
                "\$DAEMON \$XYMONLAUNCHOPTS --env=\$envfile --config=\$configfile --log=\$logfile --pidfile=\$pidfile" >/dev/null 2>&1
            RETVAL=\$?
            [ "\$RETVAL" -eq 0 ] && success && touch /var/lock/subsys/\$prog || failure
            echo
        fi
        return \$RETVAL
}

stop() {
        echo -n $"Stopping \$NAME: "
        killall -g xymonlaunch
        RETVAL=\$?
        [ "\$RETVAL" -eq 0 ] && success $"\$NAME shutdown" || failure $"\$NAME shutdown"

        if [ -z "\$RESTARTING" ] ; then
           # Kill any vmstat or other xymon processes lying around
           ps -u "\$user" -o pid= | xargs -r kill -TERM
        fi

        rm -f "\$pidfile" "/var/lock/subsys/\$prog"
        echo
        return \$RETVAL
}

restart() {
        RESTARTING=1
        stop
        start
        return \$RETVAL
}

reload() {
        echo -n $"Reloading \$NAME configuration files: "
        cat \$rundir/*.pid 2>/dev/null | xargs -r kill -HUP
        RETVAL=$\?
        [ "\$RETVAL" -eq 0 ] && success $"\$NAME reloaded" || failure $"\$NAME reloaded"
        echo
        return \$RETVAL
}

rotate() {
        echo -n $"Rotating logs for \$NAME: "
        killproc -p "\$pidfile" xymonlaunch -HUP
        RETVAL=\$?
        [ "\$RETVAL" -eq 0 ] && success || failure
        echo
        return \$RETVAL
}

status_q() {
        status xymonlaunch >/dev/null 2>&1
}

case "\$1" in
  start)
        status_q && exit 0
        start
        ;;
  stop)
        status_q || exit 0
        stop
        ;;
  restart)
        restart
        ;;
  reload)
        status_q || exit 7
        reload
        ;;
  force-reload)
        restart
        ;;
  status)
        status xymonlaunch
        RETVAL=\$?
        ;;
  condrestart|try-restart)
        status_q || exit 0
        restart
        ;;
  rotate)
        status_q || exit 0
        rotate
        ;;
  *)
        echo "Usage: \$NAME {start|stop|status|restart|condrestart|try-restart|reload|force-reload|rotate}"
        exit 1
        ;;
esac

exit \$RETVAL
EOF

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

