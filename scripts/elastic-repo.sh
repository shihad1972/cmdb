#!/bin/bash
#  Copyright (C) 2020  Iain M Conochie <iain-AT-thargoid.co.uk>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.

#  You should have received a copy of the GNU General Public License along
#  with this program; if not, write to the Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#  elastic-repo.sh
#  Create the required configuration on a debian host for the elasticsearch
#  repository

# *** 
#     We cannot currently use this script within the cbc software, unless we
#     have a domain set aside specifically for the server and the clients are
#     in a separate domain.
# ***

if [ -z "$1" ]; then
  echo "No URL passed as argument"
  exit 1;
else
  STATUS=$1;
fi

if [ -d /target ]; then
  TGT=/target/root
else
  TGT=/root
fi

DIR=`pwd`
SCRIPT="${TGT}/../usr/share/firstboot/004-elastic-search.sh"
WGET=/usr/bin/wget
if [ -z "$WGET" ]; then
  echo "Cannot find wget"
  exit 1
fi

if [ $DIR != $TGT ]; then
  cd $TGT
fi

cat > $SCRIPT << EOF
#!/bin/sh
STATUS=$STATUS
$WGET -qO - https://artifacts.elastic.co/GPG-KEY-elasticsearch | apt-key add -
echo "deb https://artifacts.elastic.co/packages/7.x/apt stable main" > /etc/apt/sources.list.d/elastic-7.x.list
apt-get update
if [ "\$STATUS" = "server" ]; then
  apt-get install elasticsearch kibana
else if [ "\$STATUS" = "client" ]; then
  apt-get install filebeat
fi
EOF
chmod 755 $SCRIPT

