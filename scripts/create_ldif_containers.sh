#!/bin/bash
#
#  Script to create an LDIF file for the base OU's needed for and LDAP
#  authentication directory.
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
while getopts "d:s" opt; do
    case $opt in
        d ) DOMAIN=$OPTARG
            ;;
        s ) SUDO=yes
            ;;
        \?) echo "Usage: $0 -d domain -s (have sudo in ldap)"
            exit 1
            ;;
    esac
done


if [ -z $DOMAIN ]; then
    echo "No domain specified"
    echo "Usage: $0 -d domain -s (have sudo in ldap)"
    exit 2
fi

declare -i len
declare -i i
declare -i j
declare -a arr
IFS=. read -a arr <<< "$DOMAIN"
len=${#arr[@]}
directory=""

for ((i=0; i<${len}; i++));
do
    let j=${i}+1
    directory="${directory}dc=${arr[${i}]},"
done

cat >/tmp/${DOMAIN}-ou.ldif <<EOF
# $DOMAIN
dn: ${directory%?}
dc: ${arr[0]}
objectClass: dcObject
objectClass: top
objectClass: organizationalUnit
ou: $DOMAIN

# people, $DOMAIN
dn: ou=people,${directory%?}
objectClass: top
objectClass: organizationalUnit
ou: people

# group, $DOMAIN
dn: ou=group,${directory%?}
objectClass: top
objectClass: organizationalUnit
ou: group

EOF

if echo $SUDO | grep yes; then
    cat >>/tmp/${DOMAIN}-ou.ldif <<EOF
# SUDOers, $DOMAIN
dn: ou=SUDOers,${directory%?}
objectClass: top
objectClass: organizationalUnit
ou: SUDOers

EOF
fi