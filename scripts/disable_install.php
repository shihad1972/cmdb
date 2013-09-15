<?php

if (isset($_SERVER['REMOTE_ADDR']))
    $ip = $_SERVER['REMOTE_ADDR'];
else
    $ip="192.168.1.203";

$long=ip2long($ip);
$hex=dechex($long);
$hex=strtoupper($hex);
print "$hex\n";
$origin = "/srv/tftp/pxelinux.cfg/$hex";
$destination = "/srv/tftp/pxelinux.cfg/$hex.disabled";
passthru("mv $origin $destination");
?>
