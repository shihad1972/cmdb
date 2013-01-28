INSERT INTO packages (package, varient_id, os_id) SELECT 'build-essential', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'hobbit-client', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'ldap-utils', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'less', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'libnss-ldap', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'locate', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'logwatch', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'nfs-common', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'ntp', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'ntpdate', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'openssh-server', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'postfix', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'sudo', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'tcpdump', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'whois', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu'
;
INSERT INTO packages (package, varient_id, os_id) SELECT 'dns-utils', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'mysql-client', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php5-curl', v.varient_id, bo.os_id FROM varient v, 
build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php5-gmp', v.varient_id, bo.os_id FROM varient v, b
uild_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php5-ldap', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php5-mysql', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php5-xcache', v.varient_id, bo.os_id FROM varient v , build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php-log', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubutnu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php-http', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php-html-common', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php-auth-http', v.varient_id, bo.os_id FROM varient  v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php-auth', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php-pear', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT 'php-db', v.varient_id, bo.os_id FROM varient v, build_os bo WHERE v.valias = 'web' AND bo.alias = 'ubuntu';
INSERT INTO packages (package, varient_id, os_id) SELECT package, v.varient_id, bo.os_id FROM packages p, varient v, build_os bo WHERE v.valias = 'lamp' AND bo.os_id = p.os_id AND p.package LIKE 'mysql-cli%' AND bo.alias = 'ubuntu';
