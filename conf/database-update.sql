#
# 13/12/2012
ALTER TABLE build_domain MODIFY start_ip int(4) unsigned NOT NULL DEFAULT 0;
ALTER TABLE build_domain MODIFY end_ip int(4) unsigned NOT NULL DEFAULT 0;
ALTER TABLE build_domain MODIFY netmask int(4) unsigned NOT NULL DEFAULT 0;
ALTER TABLE build_domain MODIFY gateway int(4) unsigned NOT NULL DEFAULT 0;
ALTER TABLE build_domain MODIFY ns int(4) unsigned NOT NULL DEFAULT 0;
ALTER TABLE build_domain MODIFY domain varchar(150) NOT NULL DEFAULT 'no.domain';
ALTER TABLE build_domain ADD COLUMN ntp_server varchar(64) NOT NULL DEFAULT 'shihad.org';
ALTER TABLE build_domain ADD COLUMN config_ntp tinyint(4) NOT NULL DEFAULT 1;
ALTER TABLE build_domain ADD COLUMN ldap_server varchar(64) NOT NULL DEFAULT 'ldap01.shihad.org';
ALTER TABLE build_domain ADD COLUMN ldap_ssl tinyint(4) NOT NULL DEFAULT 1;
ALTER TABLE build_domain ADD COLUMN ldap_dn varchar(96) NOT NULL DEFAULT 'dc=shihad,dc=org';
ALTER TABLE build_domain ADD COLUMN config_ldap tinyint(4) NOT NULL DEFAULT 1;
ALTER TABLE build_domain ADD COLUMN ldap_bind varchar(128) NOT NULL DEFAULT 'cn=thargoid,dc=shihad,dc=org';
ALTER TABLE build_domain ADD COLUMN log_server varchar(64) NOT NULL DEFAULT 'logger01.shihad.org';
ALTER TABLE build_domain ADD COLUMN config_log tinyint (4) NOT NULL DEFAULT 1;
ALTER TABLE build_domain ADD COLUMN email_server varchar(64) NOT NULL DEFAULT 'mail01.scots.shihad.org';
ALTER TABLE build_domain ADD COLUMN config_email tinyint (4) NOT NULL DEFAULT 1;
INSERT INTO locale (os_id, bt_id) SELECT os_id, bt_id FROM build_os WHERE alias = 'debian';
ALTER TABLE build_type ADD COLUMN mirror varchar(256) NOT NULL DEFAULT 'none';
UPDATE  build_type SET mirror = 'ftp.uk.debian.org' WHERE alias = 'debian';
ALTER TABLE build_domain ADD COLUMN xymon_server varchar(64) NOT NULL DEFAULT 'hobbit.shihad.org';
ALTER TABLE build_domain ADD COLUMN xymon_config tinyint NOT NULL DEFAULT 1;
ALTER TABLE build_domain CHANGE COLUMN email_server smtp_server varchar(64) NOT NULL DEFAULT 'mail01.scots.shihad.org';
ALTER TABLE build_domain CHANGE COLUMN ldap_server ldap_url varchar(96) NOT NULL DEFAULT 'ldap01.shihad.org';
UPDATE build_domain SET ntp_server = 'shihad.org';
## 17/12/2012
ALTER TABLE partitions ADD COLUMN logical_volume varchar(16) NOT NULL DEFAULT 'none';
ALTER TABLE build_domain CHANGE COLUMN xymon_config config_xymon tinyint;
## 18/12/2012
ALTER TABLE build_domain MODIFY COLUMN ldap_url varchar(128) NOT NULL DEFAULT 'ldap01.shihad.org';
UPDATE build_domain SET ldap_url = 'ldap01.shihad.org';
ALTER TABLE build_domain MODIFY COLUMN config_xymon tinyint NOT NULL DEFAULT 1;
ALTER TABLE build_domain MODIFY COLUMN xymon_server varchar(64) NOT NULL DEFAULT '192.168.1.50';
UPDATE build_domain SET xymon_server = '192.168.1.50';
