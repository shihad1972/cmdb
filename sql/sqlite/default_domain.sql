CREATE TABLE `default_domain` (
  `restrict` int(7) UNIQUE DEFAULT 0 CHECK(restrict = 0),
  `bd_id` int(7) NOT NULL,
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`bd_id`)
    REFERENCES `build_domain` (`bd_id`)
    ON DELETE CASCADE ON UPDATE CASCADE
);
CREATE TRIGGER insert_default_domain AFTER INSERT ON default_domain
BEGIN
UPDATE default_domain SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE bd_id = new.bd_id;
END;
CREATE TRIGGER update_default_domain AFTER UPDATE ON default_domain
BEGIN
UPDATE default_domain SET mtime = CURRENT_TIMESTAMP WHERE bd_id = new.bd_id;
END;
