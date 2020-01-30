CREATE TABLE `default_locale` (
  `restrict` int(7) UNIQUE DEFAULT 0 CHECK(restrict = 0),
  `locale_id` int(7) NOT NULL,
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`locale_id`)
    REFERENCES `locale` (`locale_id`)
    ON DELETE CASCADE ON UPDATE CASCADE
);
CREATE TRIGGER insert_default_locale AFTER INSERT ON default_locale
BEGIN
UPDATE default_locale SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE locale_id = new.locale_id;
END;
CREATE TRIGGER update_default_locale AFTER UPDATE ON default_locale
BEGIN
UPDATE default_locale SET mtime = CURRENT_TIMESTAMP WHERE locale_id = new.locale_id;
END;
