CREATE TABLE `disk_dev` (
  `disk_id` INTEGER PRIMARY KEY,
  `server_id` int(7) NOT NULL,
  `device` varchar(63) NOT NULL,
  `lvm` smallint(4) NOT NULL,

  FOREIGN KEY(`server_id`)
    REFERENCES server(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
