-- Create build_type entries
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES ("debian", "preseed", "url", "http://${HOSTNAME}.${DOMAIN}/cmdb/", "$MIRROR", "auto=true priority=critical vga=788");
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES ("ubuntu", "preseed", "url", "http://${HOSTNAME}.${DOMAIN}/cmdb/", "$MIRROR", "auto=true priority=critical vga=788");
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES ("centos", "kickstart", "ks", "http://${HOSTNAME}.${DOMAIN}/cmdb/", "$MIRROR", "ksdevice=eth0 console=tty0 ramdisk_size=8192");
--
-- Create build_os entries for the above OS's
INSERT INTO build_os (os, os_version, alias, ver_alias, arch, bt_id) SELECT "Debian", "7", alias, "wheezy", "i386", bt_id FROM build_type WHERE alias = "debian";
INSERT INTO build_os (os, os_version, alias, ver_alias, arch, bt_id) SELECT "Debian", "7", alias, "wheezy", "x86_64", bt_id FROM build_type WHERE alias = "debian";
INSERT INTO build_os (os, os_version, alias, ver_alias, arch, bt_id) SELECT "Centos", "5", alias, "none", "i386", bt_id FROM build_type WHERE alias = "centos";
INSERT INTO build_os (os, os_version, alias, ver_alias, arch, bt_id) SELECT "Centos", "5", alias, "none", "x86_64", bt_id FROM build_type WHERE alias = "centos";
INSERT INTO build_os (os, os_version, alias, ver_alias, arch, bt_id) SELECT "Centos", "6", alias, "none", "i386", bt_id FROM build_type WHERE alias = "centos";
INSERT INTO build_os (os, os_version, alias, ver_alias, arch, bt_id) SELECT "Centos", "6", alias, "none", "x86_64", bt_id FROM build_type WHERE alias = "centos";
INSERT INTO build_os (os, os_version, alias, ver_alias, arch, bt_id) SELECT "Ubuntu", "12.04", alias, "precise", "i386", bt_id FROM build_type WHERE alias = "ubuntu";
INSERT INTO build_os (os, os_version, alias, ver_alias, arch, bt_id) SELECT "Ubuntu", "12.04", alias, "precise", "x86_64", bt_id FROM build_type WHERE alias = "ubuntu";
INSERT INTO build_os (os, os_version, alias, ver_alias, arch, bt_id) SELECT "Ubuntu", "12.10", alias, "quantal", "i386", bt_id FROM build_type WHERE alias = "ubuntu";
INSERT INTO build_os (os, os_version, alias, ver_alias, arch, bt_id) SELECT "Ubuntu", "12.10", alias, "quantal", "x86_64", bt_id FROM build_type WHERE alias = "ubuntu";
INSERT INTO build_os (os, os_version, alias, ver_alias, arch, bt_id) SELECT "Ubuntu", "13.04", alias, "raring", "i386", bt_id FROM build_type WHERE alias = "ubuntu";
INSERT INTO build_os (os, os_version, alias, ver_alias, arch, bt_id) SELECT "Ubuntu", "13.04", alias, "raring", "x86_64", bt_id FROM build_type WHERE alias = "ubuntu";
--
-- Create build varients
INSERT INTO varient (varient, valias) VALUES ("Web Server", "web");
INSERT INTO varient (varient, valias) VALUES ("MySQL Server", "mysql");
INSERT INTO varient (varient, valias) VALUES ("Full Lamp Stack", "lamp");
INSERT INTO varient (varient, valias) VALUES ("LDAP Server", "ldap");
INSERT INTO varient (varient, valias) VALUES ("IMAP Server", "imap");
INSERT INTO varient (varient, valias) VALUES ("SMTP Server", "smtp");
INSERT INTO varient (varient, valias) VALUES ("DNS Server", "dns");
--
-- Create Debian package lists for the above builds
INSERT INTO packages (package, varient_id, os_id) SELECT "logwatch", v.varient_id, o.os_id FROM varient v, build_os o;
INSERT INTO packages (package, varient_id, os_id) SELECT "ntp", v.varient_id, o.os_id FROM varient v, build_os o;
INSERT INTO packages (package, varient_id, os_id) SELECT "openssh-server", v.varient_id, o.os_id FROM varient v, build_os o;
INSERT INTO packages (package, varient_id, os_id) SELECT "postfix", v.varient_id, o.os_id FROM varient v, build_os o;
INSERT INTO packages (package, varient_id, os_id) SELECT "sudo", v.varient_id, o.os_id FROM varient v, build_os o;
INSERT INTO packages (package, varient_id, os_id) SELECT "sysstat", v.varient_id, o.os_id FROM varient v, build_os o;
INSERT INTO packages (package, varient_id, os_id) SELECT "ntp", v.varient_id, o.os_id FROM varient v, build_os o;
INSERT INTO packages (package, varient_id, os_id) SELECT "ntpdate", v.varient_id, o.os_id FROM varient v, build_os o;
INSERT INTO packages (package, varient_id, os_id) SELECT "nfs4-acl-tools", v.varient_id, o.os_id FROM varient v, build_os o;