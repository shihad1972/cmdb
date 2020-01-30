CREATE TABLE `services` (
  `service_id` INTEGER PRIMARY KEY,
  `server_id` int(7) NOT NULL,
  `cust_id` int(7) NOT NULL,
  `service_type_id` int(7) NOT NULL,
  `detail` varchar(63) NOT NULL DEFAULT 'none',
  `url` varchar(255) NOT NULL DEFAULT 'none',
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`server_id`)
    REFERENCES `server`(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY (`cust_id`)
    REFERENCES `customer`(`cust_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY (`service_type_id`)
    REFERENCES `service_type`(`service_type_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TRIGGER insert_services AFTER INSERT ON services
BEGIN
UPDATE services SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE service_id = new.service_id;
END;
CREATE TRIGGER update_services AFTER UPDATE ON services
BEGIN
UPDATE services SET mtime = CURRENT_TIMESTAMP WHERE service_id = new.service_id;
END;
