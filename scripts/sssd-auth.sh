#!/bin/sh
#
# Script to build an ssd configuration file
#
# Parameters:
#		-r REALM
#		  Use kerberos REALM
#		-b base-dn
#		  Base ldap search dn
#		-d domain
#		  Domain name to use for DNS discovery
#		-k
#		  Do not setup kerberos authentication
#
# SRV records are required for ldap and kerberos related services in the domain
#

WGET=/usr/bin/wget
while getopts "b:d:kr:u:" opt; do
  case $opt in 
    b)	BASE_DN=$OPTARG
	;;
    d)	DOMAIN=$OPTARG
	;;
    k)	KERBEROS=no	# Currently a place holder. Does nothing.
	;;		# Kerberos is always configured
    r)	REALM=$OPTARG
	;;
    u)	URL=$OPTARG
	;;
    \?) echo "Usage: $0 -b <base-dn> -d <domain name> [ -r <KRB5 REALM> ] [ -k ] [ -u <url-for-ssl-cert> ]"
	exit 1
  esac
done

# Sanity checks
if [ -z "$DOMAIN" ]; then
  echo "Must provide DNS domain"
  exit 1
fi

if [ -z "$BASE_DN" ]; then
  echo "Must provide a base DN"
  exit 1
fi

# This does not work as tr in busybox does not recognise these options.
#if [ -z "$REALM" ]; then
#  REALM=`echo $DOMAIN | tr '[:lower:]' '[:upper:]'`
#  echo "Used domain $DOMAIN to create kerberos realm $REALM"
#fi

# Check to see if we are running in a preseed install
if [ -d /target ]; then
  TARGET=/target/etc/sssd/sssd.conf
  TGT=/target
else
  TARGET=/etc/sssd/sssd.conf
  unset TGT
fi

echo "Creating $TARGET file"
cat >$TARGET<<EOF
[sssd]
config_file_version = 2
services = nss, pam, ssh, sudo
domains = $DOMAIN

[nss]

[pam]

[domain/$DOMAIN]
id_provider = ldap
auth_provider = krb5
ldap_schema = rfc2307
ldap_search_base = $BASE_DN
dns_discovery_domain = $DOMAIN
EOF

if [ -z "$REALM" ]; then
  cat >> ${TGT}/usr/share/firstboot/001-sssd.sh <<EOF
#!/bin/sh
DOMAIN=$DOMAIN
REALM=\`echo \$DOMAIN | tr '[:lower:]' '[:upper:]'\`
echo "krb5_realm = \$REALM" >> /etc/sssd/sssd.conf
EOF
  chmod 755 ${TGT}/usr/share/firstboot/001-sssd.sh
else
  echo "rkb5_realm = $REALM" >> $TARGET
fi

echo "Updating permissions on $TARGET"
chmod 600 $TARGET
if [ -n "$URL" ] && [ -n "$WGET" ]; then
  echo "Command: $WGET -O ${TGT}/etc/ssl/certs/Root-CA.pem $URL"
  $WGET -O ${TGT}/etc/ssl/certs/Root-CA.pem $URL
  if [ -n $TGT ]; then
    cat >> ${TGT}/usr/share/firstboot/001-rehash.sh <<EOF
#!/bin/sh

/usr/bin/c_rehash /etc/ssl/certs/
EOF
    chmod 755 ${TGT}/usr/share/firstboot/001-rehash.sh
  else
    /usr/bin/c_rehash /etc/ssl/certs/
  fi
fi

