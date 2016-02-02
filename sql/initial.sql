
--
-- Create build_os entries for the above OS's
INSERT INTO build_os (os, os_version, alias, ver_alias, arch, bt_id) SELECT "Debian", "7", alias, "wheezy", "i386", bt_id FROM build_type WHERE alias = "debian";
INSERT INTO build_os (os, os_version, alias, ver_alias, arch, bt_id) SELECT "Debian", "7", alias, "wheezy", "x86_64", bt_id FROM build_type WHERE alias = "debian";
INSERT INTO build_os (os, os_version, alias, ver_alias, arch, bt_id) SELECT "Debian", "8", alias, "jessie", "i386", bt_id FROM build_type WHERE alias = "debian";
INSERT INTO build_os (os, os_version, alias, ver_alias, arch, bt_id) SELECT "Debian", "8", alias, "jessie", "x86_64", bt_id FROM build_type WHERE alias = "debian";
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
INSERT INTO build_os (os, os_version, alias, ver_alias, arch, bt_id) SELECT "Ubuntu", "14.04", alias, "trusty", "i386", bt_id FROM build_type WHERE alias = "ubuntu";
INSERT INTO build_os (os, os_version, alias, ver_alias, arch, bt_id) SELECT "Ubuntu", "14.04", alias, "trusty", "x86_64", bt_id FROM build_type WHERE alias = "ubuntu";
--
-- Create default locales
INSERT INTO locale (os_id, bt_id) SELECT os_id, bt_id FROM build_os;
--
-- Create build varients
INSERT INTO varient (varient, valias) VALUES ("Base Build", "base");
INSERT INTO varient (varient, valias) VALUES ("Web Server", "web");
INSERT INTO varient (varient, valias) VALUES ("MySQL Server", "mysql");
INSERT INTO varient (varient, valias) VALUES ("Full Lamp Stack", "lamp");
INSERT INTO varient (varient, valias) VALUES ("LDAP Server", "ldap");
INSERT INTO varient (varient, valias) VALUES ("IMAP Server", "imap");
INSERT INTO varient (varient, valias) VALUES ("SMTP Server", "smtp");
INSERT INTO varient (varient, valias) VALUES ("DNS Server", "dns");
--
-- Create package lists for the above builds
INSERT INTO packages (package, varient_id, os_id) SELECT "logwatch", v.varient_id, o.os_id FROM varient v, build_os o;
INSERT INTO packages (package, varient_id, os_id) SELECT "ntp", v.varient_id, o.os_id FROM varient v, build_os o;
INSERT INTO packages (package, varient_id, os_id) SELECT "openssh-server", v.varient_id, o.os_id FROM varient v, build_os o;
INSERT INTO packages (package, varient_id, os_id) SELECT "postfix", v.varient_id, o.os_id FROM varient v, build_os o;
INSERT INTO packages (package, varient_id, os_id) SELECT "sudo", v.varient_id, o.os_id FROM varient v, build_os o;
INSERT INTO packages (package, varient_id, os_id) SELECT "sysstat", v.varient_id, o.os_id FROM varient v, build_os o;
INSERT INTO packages (package, varient_id, os_id) SELECT "ntp", v.varient_id, o.os_id FROM varient v, build_os o;
INSERT INTO packages (package, varient_id, os_id) SELECT "ntpdate", v.varient_id, o.os_id FROM varient v, build_os o;
INSERT INTO packages (package, varient_id, os_id) SELECT "nfs4-acl-tools", v.varient_id, o.os_id FROM varient v, build_os o;
INSERT INTO packages (package, varient_id, os_id) SELECT "less", v.varient_id, o.os_id FROM varient v, build_os o;

INSERT INTO packages (package, varient_id, os_id) SELECT "locate", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias IN ("debian", "ubuntu");
INSERT INTO packages (package, varient_id, os_id) SELECT "ldap-utils", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias IN ("debian", "ubuntu");
INSERT INTO packages (package, varient_id, os_id) SELECT "libnss-ldapd", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias IN ("debian", "ubuntu");
INSERT INTO packages (package, varient_id, os_id) SELECT "libpam-ldapd", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias IN ("debian", "ubuntu");
INSERT INTO packages (package, varient_id, os_id) SELECT "hobbit-client", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias IN ("debian", "ubuntu");
INSERT INTO packages (package, varient_id, os_id) SELECT "openldap", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias = "centos";
INSERT INTO packages (package, varient_id, os_id) SELECT "openldap-clients", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias = "centos";
INSERT INTO packages (package, varient_id, os_id) SELECT "nss-pam-ldapd", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias = "centos";
INSERT INTO packages (package, varient_id, os_id) SELECT "pam_ldap", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias = "centos";
INSERT INTO packages (package, varient_id, os_id) SELECT "openssl-perl", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias = "centos";

