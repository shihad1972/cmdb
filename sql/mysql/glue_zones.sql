CREATE TABLE `glue_zones` (
  `id` int(7) NOT NULL AUTO_INCREMENT,
  `zone_id` int(7) NOT NULL DEFAULT '0',
  `name` varchar(255) NOT NULL,
  `pri_ns` varchar(255) NOT NULL,
  `sec_ns` varchar(255) NOT NULL DEFAULT 'none',
  `pri_dns` varchar(15) NOT NULL,
  `sec_dns` varchar(15) NOT NULL DEFAULT 'none',
  PRIMARY KEY (`id`),

  INDEX (`name`, `zone_id`),

  FOREIGN KEY (`zone_id`)
    REFERENCES `zones`(`id`)
    ON UPDATE CASCADE ON DELETE CASCADE

) ENGINE=InnoDB DEFAULT CHARSET=latin1;

