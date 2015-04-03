CREATE TABLE `part_options` (
  `part_options_id` int(7) NOT NULL AUTO_INCREMENT,
  `def_part_id` int(11) NOT NULL,
  `def_scheme_id` int(11) NOT NULL,
  `poption` varchar(31) NOT NULL,
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `mtime` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
PRIMARY KEY (`part_options_id`),
KEY `def_part_id` (`def_part_id`),
KEY `def_scheme_id` (`def_scheme_id`),
FOREIGN KEY(`def_part_id`)
REFERENCES `default_part`(`def_part_id`)
ON UPDATE CASCADE ON DELETE CASCADE,
FOREIGN KEY(`def_scheme_id`)
REFERENCES `seed_schemes`(`def_scheme_id`)
ON UPDATE CASCADE ON DELETE CASCADE
);

CREATE TRIGGER part_options_insert BEFORE INSERT ON part_options FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER part_options_update BEFORE UPDATE ON part_options FOR EACH ROW set NEW.mtime = NOW();

