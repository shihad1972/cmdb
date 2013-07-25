#!/bin/bash
#
#  Script to create an LDIF file to add a group into and LDAP authentication
#  directory.
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
while getopts "d:g:n:u:" opt; do
    case $opt in 
        d  ) DOMAIN=$OPTARG
             ;;
        g  ) GROUPID=$OPTARG
             ;;
        n  ) NAME=$OPTARG
             ;;
        u  ) USERS=$OPTARG
             ;;
        \? ) echo "Usage: $0 -d domain -g gid -n group-name (-u user,user,user....user)"
             exit 1
             ;;
    esac
done

if [ -z $DOMAIN ]; then
    echo "No domain supplied on command line!"
    exit 2
fi

if [ -z $GROUPID ]; then
    echo "No gid supplied on command line!"
    exit 3
fi

if [ -z $NAME ]; then
    echo "No group name supplied on command line!"
    exit 4
fi

declare -i len
declare -i i
declare -a arr
IFS=. read -a arr <<< "$DOMAIN"
len=${#arr[@]}
directory=""

for ((i=0; i<${len}; i++));
do
    directory="${directory}dc=${arr[${i}]},"
done

cat > /tmp/${NAME}-group-add.ldif <<EOF
# ${NAME}, group, ${DOMAIN}
dn: cn=${NAME},${directory%?}
cn: ${NAME}
gidNumber: ${GROUPID}
objectClass: posixGroup
objectClass: top
EOF

if ! [ -z ${USERS} ]; then
    IFS=, read -a arr <<< "$USERS"
    len=${#arr[@]}
    for ((i=0; i<${len}; i++))
    do
        cat >> /tmp/${NAME}-group-add.ldif <<EOF
memberUid: ${arr[${i}]}
EOF
    done
fi
