CREATE TABLE `system_packages` (
`syspack_id` INTEGER PRIMARY KEY,
`name` varchar(127) NOT NULL,
`cuser` int(11) NOT NULL DEFAULT 0,
`muser` int(11) NOT NULL DEFAULT 0,
`ctime` timestamp NOT NULL DEFAULT 0,
`mtime` timestamp NOT NULL DEFAULT 0
);
CREATE TRIGGER insert_system_packages AFTER INSERT ON system_packages
BEGIN
UPDATE system_packages SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE syspack_id = new.syspack_id;
END;
CREATE TRIGGER update_system_packages AFTER UPDATE ON system_packages
BEGIN
UPDATE system_packages SET mtime = CURRENT_TIMESTAMP WHERE syspack_id = new.syspack_id;
END;
