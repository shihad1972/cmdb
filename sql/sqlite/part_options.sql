CREATE TABLE `part_options` (
  `part_options_id` INTEGER PRIMARY KEY,
  `def_part_id` INTEGER NOT NULL,
  `def_scheme_id` INTEGER NOT NULL,
  `poption` varchar(31) NOT NULL,
  `cuser` INTEGER NOT NULL DEFAULT 0,
  `muser` INTEGER NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

    FOREIGN KEY(`def_part_id`)
      REFERENCES `default_part`(`def_part_id`)
      ON UPDATE CASCADE ON DELETE CASCADE,

    FOREIGN KEY(`def_scheme_id`)
      REFERENCES `seed_schemes`(`def_scheme_id`)
      ON UPDATE CASCADE ON DELETE CASCADE

);

CREATE TRIGGER insert_part_options AFTER INSERT ON part_options
BEGIN
UPDATE part_options SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE part_options_id = new.part_options_id;
END;
CREATE TRIGGER update_part_options AFTER UPDATE on part_options
BEGIN
UPDATE part_options SET mtime = CURRENT_TIMESTAMP WHERE part_options_id = new.part_options_id;
END;

