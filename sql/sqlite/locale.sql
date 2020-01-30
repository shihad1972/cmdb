CREATE TABLE `locale` (
  `locale_id` INTEGER PRIMARY KEY,
  `locale` varchar(31) NOT NULL DEFAULT 'en_GB',
  `country` varchar(15) NOT NULL DEFAULT 'GB',
  `language` varchar(15) NOT NULL DEFAULT 'en',
  `keymap` varchar(15) NOT NULL DEFAULT 'gb',
  `timezone` varchar(63) NOT NULL DEFAULT 'Europe/London',
  `name` varchar(127) NOT NULL,
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0

);
CREATE TRIGGER insert_locale AFTER INSERT ON locale
BEGIN
UPDATE locale SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE locale_id = new.locale_id;
END;
CREATE TRIGGER update_locale AFTER UPDATE ON locale
BEGIN
UPDATE locale SET mtime = CURRENT_TIMESTAMP WHERE locale_id = new.locale_id;
END;
