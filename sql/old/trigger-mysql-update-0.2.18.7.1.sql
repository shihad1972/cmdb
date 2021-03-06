CREATE TRIGGER build_update BEFORE UPDATE ON build FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER build_domain_update BEFORE UPDATE ON build_domain FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER build_ip_update BEFORE UPDATE ON build_ip FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER build_os_update BEFORE UPDATE ON build_os FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER contacts_update BEFORE UPDATE ON contacts FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER customer_update BEFORE UPDATE ON customer FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER glue_zones_update BEFORE UPDATE ON glue_zones FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER hardware_update BEFORE UPDATE ON hardware FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER packages_update BEFORE UPDATE ON packages FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER preferred_a_update BEFORE UPDATE ON preferred_a FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER records_update BEFORE UPDATE ON records FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER rev_records_update BEFORE UPDATE ON rev_records FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER rev_zones_update BEFORE UPDATE ON rev_zones FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER seed_schemes_update BEFORE UPDATE ON seed_schemes FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER server_update BEFORE UPDATE ON server FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER services_update BEFORE UPDATE ON services FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER varient_update BEFORE UPDATE ON varient FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER vm_server_hosts_update BEFORE UPDATE ON vm_server_hosts FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER zones_update BEFORE UPDATE ON zones FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER build_insert BEFORE INSERT ON build FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER build_domain_insert BEFORE INSERT ON build_domain FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER build_ip_insert BEFORE INSERT ON build_ip FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER build_os_insert BEFORE INSERT ON build_os FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER contacts_insert BEFORE INSERT ON contacts FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER customer_insert BEFORE INSERT ON customer FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER glue_zones_insert BEFORE INSERT ON glue_zones FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER hardware_insert BEFORE INSERT ON hardware FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER packages_insert BEFORE INSERT ON packages FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER preferred_a_insert BEFORE INSERT ON preferred_a FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER records_insert BEFORE INSERT ON records FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER rev_records_insert BEFORE INSERT ON rev_records FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER rev_zones_insert BEFORE INSERT ON rev_zones FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER seed_schemes_insert BEFORE INSERT ON seed_schemes FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER server_insert BEFORE INSERT ON server FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER services_insert BEFORE INSERT ON services FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER varient_insert BEFORE INSERT ON varient FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER vm_server_hosts_insert BEFORE INSERT ON vm_server_hosts FOR EACH ROW set NEW.mtime = NOW();
CREATE TRIGGER zones_insert BEFORE INSERT ON zones FOR EACH ROW set NEW.mtime = NOW();
