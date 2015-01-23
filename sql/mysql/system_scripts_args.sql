CREATE TABLE `system_scripts_args` (
`systscr_arg_id` int(7) NOT NULL AUTO_INCREMENT,
`systscr_id` int(11) NOT NULL,
`bd_id` int(11) NOT NULL,
`bt_id` int(11) NOT NULL,
`arg` varchar(127) NOT NULL,
`no` int(11) NOT NULL,
`cuser` int(11) NOT NULL DEFAULT 0,
`muser` int(11) NOT NULL DEFAULT 0,
`ctime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
`mtime` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
PRIMARY KEY (`systscr_arg_id`),
KEY `systscr_id` (`systscr_id`),
KEY `bd_id` (`bd_id`),
KEY `bt_id` (`bt_id`),
FOREIGN KEY(`systscr_id`)
REFERENCES `system_scripts`(`systscr_id`)
ON UPDATE CASCADE ON DELETE CASCADE,
FOREIGN KEY(`bd_id`)
REFERENCES `build_domain`(`bd_id`)
ON UPDATE CASCADE ON DELETE CASCADE,
FOREIGN KEY(`bt_id`)
REFERENCES `build_type`(`bt_id`)
ON UPDATE CASCADE ON DELETE CASCADE
);

CREATE TRIGGER system_scripts_args_update BEFORE UPDATE ON system_scripts_args FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER system_scripts_args_insert BEFORE INSERT ON system_scripts_args FOR EACH ROW set NEW.mtime = NOW();

