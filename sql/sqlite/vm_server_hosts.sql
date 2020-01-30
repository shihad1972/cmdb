CREATE TABLE `vm_server_hosts` (
  `vm_server_id` INTEGER PRIMARY KEY,
  `vm_server` varchar(255) NOT NULL,
  `type` varchar(31) NOT NULL,
  `server_id` int(7) NOT NULL,
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY (`server_id`)
    REFERENCES `server`(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TRIGGER insert_vm_server_hosts AFTER INSERT ON vm_server_hosts
BEGIN
UPDATE vm_server_hosts SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE vm_server_id = new.vm_server_id;
END;
CREATE TRIGGER update_vm_server_hosts AFTER UPDATE ON vm_server_hosts
BEGIN
UPDATE vm_server_hosts SET mtime = CURRENT_TIMESTAMP WHERE vm_server_id = new.vm_server_id;
END;
