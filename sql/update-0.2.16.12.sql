ALTER TABLE records ADD COLUMN `protocol` varchar(15) AFTER type;
ALTER TABLE records ADD COLUMN `service` varchar(15) AFTER protocol;