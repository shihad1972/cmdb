CREATE TABLE preferred_a (
	prefa_id INTEGER PRIMARY KEY,
	ip VARCHAR(15) NOT NULL DEFAULT '0.0.0.0',
	ip_addr INTEGER NOT NULL DEFAULT 0,
	record_id INTEGER NOT NULL,
	fqdn VARCHAR(255) NOT NULL DEFAULT 'none');
