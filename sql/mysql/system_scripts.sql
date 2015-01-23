CREATE TABLE `system_scripts` (
`systscr_id` int(7) NOT NULL AUTO_INCREMENT,
`name` varchar(127) NOT NULL,
`cuser` int(11) NOT NULL DEFAULT 0,
`muser` int(11) NOT NULL DEFAULT 0,
`ctime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
`mtime` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
PRIMARY KEY (`systscr_id`)
);

CREATE TRIGGER system_scripts_update BEFORE UPDATE ON system_scripts FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER system_scripts_insert BEFORE INSERT ON system_scripts FOR EACH ROW set NEW.mtime = NOW();
