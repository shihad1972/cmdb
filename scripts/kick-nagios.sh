#!/bin/sh
#
#
# Script to setup nagios on centos server
#
# Very basic setup
#
#
# Usage: kick-nagios.sh <nagios-server> <download-url>

RPM=/bin/rpm
YUM=/usr/bin/yum

HOST=$1
URL=$2

NRPE=2.15
NAGPLUG=2.0
#
# Grab headers needed for compilation

$YUM install -y zlib-devel krb5-devel openssl-devel make gcc

cd /tmp

for i in nagios-plugins-${NAGPLUG}.tar.gz nrpe-${NRPE}.tar.gz

  do wget ${URL}/${i}

done

getent group nagios > /dev/null
RETVAL=$?
[ $RETVAL -eq 0 ] || groupadd -g 5666 nagios
getent passwd nagios > /dev/null
RETVAL=$?
[ $RETVAL -eq 0 ] || useradd -d /usr/local/nagios -g 5666 -u 5666 nagios

cd /usr/local/src
tar xvzf /tmp/nrpe-${NRPE}.tar.gz
cd nrpe-${NRPE}
./configure --prefix=/opt/nagios
make
make install
cd ../
tar xvzf /tmp/nagios-plugins-${NAGPLUG}.tar.gz
cd nagios-plugins-${NAGPLUG}
./configure --prefix=/opt/nagios
make
make install

cat >/opt/nagios/nrpe.cfg <<EOF
log_facility=daemon
pid_file=/var/run/nrpe.pid
server_port=5666
nrpe_user=nagios
nrpe_group=nagios
allowed_hosts=$HOST
dont_blame_nrpe=0
debug=0
command_timeout=60
connection_timeout=300

command[check_users]=/opt/nagios/libexec/check_users -w 5 -c 10
command[check_load]=/opt/nagios/libexec/check_load -w 15,10,5 -c 30,25,20
command[check_disk_root]=/opt/nagios/libexec/check_disk -w 20% -c 10% -p /
command[check_zombie_procs]=/opt/nagios/libexec/check_procs -w 5 -c 10 -s Z
command[check_total_procs]=/opt/nagios/libexec/check_procs -w 150 -c 200
command[check_all_disks]=/opt/nagios/libexec/check_disk -w 6% -c 2% -X nfs4 -X tmpfs -X rootfs -X udev -X nfs -X none -A
command[check_swap]=/opt/nagios/libexec/check_swap -w 10 -c 5
command[check_mailq]=/opt/nagios/libexec/check_mailq -w 200 -c 500 -M postfix
include=/etc/nagios/nrpe_local.cfg
EOF

cat >/etc/init.d/nrpe <<EOF
#!/bin/bash
#
# Init file for Nagios NRPE
#
# Written by Dag Wieers <dag@wieers.com>.
#
# chkconfig: - 80 20
# description: Nagios NRPE daemon
#
# processname: nrpe
# config: /opt/nagios/nrpe.cfg
# pidfile: /var/run/nrpe

source /etc/rc.d/init.d/functions

### Default variables
CONFIG="/opt/nagios/nrpe.cfg"

[ -x /opt/nagios/bin/nrpe ] || exit 1
[ -r "\$CONFIG" ] || exit 1

RETVAL=0
daemon="/opt/nagios/bin/nrpe"
prog="nrpe"
desc="Nagios NRPE daemon"

start() {
        echo -n $"Starting \$desc (\$prog): "
        \$daemon -c "\$CONFIG" -d
        RETVAL=\$?
        echo
        [ \$RETVAL -eq 0 ] && touch /var/lock/subsys/\$prog
        return \$RETVAL
}

stop() {
        echo -n $"Shutting down \$desc (\$prog): "
        killproc \$prog
        RETVAL=\$?
        echo
        [ \$RETVAL -eq 0 ] && rm -f /var/lock/subsys/\$prog
        return \$RETVAL
}

restart() {
        stop
        start
}

reload() {
        echo -n $"Reloading \$desc (\$prog): "
        killproc \$prog -HUP
        RETVAL=\$?
        echo
        return \$RETVAL
}

case "\$1" in
  start)
        start
        ;;
  stop)
        stop
        ;;
  restart)
        restart
        ;;
  reload)
        reload
        ;;
  condrestart)
        [ -e /var/lock/subsys/\$prog ] && restart
        RETVAL=\$?
        ;;
  status)
        status \$prog
        RETVAL=\$?
        ;;
  *)
        echo $"Usage: \$0 {start|stop|restart|reload|condrestart|status}"
        RETVAL=1
esac

exit \$RETVAL

EOF

chmod 755 /etc/init.d/nrpe

chkconfig --levels 2345 nrpe on

