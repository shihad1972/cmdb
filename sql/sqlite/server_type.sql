CREATE TABLE `server_type` (
  `server_type_id` INTEGER PRIMARY KEY,
  `vendor` varchar(63) NOT NULL DEFAULT 'none',
  `make` varchar(63) NOT NULL DEFAULT 'none',
  `model` varchar(31) NOT NULL DEFAULT 'none',
  `alias` varchar(31) NOT NULL,
  `cuser` int(11) NOT NULL DEFAULT '0',
  `muser` int(11) NOT NULL DEFAULT '0',
  `ctime` timestamp NOT NULL DEFAULT '0',
  `mtime` timestamp NOT NULL DEFAULT '0'
);

CREATE TRIGGER insert_server_type AFTER INSERT ON server_type
BEGIN
UPDATE server_type SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE server_type_id = new.server_type_id;
end;
CREATE TRIGGER update_server_type AFTER UPDATE ON server_type
BEGIN
UPDATE server_type SET mtime = CURRENT_TIMESTAMP WHERE server_type_id = new.server_type_id;
END;