INSERT INTO packages (package, varient_id, os_id) SELECT "apache2", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias IN ("debian", "ubuntu") AND v.valias = "web";
INSERT INTO packages (package, varient_id, os_id) SELECT "libapache2-mod-php5", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias IN ("debian", "ubuntu") AND v.valias = "web";
INSERT INTO packages (package, varient_id, os_id) SELECT "mysql-client", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias IN ("debian", "ubuntu") AND v.valias = "web";
INSERT INTO packages (package, varient_id, os_id) SELECT "apache2", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias IN ("debian", "ubuntu") AND v.valias = "lamp";
INSERT INTO packages (package, varient_id, os_id) SELECT "libapache2-mod-php5", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias IN ("debian", "ubuntu") AND v.valias = "lamp";
INSERT INTO packages (package, varient_id, os_id) SELECT "mysql-client", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias IN ("debian", "ubuntu") AND v.valias = "lamp";
INSERT INTO packages (package, varient_id, os_id) SELECT "httpd", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias = "centos" AND v.valias = "web";
INSERT INTO packages (package, varient_id, os_id) SELECT "php", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias = "centos" AND v.valias = "web";
INSERT INTO packages (package, varient_id, os_id) SELECT "mysql", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias = "centos" AND v.valias = "web";
INSERT INTO packages (package, varient_id, os_id) SELECT "httpd", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias = "centos" AND v.valias = "lamp";
INSERT INTO packages (package, varient_id, os_id) SELECT "php", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias = "centos" AND v.valias = "lamp";
INSERT INTO packages (package, varient_id, os_id) SELECT "mysql", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias = "centos" AND v.valias = "lamp";

INSERT INTO packages (package, varient_id, os_id) SELECT "mysql-server", v.varient_id, o.os_id FROM varient v, build_os o WHERE v.valias = "lamp";
INSERT INTO packages (package, varient_id, os_id) SELECT "mysql-server", v.varient_id, o.os_id FROM varient v, build_os o WHERE v.valias = "mysql";

INSERT INTO packages (package, varient_id, os_id) SELECT "postfix", v.varient_id, o.os_id FROM varient v, build_os o WHERE v.valias = "smtp";

INSERT INTO packages (package, varient_id, os_id) SELECT "dovecot", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias = "centos" AND v.valias = "imap";
INSERT INTO packages (package, varient_id, os_id) SELECT "dovecot-imapd", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias IN ("debian", "ubuntu") AND v.valias = "imap";

INSERT INTO packages (package, varient_id, os_id) SELECT "slapd", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias IN ("debian", "ubuntu") AND v.valias = "ldap";
INSERT INTO packages (package, varient_id, os_id) SELECT "slapd", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias = "centos" AND v.valias = "openldap-servers";

INSERT INTO packages (package, varient_id, os_id) SELECT "bind9", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias IN ("debian", "ubuntu") AND v.valias = "dns";
INSERT INTO packages (package, varient_id, os_id) SELECT "bind9utils", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias IN ("debian", "ubuntu") AND v.valias = "dns";
INSERT INTO packages (package, varient_id, os_id) SELECT "bind9-host", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias IN ("debian", "ubuntu") AND v.valias = "dns";
INSERT INTO packages (package, varient_id, os_id) SELECT "bind", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias = "centos" AND v.valias = "dns";
INSERT INTO packages (package, varient_id, os_id) SELECT "bind-utils", v.varient_id, o.os_id FROM varient v, build_os o WHERE o.alias = "centos" AND v.valias = "dns";
--
-- Partitions
INSERT INTO seed_schemes (scheme_name, lvm) VALUES ("base", 0);
INSERT INTO seed_schemes (scheme_name, lvm) VALUES ("base-lvm", 1);
INSERT INTO seed_schemes (scheme_name, lvm) VALUES ("base-var", 0);
INSERT INTO seed_schemes (scheme_name, lvm) VALUES ("base-var-lvm", 1);
INSERT INTO seed_schemes (scheme_name, lvm) VALUES ("full", 0);
INSERT INTO seed_schemes (scheme_name, lvm) VALUES ("full-lvm", 1);

