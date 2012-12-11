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
