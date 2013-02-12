ALTER TABLE zones MODIFY COLUMN valid varchar(15) NOT NULL DEFAULT 'unknown';
ALTER TABLE zones MODIFY COLUMN updated varchar(15) NOT NULL DEFAULT 'yes';
ALTER TABLE rev_zones MODIFY COLUMN updated varchar(15) NOT NULL DEFAULT 'unknown';
ALTER TABLE rev_zones MODIFY COLUMN valid varchar(15) NOT NULL DEFAULT 'yes';
ALTER TABLE records MODIFY COLUMN valid varchar(15) NOT NULL DEFAULT 'unknown';
ALTER TABLE rev_records MODIFY COLUMN valid varchar(15) NOT NULL DEFAULT 'unknown';
