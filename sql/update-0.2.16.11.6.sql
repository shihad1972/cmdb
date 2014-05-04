ALTER TABLE vm_server_hosts MODIFY COLUMN vm_server varchar(255);
ALTER TABLE hard_type MODIFY COLUMN class varchar(31);
ALTER TABLE hard_type MODIFY COLUMN type varchar(31);
ALTER TABLE server MODIFY COLUMN name varchar(63);
ALTER TABLE service_type MODIFY COLUMN detail varchar(31);