INSERT INTO default_part (minimum, maximum, priority, mount_point, filesystem, def_scheme_id, logical_volume) SELECT 5120, 30720, 100, "/", "ext4", def_scheme_id, "none" FROM seed_schemes WHERE scheme_name IN ("base", "base-var", "full");
INSERT INTO default_part (minimum, maximum, priority, mount_point, filesystem, def_scheme_id, logical_volume) SELECT 5120, 15360, 100, "/var", "ext4", def_scheme_id, "none" FROM seed_schemes WHERE scheme_name IN ("base-var", "full");
INSERT INTO default_part (minimum, maximum, priority, mount_point, filesystem, def_scheme_id, logical_volume) SELECT 5120, 40960, 100, "/usr", "ext4", def_scheme_id, "none" FROM seed_schemes WHERE scheme_name = "full";
INSERT INTO default_part (minimum, maximum, priority, mount_point, filesystem, def_scheme_id, logical_volume) SELECT 5120, 20480, 100, "/home", "ext4", def_scheme_id, "none" FROM seed_schemes WHERE scheme_name = "full";
INSERT INTO default_part (minimum, maximum, priority, mount_point, filesystem, def_scheme_id, logical_volume) SELECT 1024, 10240, 80, "swap", "swap", def_scheme_id, "none" FROM seed_schemes WHERE scheme_name IN ("base", "base-var", "full");
INSERT INTO default_part (minimum, maximum, priority, mount_point, filesystem, def_scheme_id, logical_volume) SELECT 5120, 30720, 100, "/", "ext4", def_scheme_id, "root" FROM seed_schemes WHERE scheme_name IN ("base-lvm", "base-var-lvm", "full-lvm");
INSERT INTO default_part (minimum, maximum, priority, mount_point, filesystem, def_scheme_id, logical_volume) SELECT 5120, 15360, 100, "/var", "ext4", def_scheme_id, "var" FROM seed_schemes WHERE scheme_name IN ("base-var-lvm", "full-lvm");
INSERT INTO default_part (minimum, maximum, priority, mount_point, filesystem, def_scheme_id, logical_volume) SELECT 5120, 40960, 100, "/usr", "ext4", def_scheme_id, "usr" FROM seed_schemes WHERE scheme_name = "full-lvm";
INSERT INTO default_part (minimum, maximum, priority, mount_point, filesystem, def_scheme_id, logical_volume) SELECT 5120, 20480, 100, "/home", "ext4", def_scheme_id, "home" FROM seed_schemes WHERE scheme_name = "full-lvm";
INSERT INTO default_part (minimum, maximum, priority, mount_point, filesystem, def_scheme_id, logical_volume) SELECT 1024, 10240, 80, "swap", "swap", def_scheme_id, "swap" FROM seed_schemes WHERE scheme_name IN ("base-lvm", "base-var-lvm", "full-lvm");
--
-- Service and hardware types
INSERT INTO hard_type (type, class) VALUES ("network", "Network Card");
INSERT INTO hard_type (type, class) VALUES ("storage", "Hard Disk");
INSERT INTO hard_type (type, class) VALUES ("storage", "CD-ROM");
INSERT INTO hard_type (type, class) VALUES ("cpu", "AMD CPU");
INSERT INTO hard_type (type, class) VALUES ("cpu", "Intel CPU");
INSERT INTO hard_type (type, class) VALUES ("cpu", "Virtual CPU");
INSERT INTO hard_type (type, class) VALUES ("ram", "RAM Modules");
INSERT INTO hard_type (type, class) VALUES ("ram", "Virtual RAM");
INSERT INTO hard_type (type, class) VALUES ("fibre", "Fibre Card");

INSERT INTO service_type (service, detail) VALUES ("imap", "Email retrieval system");
INSERT INTO service_type (service, detail) VALUES ("smtp", "Email delivery system");
INSERT INTO service_type (service, detail) VALUES ("http", "Web sites");
INSERT INTO service_type (service, detail) VALUES ("dns", "Domain name system");
INSERT INTO service_type (service, detail) VALUES ("mysql", "MySQL Database");
INSERT INTO service_type (service, detail) VALUES ("ldap", "LDAP Directory");
--
-- Still need to add locales and options
