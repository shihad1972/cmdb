INSERT INTO server (vendor, make, model, uuid, cust_id, vm_server_id, name) VALUES ('KVM', 'Virtual Machine', 'x86_64', '84f0a8b9-4484-bbdf-8b21-4579031cc75a', 2, 2, 'newcastle');
INSERT INTO build_ip (ip, hostname, domainname, bd_id) VALUES (INET_ATON('192.168.50.3'), 'newcastle', 'epl.shihad.org', 3);
INSERT INTO build (mac_addr, net_inst_int, varient_id, ip_id, boot_id, os_id, server_id) VALUES ('52:54:00:ee:cf:e9', 'eth0', 4, 61, 2, 40, 36);
INSERT INTO hardware (detail, device, server_id, hard_type_id) VALUES ('52:54:00:ee:cf:e9', 'eth0', 36, 1);
INSERT INTO hardware (detail, device, server_id, hard_type_id) VALUES ('20 GB', 'vda', 36, 1);
UPDATE hardware SET hard_type_id = 2 WHERE hard_id = 74;
INSERT INTO hardware (detail, device, server_id, hard_type_id) VALUES ('1', '', 36, 3);
INSERT INTO hardware (detail, device, server_id, hard_type_id) VALUES ('512 MB', '', 36, 4);
INSERT INTO disk_dev (server_id, device, lvm) VALUES (36, '/dev/vda', 1);
INSERT INTO partitions (minimum, maximum, priority, mount_point, filesystem, server_id, logical_volume) VALUES  (64, 200, 512, 'swap', 'linux-swap', 36, 'swap');
INSERT INTO partitions (minimum, maximum, priority, mount_point, filesystem, server_id, logical_volume) VALUES  (500, 1000, 1000, '/', 'ext4', 36, 'root');
INSERT INTO partitions (minimum, maximum, priority, mount_point, filesystem, server_id, logical_volume) VALUES  (500, 1000, 700, '/var', 'ext4', 36, 'var');
INSERT INTO partitions (minimum, maximum, priority, mount_point, filesystem, server_id) VALUES  (128, 300, 500, '/boot', 'ext4', 36);
INSERT INTO disk_dev (server_id, device, lvm) VALUES (2, '/dev/vda', 0);
