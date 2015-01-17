CREATE TABLE `system_scripts_args` (
`systscr_arg_id` INTEGER PRIMARY KEY,
`systscr_id` int(11) NOT NULL,
`bd_id` int(11),
`arg` varchar(127) NOT NULL,
`no` int(11) NOT NULL,
`cuser` int(11) NOT NULL DEFAULT 0,
`muser` int(11) NOT NULL DEFAULT 0,
`ctime` timestamp NOT NULL DEFAULT 0,
`mtime` timestamp NOT NULL DEFAULT 0,
FOREIGN KEY(`systscr_id`)
REFERENCES `system_scripts`(`systscr_id`)
ON UPDATE CASCADE ON DELETE CASCADE,
FOREIGN KEY(`bd_id`)
REFERENCES `build_domain`(`bd_id`)
ON UPDATE CASCADE ON DELETE CASCADE
);
CREATE TRIGGER insert_system_scripts_args AFTER INSERT ON system_scripts_args
BEGIN
UPDATE system_scripts_args SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP where systscr_arg_id = new.systscr_arg_id;
END;
CREATE TRIGGER update_system_scripts_args AFTER UPDATE ON system_scripts_args
BEGIN
UPDATE system_scripts_args SET mtime = CURRENT_TIMESTAMP WHERE systscr_arg_id = new.systscr_arg_id;
END;
