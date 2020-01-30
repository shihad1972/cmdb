CREATE TABLE `build` (
  `build_id` INTEGER PRIMARY KEY,
  `mac_addr` varchar(17) NOT NULL,
  `varient_id` int(7) NOT NULL,
  `net_inst_int` varchar(15) NOT NULL,
  `server_id` int(7) NOT NULL,
  `os_id` int(7) NOT NULL,
  `ip_id` int(7) NOT NULL,
  `locale_id` int(7) NOT NULL,
  `def_scheme_id` int(7) NOT NULL,
  `cuser` int(11) NOT NULL DEFAULT 0,
  `muser` int(11) NOT NULL DEFAULT 0,
  `ctime` timestamp NOT NULL DEFAULT 0,
  `mtime` timestamp NOT NULL DEFAULT 0,

  FOREIGN KEY(`varient_id`)
    REFERENCES `varient`(`varient_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`os_id`)
    REFERENCES `build_os`(`os_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`ip_id`)
    REFERENCES `build_ip`(`ip_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`server_id`)
    REFERENCES `server`(`server_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`locale_id`)
    REFERENCES `locale`(`locale_id`)
    ON UPDATE CASCADE ON DELETE CASCADE,

  FOREIGN KEY(`def_scheme_id`)
    REFERENCES `seed_schemes`(`def_scheme_id`)
    ON UPDATE CASCADE ON DELETE CASCADE

);
CREATE TRIGGER insert_build AFTER INSERT ON build
BEGIN
UPDATE build SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE build_id = new.build_id;
END;
CREATE TRIGGER update_build AFTER UPDATE ON build
BEGIN
UPDATE build SET mtime = CURRENT_TIMESTAMP WHERE build_id = new.build_id;
END;

