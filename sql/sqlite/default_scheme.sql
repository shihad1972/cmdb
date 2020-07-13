CREATE TABLE `default_scheme` (
  `restrict` int(7) UNIQUE DEFAULT 0 CHECK(restrict = 0),
  `def_scheme_id` int(7) NOT NULL,
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`def_scheme_id`)
    REFERENCES `seed_schemes` (`def_scheme_id`)
    ON DELETE CASCADE ON UPDATE CASCADE
);
CREATE TRIGGER insert_default_scheme AFTER INSERT ON default_scheme
BEGIN
UPDATE default_scheme SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE def_scheme_id = new.def_scheme_id;
END;
CREATE TRIGGER update_default_scheme AFTER UPDATE ON default_scheme
BEGIN
UPDATE default_scheme SET mtime = CURRENT_TIMESTAMP WHERE def_scheme_id = new.def_scheme_id;
END;
