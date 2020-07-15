CREATE TABLE `default_customer` (
  `restrict` int(7) UNIQUE DEFAULT 0 CHECK(restrict = 0),
  `cust_id` int(7) NOT NULL,
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`cust_id`)
    REFERENCES `customer` (`cust_id`)
    ON DELETE CASCADE ON UPDATE CASCADE
);
CREATE TRIGGER insert_default_customer AFTER INSERT ON default_customer
BEGIN
UPDATE default_customer SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE cust_id = new.cust_id;
END;
CREATE TRIGGER update_default_customer AFTER UPDATE ON default_customer
BEGIN
UPDATE default_customer SET mtime = CURRENT_TIMESTAMP WHERE cust_id = new.cust_id;
END;
