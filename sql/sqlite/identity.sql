CREATE TABLE `identity` (
  `identity_id` INTEGER PRIMARY KEY,
  `server_id` int(7) NOT NULL,
  `username` varchar(256) DEFAULT NULL,
  `pass` varchar(4096) DEFAULT NULL,
  `hash` varchar(256) DEFAULT NULL,
  `cuser` int(11) NOT NULL DEFAULT '0',
  `muser` int(11) NOT NULL DEFAULT '0',
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY(`server_id`)
    REFERENCES `server`(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TRIGGER insert_identity AFTER INSERT ON identity
BEGIN
UPDATE identity SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE identity_id = new.identity_id;
END;
CREATE TRIGGER update_identity AFTER UPDATE ON identity
BEGIN
UPDATE identity SET mtime = CURRENT_TIMESTAMP WHERE identity_id = new.identity_id;
END;
