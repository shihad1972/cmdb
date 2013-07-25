#!/bin/bash
#
#
#  script to create an LDIF file to add a user to an authentication LDAP
#  directory. Username options are wither firstname or initialsurname
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
#
#
while getopts "d:gln:p:s:u:" opt; do
    case $opt in
        d ) DOMAIN=$OPTARG
            ;;
        g ) GROUP=yes
            ;;
        l ) LONGUSER=yes
            ;;
        n ) NAME=$OPTARG
            ;;
        p ) PASSWORD=$OPTARG
            ;;
        s ) SURNAME=$OPTARG
            ;;
        u ) USERID=$OPTARG
            ;;
        \?) echo "Usage: $0 -d domain -g (create group) -l (use surname in username) -n name -p password -u uid-number -s surname"
            exit 1
            ;;
    esac
done

if [ -z $DOMAIN ]; then
    echo "No domain specified"
    echo "Usage: $0 -d domain -g (create group) -l (use surname in username) -n name -p password -u uid-number -s surname"
    exit 2
fi

if [ -z $NAME ]; then
    echo "Does your user not have a first name?"
    echo "Usage: $0 -d domain -g (create group) -l (use surname in username) -n first-name -p password -u uid-number -s surname"
    exit 3
fi

if [ -z $PASSWORD ]; then
    echo "No password specified"
    echo "Usage: $0 -d domain -g (create group) -l (use surname in username) -n name -p password -u uid-number -s surname"
    exit 4
fi

if [ -z $SURNAME ]; then
    echo "Does this user not have a surname?"
    echo "Usage: $0 -d domain -g (create group) -l (use surname in username) -n name -p password -u uid-number -s surname"
    exit 5
fi

if [ -z $USERID ]; then
    echo "You MUST specify a uid number on the command line"
    echo "Usage: $0 -d domain -g (create group) -l (use surname in username) -n name -p password -u uid-number -s surname"
    exit 6
fi

if echo $LONGUSER | grep yes >/dev/null 2>&1; then
    user=`echo $NAME | cut -b 1`
    USER=`echo ${user}${SURNAME} | tr '[:upper:]' '[:lower:]'`
else
    USER=`echo $NAME | tr '[:upper:]' '[:lower:]'`
fi

declare -i len
declare -i i
declare -a arr
PASSWD=`slappasswd -s $PASSWORD`
IFS=. read -a arr <<< "$DOMAIN"
len=${#arr[@]}
directory=""

for ((i=0; i<${len}; i++));
do
    directory="${directory}dc=${arr[${i}]},"
done

cat >/tmp/${USER}-add.ldif <<EOF
# ${USER}, people, ${DOMAIN}
dn: uid=${USER},${directory%?}
uid: $USER
cn: $NAME $SURNAME
sn: $SURNAME
objectClass: inetOrgPerson
objectClass: posixAccount
objectClass: top
objectClass: shadowAccount
shadowLastChange: 0
shadowMax: 99999
shadowWarning: 7
loginShell: /bin/bash
uidNumber: ${USERID}
homeDirectory: /home/${USER}
gecos: $NAME $SURNAME
mail: ${USER}@${DOMAIN}
EOF

if echo $GROUP | grep yes >/dev/null 2>&1; then
    cat >>/tmp/${USER}-add.ldif <<EOF
gidNumber: $USERID

# ${USER}, group, ${DOMAIN}
dn: cn=${USER},${directory%?}
cn: $USER
gidNumber: ${USERID}
objectClass: posixGroup
objectClass: top

EOF
else
    cat >>/tmp/${USER}-add.ldif <<EOF
gidNumber: 100
EOF
fi