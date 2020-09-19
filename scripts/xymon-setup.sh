#!/bin/sh
#
#  Build script to configure rsyslog to log remotely to a logging 
#  server
#
#  Takes the name or IP of the logging server as argument
#
#
#  Copyright (C) 2012 - 2013  Iain M Conochie <iain-AT-thargoid.co.uk>
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

if [ -z "$1" ]; then
  echo "No URL passed as argument"
  exit 1;
else
  url=$1;
fi

if [ -d /target ]; then
  TGT=/target/root
else
  TGT=/root
fi

DIR=`pwd`
WGET=/usr/bin/wget
SCRIPT="${TGT}/../usr/share/firstboot/003-hobbit-setup.sh"
if [ -z "$WGET" ]; then
  echo "Cannot find wget"
  exit 1
fi

if [ $DIR != $TGT ]; then
  cd $TGT
fi

for i in hobbit-patch.sh deploy-patch.sh lspci.sh deploy-lspci.sh; do
  $WGET ${url}${i}
  chmod 755 $i
done

if [ -f hobbit-patch.sh ] && [ -f deploy-patch.sh ]; then
  cat > $SCRIPT <<EOF
#!/bin/sh
#
# Setup hobbit-patch script
#

cd /root
./deploy-patch.sh > hobbit-patch.log 2>&1
./deploy-lspci.sh > lspci.log 2>&1
if [ -f /etc/init.d/hobbit-client ]; then
  CLIENT=hobbit-client
elif [ -f /etc/init.d/xymon-client ]; then
  CLIENT=xymon-client
fi
service \$CLIENT restart
EOF
  chmod 755 $SCRIPT
else
  echo "Unable to find files"
fi

