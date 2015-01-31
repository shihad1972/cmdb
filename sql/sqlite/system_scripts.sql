CREATE TABLE `system_scripts` (
`systscr_id` INTEGER PRIMARY KEY,
`name` varchar(127) NOT NULL,
`cuser` int(11) NOT NULL DEFAULT 0,
`muser` int(11) NOT NULL DEFAULT 0,
`ctime` timestamp NOT NULL DEFAULT 0,
`mtime` timestamp NOT NULL DEFAULT 0
);
CREATE TRIGGER insert_system_scripts AFTER INSERT ON system_scripts
BEGIN
UPDATE system_scripts SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE systscr_id = new.systscr_id;
end;
CREATE TRIGGER update_system_scripts AFTER UPDATE ON system_scripts
BEGIN
UPDATE system_scripts SET mtime = CURRENT_TIMESTAMP WHERE systscr_id = new.systscr_id;
END;
