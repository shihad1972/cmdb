#!/bin/bash

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
    exit 2
fi

if [ -z $PASSWORD ]; then
    echo "No password specified"
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

