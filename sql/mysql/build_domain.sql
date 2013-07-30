-- MySQL dump 10.13  Distrib 5.1.66, for debian-linux-gnu (i486)
--
-- Host: mysql.shihad.org    Database: cmdbdev
-- ------------------------------------------------------
-- Server version	5.1.66-0+squeeze1-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `build_domain`
--

DROP TABLE IF EXISTS `build_domain`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `build_domain` (
  `bd_id` int(7) NOT NULL AUTO_INCREMENT,
  `start_ip` int(4) unsigned NOT NULL DEFAULT '0',
  `end_ip` int(4) unsigned NOT NULL DEFAULT '0',
  `netmask` int(4) unsigned NOT NULL DEFAULT '0',
  `gateway` int(4) unsigned NOT NULL DEFAULT '0',
  `ns` int(4) unsigned NOT NULL DEFAULT '0',
  `domain` varchar(150) NOT NULL DEFAULT 'no.domain',
  `ntp_server` varchar(64) NOT NULL DEFAULT 'shihad.org',
  `config_ntp` tinyint(4) NOT NULL DEFAULT '1',
  `ldap_ssl` tinyint(4) NOT NULL DEFAULT '1',
  `ldap_dn` varchar(96) NOT NULL DEFAULT 'dc=shihad,dc=org',
  `ldap_bind` varchar(128) NOT NULL DEFAULT 'cn=thargoid,dc=shihad,dc=org',
  `config_ldap` tinyint(4) NOT NULL DEFAULT '1',
  `log_server` varchar(64) NOT NULL DEFAULT 'logger01.shihad.org',
  `config_log` tinyint(4) NOT NULL DEFAULT '1',
  `smtp_server` varchar(64) NOT NULL DEFAULT 'weezer.epl.shihad.org',
  `config_email` tinyint(4) NOT NULL DEFAULT '1',
  `xymon_server` varchar(64) NOT NULL DEFAULT '192.168.1.50',
  `config_xymon` tinyint(4) NOT NULL DEFAULT '1',
  `ldap_server` varchar(64) NOT NULL DEFAULT 'ldap01.shihad.org',
  `email_server` varchar(64) NOT NULL DEFAULT 'mail01.scots.shihad.org',
  `xymon_config` tinyint(4) NOT NULL DEFAULT '1',
  `nfs_domain` varchar(79) NOT NULL DEFAULT 'shihad.org',
  PRIMARY KEY (`bd_id`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=latin1;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2013-01-28 22:46:38
