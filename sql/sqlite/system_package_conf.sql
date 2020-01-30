CREATE TABLE `system_package_conf` (
`syspack_conf_id` INTEGER PRIMARY KEY,
`syspack_arg_id` int(11) NOT NULL,
`syspack_id` int(11) NOT NULL,
`bd_id` int(11) NOT NULL,
`arg` varchar(255) NOT NULL,
`cuser` int(11) NOT NULL DEFAULT 0,
`muser` int(11) NOT NULL DEFAULT 0,
`ctime` timestamp NOT NULL DEFAULT 0,
`mtime` timestamp NOT NULL DEFAULT 0,
FOREIGN KEY(`syspack_id`)
REFERENCES `system_packages`(`syspack_id`)
ON UPDATE CASCADE ON DELETE CASCADE,
FOREIGN KEY(`syspack_arg_id`)
REFERENCES `system_package_args`(`syspack_arg_id`)
ON UPDATE CASCADE ON DELETE CASCADE
);
CREATE TRIGGER insert_system_package_conf AFTER INSERT ON system_package_conf
BEGIN
UPDATE system_package_conf SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP where syspack_conf_id = new.syspack_conf_id;
END;
CREATE TRIGGER update_system_package_conf AFTER UPDATE ON system_package_conf
BEGIN
UPDATE system_package_conf SET mtime = CURRENT_TIMESTAMP WHERE syspack_conf_id = new.syspack_conf_id;
END;
