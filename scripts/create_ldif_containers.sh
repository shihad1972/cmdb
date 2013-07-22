#!/bin/bash

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