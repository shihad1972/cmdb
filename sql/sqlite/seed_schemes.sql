CREATE TABLE `seed_schemes` (
  `def_scheme_id` INTEGER PRIMARY KEY,
  `scheme_name` varchar(79) NOT NULL,
  `lvm` smallint(4) NOT NULL,
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0
);
CREATE TRIGGER insert_seed_schemes AFTER INSERT ON seed_schemes
BEGIN
UPDATE seed_schemes SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE def_scheme_id = new.def_scheme_id;
END;
CREATE TRIGGER update_seed_schemes AFTER UPDATE ON seed_schemes
BEGIN
UPDATE seed_schemes SET mtime = CURRENT_TIMESTAMP WHERE def_scheme_id = new.def_scheme_id;
END;
