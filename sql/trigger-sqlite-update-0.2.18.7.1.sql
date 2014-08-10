CREATE TRIGGER insert_build AFTER INSERT ON build
BEGIN
UPDATE build SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE build_id = new.build_id;
END;

CREATE TRIGGER update_build AFTER UPDATE ON build
BEGIN
UPDATE build SET mtime = CURRENT_TIMESTAMP WHERE build_id = new.build_id;
END;

CREATE TRIGGER insert_build_domain AFTER INSERT ON build_domain
BEGIN
UPDATE build_domain SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE bd_id = new.bd_id;
END;

CREATE TRIGGER update_build_domain AFTER UPDATE ON build_domain
BEGIN
UPDATE build_domain SET mtime = CURRENT_TIMESTAMP WHERE bd_id = new.bd_id;
END;

CREATE TRIGGER insert_build_ip AFTER INSERT ON build_ip
BEGIN
UPDATE build_ip SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE ip_id = new.ip_id;
END;

CREATE TRIGGER update_build_ip AFTER UPDATE ON build_ip
BEGIN
UPDATE build_ip SET mtime = CURRENT_TIMESTAMP WHERE ip_id = new.ip_id;
END;

CREATE TRIGGER insert_build_os AFTER INSERT ON build_os
BEGIN
UPDATE build_os SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE os_id = new.os_id;
END;

CREATE TRIGGER update_build_os AFTER UPDATE ON build_os
BEGIN
UPDATE build_os SET mtime = CURRENT_TIMESTAMP WHERE os_id = new.os_id;
END;

CREATE TRIGGER insert_contacts AFTER INSERT ON contacts
BEGIN
UPDATE contacts SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE cont_id = new.cont_id;
END;

CREATE TRIGGER update_contacts AFTER UPDATE ON contacts
BEGIN
UPDATE contacts SET mtime = CURRENT_TIMESTAMP WHERE cont_id = new.cont_id;
END;

CREATE TRIGGER insert_customer AFTER INSERT ON customer
BEGIN
UPDATE customer SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE cust_id = new.cust_id;
END;

CREATE TRIGGER update_customer AFTER UPDATE ON customer
BEGIN
UPDATE customer SET mtime = CURRENT_TIMESTAMP WHERE cust_id = new.cust_id;
END;

CREATE TRIGGER insert_glue_zones AFTER INSERT ON glue_zones
BEGIN
UPDATE glue_zones SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;

CREATE TRIGGER update_glue_zones AFTER UPDATE ON glue_zones
BEGIN
UPDATE glue_zones SET mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;

CREATE TRIGGER insert_hardware AFTER INSERT ON hardware
BEGIN
UPDATE hardware SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE hard_id = new.hard_id;
END;

CREATE TRIGGER update_hardware AFTER UPDATE ON hardware
BEGIN
UPDATE hardware SET mtime = CURRENT_TIMESTAMP WHERE hard_id = new.hard_id;
END;

CREATE TRIGGER insert_packages AFTER INSERT ON packages
BEGIN
UPDATE packages SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE pack_id = new.pack_id;
END;

CREATE TRIGGER update_packages AFTER UPDATE ON packages
BEGIN
UPDATE packages SET mtime = CURRENT_TIMESTAMP WHERE pack_id = new.pack_id;
END;

CREATE TRIGGER insert_preferred_a AFTER INSERT ON preferred_a
BEGIN
UPDATE preferred_a SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE prefa_id = new.prefa_id;
END;

CREATE TRIGGER update_preferred_a AFTER UPDATE ON preferred_a
BEGIN
UPDATE preferred_a SET mtime = CURRENT_TIMESTAMP WHERE prefa_id = new.prefa_id;
END;

CREATE TRIGGER insert_records AFTER INSERT ON records
BEGIN
UPDATE records SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;

CREATE TRIGGER update_records AFTER UPDATE ON records
BEGIN
UPDATE records SET mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;

CREATE TRIGGER insert_rev_records AFTER INSERT ON rev_records
BEGIN
UPDATE rev_records SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE rev_record_id = new.rev_record_id;
END;

CREATE TRIGGER update_rev_records AFTER UPDATE ON rev_records
BEGIN
UPDATE rev_records SET mtime = CURRENT_TIMESTAMP WHERE rev_record_id = new.rev_record_id;
END;

CREATE TRIGGER insert_rev_zones AFTER INSERT ON rev_zones
BEGIN
UPDATE rev_zones SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE rev_zone_id = new.rev_zone_id;
END;

CREATE TRIGGER update_rev_zones AFTER UPDATE ON rev_zones
BEGIN
UPDATE rev_zones SET mtime = CURRENT_TIMESTAMP WHERE rev_zone_id = new.rev_zone_id;
END;

CREATE TRIGGER insert_seed_schemes AFTER INSERT ON seed_schemes
BEGIN
UPDATE seed_schemes SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE def_scheme_id = new.def_scheme_id;
END;

CREATE TRIGGER update_seed_schemes AFTER UPDATE ON seed_schemes
BEGIN
UPDATE seed_schemes SET mtime = CURRENT_TIMESTAMP WHERE def_scheme_id = new.def_scheme_id;
END;

CREATE TRIGGER insert_server AFTER INSERT ON server
BEGIN
UPDATE server SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE server_id = new.server_id;
END;

CREATE TRIGGER update_server AFTER UPDATE ON server
BEGIN
UPDATE server SET mtime = CURRENT_TIMESTAMP WHERE server_id = new.server_id;
END;

CREATE TRIGGER insert_services AFTER INSERT ON services
BEGIN
UPDATE services SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE service_id = new.service_id;
END;

CREATE TRIGGER update_services AFTER UPDATE ON services
BEGIN
UPDATE services SET mtime = CURRENT_TIMESTAMP WHERE service_id = new.service_id;
END;

CREATE TRIGGER insert_varient AFTER INSERT ON varient
BEGIN
UPDATE varient SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE varient_id = new.varient_id;
END;

CREATE TRIGGER update_varient AFTER UPDATE ON varient
BEGIN
UPDATE varient SET mtime = CURRENT_TIMESTAMP WHERE varient_id = new.varient_id;
END;

CREATE TRIGGER insert_vm_server_hosts AFTER INSERT ON vm_server_hosts
BEGIN
UPDATE vm_server_hosts SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE vm_server_id = new.vm_server_id;
END;

CREATE TRIGGER update_vm_server_hosts AFTER UPDATE ON vm_server_hosts
BEGIN
UPDATE vm_server_hosts SET mtime = CURRENT_TIMESTAMP WHERE vm_server_id = new.vm_server_id;
END;

CREATE TRIGGER insert_zones AFTER INSERT ON zones
BEGIN
UPDATE zones SET ctime = CURRENT_TIMESTAMP, mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;

CREATE TRIGGER update_zones AFTER UPDATE ON zones
BEGIN
UPDATE zones SET mtime = CURRENT_TIMESTAMP WHERE id = new.id;
END;

