#!/bin/sh
#
# Script to allow sudo to use an ldap backend database
#
BASE=$1
LDAP_SERVER=$2
SSL=$3

FILE=/etc/sudo-ldap.conf

cp $FILE /root/sudo-ldap.conf
cat > $FILE<<EOF
uri $LDAP_SERVER
sudoers_base $BASE
EOF

if [ -n "$SSL" ]; then
  cat >> $FILE<<EOF
ssl start_tls
tls_checkpeer yes
tls_cacertfile $SSL
EOF
fi

cat >>/etc/nsswitch.conf<<EOF
sudoers:    ldap files

EOF
