#!/bin/sh
#
# Updated a kickstart installation to use ldap and / or kerberos authentication
# methods
#
# (C) 2015 Iain M Conochie
#
while getopts "klb:d:e:r:s:" opt; do

    case $opt in

        k  ) kerberos=YES
        ;;

        l  ) ldap=YES
        ;;

        s  ) ssl=$OPTARG
        ;;

        b  ) basedn=$OPTARG
        ;;

        d  ) kdc=$OPTARG
        ;;

        r  ) realm=$OPTARG
        ;;

        e  ) ldapserver=$OPTARG
        ;;

        \? ) echo "Usage: $0 [ -k | -l ] -b ldap-base-dn -d kdc-list -r kerberos-realm -e ldapserver -s ssl-cert"
        exit 1
    esac

done

if [ -n "$kerberos" ]; then
    if [ -z "$kdc" ]; then
        echo "No KDC argument passed via -d option. Exiting."
        exit 1
    fi
    if [ -z "$realm" ]; then
        echo "NO realm arguemnt passed via -r option. Exiting."
        exit 1
    fi
fi

if [ -n "$ldap" ]; then
    if [ -z "$basedn" ]; then
        echo "No base DN specified via -b. Exiting."
        exit 1
    fi
    if [ -z "$ldapserver" ]; then
        echo "No ldapserver specified via -e. Exiting."
        exit 1
    fi
fi

AUTHS="--enableshadow --passalgo=sha512 --update"
if [ -n "$kerberos" ]; then
    AUTHS="$AUTHS --enablekrb5 --krb5realm=$realm"
    KDC=1
    for i in $(echo $kdc | tr "," "\n"); do
        if [ "$KDC" -eq 1 ]; then
            AUTHS="$AUTHS --krb5adminserver=$i"
        fi
        KDC=`expr $KDC + 1`
    done
    AUTHS="$AUTHS --krb5kdc=$kdc "
fi

if [ -n "$ldap" ]; then
    AUTHS="$AUTHS --enableldap --enableldapauth --ldapbasedn=$basedn"
    if [ -n "$ssl" ]; then
        wget $ssl
        cert=`basename $ssl`
	if [ ! -d /etc/openldap/cacerts ]; then
            mkdir -p /etc/openldap/cacerts
        fi
        mv $cert /etc/openldap/cacerts/
        /usr/bin/c_rehash /etc/openldap/cacerts
        AUTHS="$AUTHS --enableldaptls"
    fi
    AUTHS="$AUTHS --ldapserver=$ldapserver --enablemkhomedir"
fi

echo "$AUTHS"

authconfig $AUTHS
