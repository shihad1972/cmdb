# 10/12/2012

ALTER TABLE build_type ADD COLUMN url varchar(80);
ALTER TABLE boot_line ADD COLUMN bt_id int(7) AFTER os_ver;
UPDATE boot_line bl, build_type bt SET bl.bt_id = bt.bt_id WHERE bt.alias = bl.os;
ALTER TABLE build_type ADD COLUMN arg varchar(16) AFTER build_type;
UPDATE build_type SET arg = 'ks' WHERE build_type = 'kickstart';
UPDATE build_type SET arg = 'url' WHERE build_type = 'preseed';
UPDATE build_type SET url = 'http://debian.shihad.org/preseed/' WHERE alias = 'debian';
UPDATE build_type SET url = 'http://ubuntu.shihad.org/preseed/' WHERE alias = 'ubuntu';
UPDATE build_type SET url = 'http://kickstart.shihad.org/ks/' WHERE alias = 'centos';
UPDATE build_type SET url = 'http://slackware.shihad.org/' WHERE alias = 'slack';
#
# 11/12/2012
INSERT INTO configuration (config, value) VALUES ("cbctmpdir", "/tmp/cbs/");
INSERT INTO configuration (config, value) VALUES ("cbctftpdir", "/var/lib/tftpboot/");
INSERT INTO configuration (config, value) VALUES ("cbcpxe", "pxelinux.cfg/");
INSERT INTO configuration (config, value) VALUES ("cbctlos", "/usr/local/build/");
INSERT INTO configuration (config, value) VALUES ("cbcdhcp", "/etc/dhcp/dhcpd.hosts");
INSERT INTO configuration (config, value) VALUES ("cbcpreseed", "preseed/");
INSERT INTO configuration (config, value) VALUES ("cbckickstart", "ks/");
UPDATE boot_line SET boot_line = "locale=en_GB.UTF-8 keymap=uk auto=true priority=critical vga=788" WHERE os = 'debian' and os_ver = '6';
UPDATE boot_line SET boot_line = "auto=true priority=critical vga=788" WHERE os = 'debian' and os_ver = '5';
UPDATE boot_line SET boot_line = "ksdevice=eth0 console=tty0 ramdisk_size=8192" WHERE os = 'centos';
UPDATE boot_line SET boot_line = "ksdevice=eth0 console=tty0 ramdisk_size=8192" WHERE os = 'redhat';
UPDATE boot_line SET boot_line = "country=GB console-setup/layoutcode=gb auto=true priority=critical vga=788" WHERE os = 'ubuntu';
UPDATE build_os bo, boot_line bl SET bo.boot_id = bl.boot_id WHERE bo.alias = 'redhat' and bo.os_version = '6.0' AND bl.os = 'redhat';
UPDATE build_os SET ver_alias = 'none' WHERE alias = 'centos' OR alias='redhat' OR alias= 'slack';
ALTER TABLE build_os MODIFY ver_alias varchar(25) NOT NULL DEFAULT 'none';
ALTER TABLE boot_line MODIFY COLUMN boot_line varchar(150) NOT NULL DEFAULT 'none';
UPDATE boot_line SET boot_line = 'none' WHERE os = 'slack';
ALTER TABLE build_type MODIFY COLUMN arg varchar(16) NOT NULL DEFAULT 'none';
ALTER TABLE build_type MODIFY COLUMN url varchar(80) NOT NULL DEFAULT 'none';
UPDATE build_type SET build_type = 'none', arg = 'none' WHERE alias = 'slack';
ALTER TABLE build_type MODIFY COLUMN build_type varchar(25) NOT NULL DEFAULT 'none';
#
# 13/12/2012
ALTER TABLE build_domain MODIFY start_ip int(4) unsigned NOT NULL DEFAULT 0;
ALTER TABLE build_domain MODIFY end_ip int(4) unsigned NOT NULL DEFAULT 0;
ALTER TABLE build_domain MODIFY netmask int(4) unsigned NOT NULL DEFAULT 0;
ALTER TABLE build_domain MODIFY gateway int(4) unsigned NOT NULL DEFAULT 0;
ALTER TABLE build_domain MODIFY ns int(4) unsigned NOT NULL DEFAULT 0;
ALTER TABLE build_domain MODIFY domain varchar(150) NOT NULL DEFAULT 'no.domain';
INSERT INTO packages (package, varient_id, os_id) SELECT 'openssh-server', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'build-essential', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'sudo', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'locate', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'less', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'nfs-common', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'hobbit-client', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'ldap-utils', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'whois', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'dns-utils', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'postfix', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'tcpdump', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'ntp', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'ntpdate', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'libnss-ldap', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'logwatch', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT package, 2, os_id FROM packages p, varient v WHERE valias = 'web' AND p.varient_id = v.varient_id;
INSERT INTO packages (package, varient_id, os_id) SELECT package, 3, os_id FROM packages p, varient v WHERE valias = 'web' AND p.varient_id = v.varient_id;
INSERT INTO packages (package, varient_id, os_id) SELECT package, 4, os_id FROM packages p, varient v WHERE valias = 'web' AND p.varient_id = v.varient_id;
INSERT INTO packages (package, varient_id, os_id) SELECT package, 5, os_id FROM packages p, varient v WHERE valias = 'web' AND p.varient_id = v.varient_id;
INSERT INTO packages (package, varient_id, os_id) SELECT package, 6, os_id FROM packages p, varient v WHERE valias = 'web' AND p.varient_id = v.varient_id;
INSERT INTO packages (package, varient_id, os_id) SELECT package, 7, os_id FROM packages p, varient v WHERE valias = 'web' AND p.varient_id = v.varient_id;
INSERT INTO packages (package, varient_id, os_id) SELECT package, 9, os_id FROM packages p, varient v WHERE valias = 'web' AND p.varient_id = v.varient_id;
INSERT INTO packages (package, varient_id, os_id) SELECT package, 10, os_id FROM packages p, varient v WHERE valias = 'web' AND p.varient_id = v.varient_id;
INSERT INTO packages (package, varient_id, os_id) SELECT package, 11, os_id FROM packages p, varient v WHERE valias = 'web' AND p.varient_id = v.varient_id;
INSERT INTO packages (package, varient_id, os_id) SELECT 'mysql-client', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php5-curl', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php5-gmp', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php5-ldap', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php5-mysql', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php5-xcache', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php-log', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php-http', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php-html-common', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php-auth-http', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php-auth', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php-pear', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php-db', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT package, v.varient_id, bo.os_id FROM packages p, varient v, build_os bo WHERE v.valias = 'lamp' AND bo.os_id = p.os_id AND p.package LIKE 'mysql-cli%';
INSERT INTO packages (package, varient_id, os_id) SELECT package, v.varient_id, bo.os_id FROM packages p, varient v, build_os bo WHERE v.valias = 'lamp' AND bo.os_id = p.os_id AND p.package LIKE 'php5-%';
INSERT INTO packages (package, varient_id, os_id) SELECT package, v.varient_id, bo.os_id FROM packages p, varient v, build_os bo WHERE v.valias = 'lamp' AND bo.os_id = p.os_id AND p.package LIKE 'php-%';
INSERT INTO packages (package, varient_id, os_id) SELECT 'bind9', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'dns' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'bind9-host', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'dns' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'bind9utils', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'dns' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'dovecot-imapd', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'email' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'dovecot-pop3d', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'email' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'mysqmail-dovecot-logger', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'email' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'mysql-server', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'lamp' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'mysql-server', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'mysql' AND bo.alias = 'debian';
INSERT INTO packages (package, varient_id, os_id) SELECT 'slapd', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'ldap' AND bo.alias = 'debian';
ALTER TABLE build_domain ADD COLUMN ntp_server varchar(64) NOT NULL DEFAULT 'shihad.org';
ALTER TABLE build_domain ADD COLUMN config_ntp tinyint(4) NOT NULL DEFAULT 1;
ALTER TABLE build_domain ADD COLUMN ldap_server varchar(64) NOT NULL DEFAULT 'ldap01.shihad.org';
ALTER TABLE build_domain ADD COLUMN ldap_ssl tinyint(4) NOT NULL DEFAULT 1;
ALTER TABLE build_domain ADD COLUMN ldap_dn varchar(96) NOT NULL DEFAULT 'dc=shihad,dc=org';
ALTER TABLE build_domain ADD COLUMN config_ldap tinyint(4) NOT NULL DEFAULT 1;
ALTER TABLE build_domain ADD COLUMN ldap_bind varchar(128) NOT NULL DEFAULT 'cn=thargoid,dc=shihad,dc=org';
ALTER TABLE build_domain ADD COLUMN log_server varchar(64) NOT NULL DEFAULT 'logger01.shihad.org';
ALTER TABLE build_domain ADD COLUMN config_log tinyint (4) NOT NULL DEFAULT 1;
ALTER TABLE build_domain ADD COLUMN email_server varchar(64) NOT NULL DEFAULT 'mail01.scots.shihad.org';
ALTER TABLE build_domain ADD COLUMN config_email tinyint (4) NOT NULL DEFAULT 1;
INSERT INTO locale (os_id, bt_id) SELECT os_id, bt_id FROM build_os WHERE alias = 'debian';
ALTER TABLE build_type ADD COLUMN mirror varchar(256) NOT NULL DEFAULT 'none';
UPDATE  build_type SET mirror = 'ftp.uk.debian.org' WHERE alias = 'debian';
ALTER TABLE build_domain ADD COLUMN xymon_server varchar(64) NOT NULL DEFAULT 'hobbit.shihad.org';
ALTER TABLE build_domain ADD COLUMN xymon_config tinyint NOT NULL DEFAULT 1;
ALTER TABLE build_domain CHANGE COLUMN email_server smtp_server varchar(64) NOT NULL DEFAULT 'mail01.scots.shihad.org';
ALTER TABLE build_domain CHANGE COLUMN ldap_server ldap_url varchar(96) NOT NULL DEFAULT 'ldaps://ldap01.shihad.org';
UPDATE build_domain SET ntp_server = 'shihad.org';
