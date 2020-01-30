CREATE TABLE `customer` (
  `cust_id` INTEGER PRIMARY KEY,
  `name` varchar(60) NOT NULL,
  `address` varchar(63) NOT NULL DEFAULT 'none',
  `city` varchar(31) NOT NULL DEFAULT 'none',
  `county` varchar(30) NOT NULL DEFAULT 'none',
  `postcode` varchar(10) NOT NULL DEFAULT 'none',
  `coid` varchar(8) NOT NULL,
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0
);

CREATE TRIGGER insert_customer AFTER INSERT ON customer
BEGIN
UPDATE customer SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE cust_id = new.cust_id;
END;
CREATE TRIGGER update_customer AFTER UPDATE ON customer
BEGIN
UPDATE customer SET mtime = CURRENT_TIMESTAMP WHERE cust_id = new.cust_id;
END;
