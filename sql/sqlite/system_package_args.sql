CREATE TABLE `system_package_args` (
`syspack_arg_id` INTEGER PRIMARY KEY,
`syspack_id` int(11) NOT NULL,
`field` varchar(127) NOT NULL,
`type` varchar(31) NOT NULL,
`cuser` int(11) NOT NULL DEFAULT 0,
`muser` int(11) NOT NULL DEFAULT 0,
`ctime` timestamp NOT NULL DEFAULT 0,
`mtime` timestamp NOT NULL DEFAULT 0,
FOREIGN KEY(`syspack_id`)
REFERENCES `system_packages`(`syspack_id`)
ON UPDATE CASCADE ON DELETE CASCADE
);
CREATE TRIGGER insert_system_package_args AFTER INSERT ON system_package_args
BEGIN
UPDATE system_package_args SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP where syspack_arg_id = new.syspack_arg_id;
END;
CREATE TRIGGER update_system_package_args AFTER UPDATE ON system_package_args
BEGIN
UPDATE system_package_args SET mtime = CURRENT_TIMESTAMP WHERE syspack_arg_id = new.syspack_arg_id;
END;
