#!/bin/sh
#
#
# (C) 2011 Iain Conochie
#
#
# Kickstart Postfix Config script
#
# During kickstart installation, configure
# postfix SMTP server with some default settings,
# including an alias for root
#
#
# Will require the hostname and domainname to be passed
# as arguments to the script
#
# Copyright (C) 2013 Iain M Conochie <iain-AT-thargoid-DOT-co-DOT-uk>
#
# This copyrighted material is made available to anyone who wishes to use,
# modify, copy or redistribute is subject to the terms and conditions of the
# GNU General Public License V2
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation Inc.,
# 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#
#########################################################

CONFIG=/etc/postfix/main.cf
while getopts "h:d:i:r:" opt; do

    case $opt in

        h  ) hostname=$OPTARG
        ;;

        d  ) domainname=$OPTARG
        ;;

        i  ) ipaddr=$OPTARG
        ;;

	r  ) relay=$OPTARG
	;;

        \? ) echo "Usage: $0 -h hostname -d domainname -i ip-addr -r relay-host"
        exit 1
    esac

done

if [ -z $hostname ]; then
    echo "Hostname not set. Aborting!"
    exit 1
elif [ -z $domainname ]; then 
    echo "Domain not set. Aborting!"
    exit 1
elif [ -z $ipaddr ]; then
    echo "IP Address not set. Aborting!"
    exit 1
elif [ -z $relay ]; then
    echo "Relayhost not set. Aborting!"
    exit 1
fi

echo "Hostname: $hostname"
echo "Domain: $domainname"
echo "IP: $ipaddr"
echo "Relayhost: $relay"

if [ ! -f $CONFIG ]; then
    echo "There seems to be no postfix configuration file!"
    echo "Are you sure postfix is installed?"
    exit 1
fi

mv $CONFIG ${CONFIG}.OLD

cat > $CONFIG <<EOF
myhostname = ${hostname}.${domainname}
mydomain = $domainname
myorigin = $hostname.$domainname
queue_directory = /var/spool/postfix
command_directory = /usr/sbin
daemon_directory = /usr/libexec/postfix
mail_owner = postfix
inet_interfaces = localhost, $ipaddr
mydestination = \$myhostname, localhost.\$mydomain, localhost, \$myorigin
unknown_local_recipient_reject_code = 550
alias_maps = hash:/etc/aliases
alias_database = hash:/etc/aliases
relayhost = ${relay} 
  
debug_peer_level = 2
debugger_command =
         PATH=/bin:/usr/bin:/usr/local/bin:/usr/X11R6/bin
         xxgdb \$daemon_directory/\$process_name \$process_id & sleep 5
sendmail_path = /usr/sbin/sendmail.postfix
newaliases_path = /usr/bin/newaliases.postfix
mailq_path = /usr/bin/mailq.postfix
setgid_group = postdrop
html_directory = no
manpage_directory = /usr/share/man

EOF

cat >> /etc/aliases <<EOF
root:	sysadmin@${domainname}

EOF

/sbin/chkconfig --add postfix
newaliases

