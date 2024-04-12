packdir = $(localstatedir)/lib/cmdb
scriptdir = $(packdir)/scripts
script_DATA = disable_install.php kick-ntp.sh kick-postfix.sh log.sh motd.sh\
              ldap-auth.sh firstboot.sh xymon-client.sh kick-final.sh\
              apt-nagios.sh kick-nagios.sh hobbit-patch.sh deploy-patch.sh\
              kick-auth.sh xymon-setup.sh kick-ldap-sudo.sh\
              bb-mysql.pl deploy-mysql.sh lspci.sh sssd-auth.sh
bin_SCRIPTS = cmdb.sh

install-exec-hook:      mkdir $(packdir)/{hosts,web,tmp}
