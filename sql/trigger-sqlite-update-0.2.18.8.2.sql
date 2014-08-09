CREATE TRIGGER insert_default_part AFTER INSERT ON default_part
BEGIN
UPDATE default_part SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE def_part_id = new.def_part_id;
END;

CREATE TRIGGER update_default_part AFTER UPDATE ON default_part
BEGIN
UPDATE default_part SET mtime = CURRENT_TIMESTAMP WHERE def_part_id = new.def_part_id;
END;

