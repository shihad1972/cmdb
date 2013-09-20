#!/bin/sh
#
# Base late_command preseed script for debian
#
# (C) 2011 Iain Conochie
#
host=$1
url=$2
chroot /target
cd /root

if [ -z ${url} ]; then
  echo "No url was passed to this script"
  exit 0
else
  wget ${url}scripts/disable_install.php > disable.log 2>&1
fi

if [ -n "${host}" ]; then
  if wget ${url}hosts/${host}.sh ]; then
    chmod 755 ${host}.sh
    ./${host}.sh > host.log 2>&1
  fi
fi

