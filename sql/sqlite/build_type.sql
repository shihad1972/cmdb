CREATE TABLE `build_type` (
  `bt_id` INTEGER PRIMARY KEY,
  `alias` varchar(25) NOT NULL,
  `build_type` varchar(25) NOT NULL DEFAULT 'none',
  `arg` varchar(15) NOT NULL DEFAULT 'none',
  `url` varchar(255) NOT NULL DEFAULT 'none',
  `mirror` varchar(255) NOT NULL DEFAULT 'none',
  `boot_line` varchar(127) NOT NULL DEFAULT 'none'
);
