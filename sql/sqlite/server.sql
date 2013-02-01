CREATE TABLE `server` (
  `server_id` INTEGER,
  `vendor` varchar(50) DEFAULT NULL,
  `make` varchar(50) DEFAULT NULL,
  `model` varchar(30) DEFAULT NULL,
  `uuid` varchar(50) DEFAULT NULL,
  `cust_id` int(7) NOT NULL DEFAULT 0,
  `vm_server_id` int(7) NOT NULL DEFAULT 0,
  `name` varchar(30) NOT NULL,
  PRIMARY KEY (`server_id` ASC)
);
