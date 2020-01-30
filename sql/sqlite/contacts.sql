CREATE TABLE `contacts` (
  `cont_id` INTEGER PRIMARY KEY,
  `name` varchar(50) NOT NULL,
  `phone` varchar(20) NOT NULL,
  `email` varchar(50) NOT NULL,
  `cust_id` int(7) NOT NULL,
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`cust_id`)
    REFERENCES `customer` (`cust_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TRIGGER insert_contacts AFTER INSERT ON contacts
BEGIN
UPDATE contacts SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE cont_id = new.cont_id;
END;
CREATE TRIGGER update_contacts AFTER UPDATE ON contacts
BEGIN
UPDATE contacts SET mtime = CURRENT_TIMESTAMP WHERE cont_id = new.cont_id;
END;
