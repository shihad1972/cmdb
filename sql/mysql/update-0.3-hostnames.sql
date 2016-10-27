ALTER TABLE build ADD COLUMN server_name VARCHAR(255) AFTER build_id;
ALTER TABLE server ADD COLUMN server_name VARCHAR(255) AFTER server_id;
