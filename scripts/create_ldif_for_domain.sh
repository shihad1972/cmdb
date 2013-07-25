#!/bin/bash
#
#  Script to create an ldif file to add a new domain for authentication
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
while getopts "p:d:a:" opt; do
    case $opt in 
        p ) PASSWORD=$OPTARG
            ;;

        d ) DOMAIN=$OPTARG
            ;;

        a ) ADMIN=$OPTARG
            ;;

        \? ) echo "Usage: $0 -p passwd -d domain -a admin-user"
             exit 1
    esac
done

if [ -z $DOMAIN ]; then
    echo "No domain specified"
    echo "Usage: $0 -p passwd -d domain -a admin-user"
    exit 2
fi

if [ -z $PASSWORD ]; then
    echo "No password specified"
    echo "Usage: $0 -p passwd -d domain -a admin-user"
    exit 3
fi

if [ -z $ADMIN ]; then
    echo "Using default admin user admin"
    ADMIN="admin"
fi

declare -i len
declare -i i
declare -i j
declare -a arr
PASSWD=`slappasswd -s $PASSWORD`
IFS=. read -a arr <<< "$DOMAIN"
len=${#arr[@]}
admin="cn=$ADMIN"
directory=""

for ((i=0; i<${len}; i++));
do
    let j=${i}+1
    admin="${admin},dc=${arr[${i}]}"
    directory="${directory}dc=${arr[${i}]},"
done

cat >/tmp/$DOMAIN.ldif <<EOF
dn: olcDatabase=hdb,cn=config
objectClass: olcDatabaseConfig
objectClass: olcHdbConfig
olcDatabase: hdb
olcDbDirectory: /var/lib/slapd/$DOMAIN
olcSuffix: ${directory%?}
olcAccess: to attrs=userPassword,shadowLastChange by self write by anonymous auth by dn="${admin}" write by * none
olcAccess: to dn.base="" by * read
olcAccess: to * by self write by dn="$admin" write by * read
olcRootDN: $admin
olcRootPW: $PASSWD
olcDbCheckpoint: 512 30
olcDbConfig: set_cachesize 0 2097152 0
olcDbConfig: set_lk_max_objects 1500
olcDbConfig: set_lk_max_locks 1500
olcDbConfig: set_lk_max_lockers 1500
olcDbIndex: default pres,eq
olcDbIndex: uid
olcDbIndex: cn,sn pres,eq,sub
olcDbIndex: objectClass eq
olcDbIndex: uniqueMember eq
olcDbIndex: uidNumber,gidNumber pres,eq
EOF

