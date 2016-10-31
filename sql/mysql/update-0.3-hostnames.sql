CREATE TABLE `server_type` (
	`server_type_id` INT(7) NOT NULL AUTO_INCREMENT,
	`vendor` VARCHAR(63) NOT NULL DEFAULT 'none',
	`make` VARCHAR(63) NOT NULL DEFAULT 'none',
	`model` VARCHAR(31) NOT NULL DEFAULT 'none',
	`alias` VARCHAR(31) NOT NULL,
	`cuser` int(11) NOT NULL DEFAULT '0',
	`muser` int(11) NOT NULL DEFAULT '0',
	`ctime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
	`mtime` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
	PRIMARY KEY (`server_type_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

ALTER TABLE server ADD COLUMN server_type_id INT(7) AFTER vm_server_id;
ALTER TABLE server ADD CONSTRAINT fk_server_type_id FOREIGN KEY (`server_type_id`) REFERENCES `server_type` (`server_type_id`) ON DELETE CASCADE ON UPDATE CASCADE;
ALTER TABLE server DROP COLUMN `vendor`, DROP COLUMN `make`, DROP COLUMN `model`, DROP COLUMN `uuid`;

ALTER TABLE build ADD COLUMN server_name VARCHAR(255) AFTER build_id;
ALTER TABLE server ADD COLUMN server_name VARCHAR(255) AFTER server_id;
