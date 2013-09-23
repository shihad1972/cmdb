#!/bin/sh
#
# Base late_command preseed script for debian
#
# (C) 2011 Iain Conochie
#
host=`cat config.txt | awk '{print $1}'`
url=`cat config.txt | awk '{print $2}'`

if [ -z ${url} ]; then
  echo "No url was passed to this script"
  exit 0
else
  wget ${url}scripts/disable_install.php > disable.log 2>&1
fi

if [ -n "${host}" ]; then
  if wget ${url}hosts/${host}.sh; then
    chmod 755 ${host}.sh
    ./${host}.sh > host.log 2>&1
  else
    echo "No host script for $host"
  fi
else
  echo "host not set"
fi

#rm config.txt
