CREATE TABLE `default_locale` (
`restrict` ENUM('') NOT NULL,
`locale_id` int(7) NOT NULL,
`cuser` int(11) NOT NULL DEFAULT 0,
`muser` int(11) NOT NULL DEFAULT 0,
`ctime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
`mtime` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
PRIMARY KEY (`restrict`),
KEY `locale_id` (`locale_id`),
FOREIGN KEY(`locale_id`)
REFERENCES `locale`(`locale_id`)
ON UPDATE CASCADE ON DELETE CASCADE
);

CREATE TRIGGER default_locale_update BEFORE UPDATE ON default_locale FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER default_locale_insert BEFORE INSERT ON default_locale FOR EACH ROW set NEW.mtime = NOW();

