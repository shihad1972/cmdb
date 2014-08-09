CREATE TRIGGER locale_update BEFORE UPDATE ON locale FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER locale_insert BEFORE INSERT ON locale FOR EACH ROW set NEW.mtime = NOW();

