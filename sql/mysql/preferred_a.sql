CREATE TABLE preferred_a (
	prefa_id INT(7) NOT NULL AUTO_INCREMENT,
	ip VARCHAR(15) NOT NULL DEFAULT '0.0.0.0',
	ip_addr INT(4) UNSIGNED NOT NULL DEFAULT 0,
	record_id INT(7) NOT NULL,
	fqdn VARCHAR(255) NOT NULL DEFAULT 'none',
	PRIMARY KEY (prefa_id)
	) ENGINE=InnoDB;
