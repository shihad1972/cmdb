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

