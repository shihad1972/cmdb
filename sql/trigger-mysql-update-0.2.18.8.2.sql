CREATE TRIGGER default_part_update BEFORE UPDATE ON default_part FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER default_part_insert BEFORE INSERT ON default_part FOR EACH ROW set NEW.mtime = NOW();

