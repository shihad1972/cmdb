-- Create build_type entries
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES ("debian", "preseed", "url", "http://${HOSTNAME}.${DOMAIN}/cmdb/", "$MIRROR", "auto=true priority=critical vga=788");
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES ("ubuntu", "preseed", "url", "http://${HOSTNAME}.${DOMAIN}/cmdb/", "$MIRROR", "auto=true priority=critical vga=788");
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES ("centos", "kickstart", "ks", "http://${HOSTNAME}.${DOMAIN}/cmdb/", "$MIRROR", "ksdevice=eth0 console=tty0 ramdisk_size=8192");
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES ("fedora", "kickstart", "ks", "http://${HOSTNAME}.${DOMAIN}/cmdb/", "$MIRROR", "ksdevice=eth0 console=tty0 ramdisk_size=8192");

