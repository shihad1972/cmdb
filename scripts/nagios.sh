#!/bin/sh
#
# Script to install nagios NRPE client on debian / ubuntu boxes.
#
# (C) Iain Conochie 2013
#
###############################################################################

APT=`which apt-get`
NAGCFG=/etc/nagios
HOST=$1
SERVICE=`which service`

if [ -z $APT ]; then
  echo "No apt-get!"
  exit 1
fi

if [ -z $HOST ]; then
  echo "Usage: $0 hostname/ip-address "
  exit 1
fi

echo "Updating APT"
$APT update > /dev/null 2>&1

echo "Installing nrpe (and a large number of dependencies)"
echo "Please be patient"
$APT install -y nagios-nrpe-server > /dev/null 2>&1 || ( echo "Cannot install nrpe" && exit 1 )

mv ${NAGCFG}/nrpe.cfg ${NAGCFG}/nrpe.old
cat > ${NAGCFG}/nrpe.cfg <<EOF
log_facility=daemon
pid_file=/var/run/nagios/nrpe.pid
server_port=5666
nrpe_user=nagios
nrpe_group=nagios
allowed_hosts=$HOST
 
dont_blame_nrpe=0
debug=0
command_timeout=60
connection_timeout=300
command[check_users]=/usr/lib/nagios/plugins/check_users -w 5 -c 10
command[check_load]=/usr/lib/nagios/plugins/check_load -w 15,10,5 -c 30,25,20
command[check_zombie_procs]=/usr/lib/nagios/plugins/check_procs -w 5 -c 10 -s Z
command[check_total_procs]=/usr/lib/nagios/plugins/check_procs -w 150 -c 200 
command[patches]=/usr/lib/nagios/plugins/check_apt
command[check_root]=/usr/lib/nagios/plugins/check_disk -w 10% -c 5% -p /
command[check_var]=/usr/lib/nagios/plugins/check_disk -w 10% -c 5% -p /var
command[check_tmp]=/usr/lib/nagios/plugins/check_disk -w 10% -c 5% -p /tmp
command[check_usr]=/usr/lib/nagios/plugins/check_disk -w 10% -c 5% -p /usr
command[check_swap]=/usr/lib/nagios/plugins/check_swap -w 10 -c 5 
command[check_mailq]=/usr/lib/nagios/plugins/check_mailq -w 200 -c 500 -M postfix 
command[check_all_disks]=/usr/lib/nagios/plugins/check_disk -w 6% -c 2% -X nfs4 -X tmpfs -X rootfs -X udev -X nfs -X none -A
include=/etc/nagios/nrpe_local.cfg
include_dir=/etc/nagios/nrpe.d/

EOF

$SERVICE nagios-nrpe-server restart
