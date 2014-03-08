ALTER TABLE records ADD COLUMN `protocol` varchar(15) NOT NULL AFTER type;
ALTER TABLE records ADD COLUMN `service` varchar(15) NOT NULL AFTER protocol;