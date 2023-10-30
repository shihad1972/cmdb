#!/bin/sh
#
#  Build script to configure puppet agent on the newly built server
#
#  Takes the name or IP of the puppet server as argument
#
#
#  Copyright (C) 2023  Iain M Conochie <iain-AT-ailsatech.net>
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

PUPPET=$1

if [ -d /target ]; then
  TGT=/target
fi

DIR=${TGT}/etc/puppet

if [ ! -d ${DIR} ]; then
  echo "$DIR does not exist"
  exit 0
fi

if [ -n "${PUPPET}" ]; then
  echo "Configuring the puppet config file ${DIR}/puppet.conf"
  cat >${DIR}/puppet.conf <<STOP
[main]
ssldir = /var/lib/puppet/ssl
ca_server = ${PUPPET}

[master]
vardir = /var/lib/puppet
cadir  = /var/lib/puppet/ssl/ca

[agent]
server = ${PUPPET}

STOP
else
  echo "No puppet server passed to the script"
fi

cat >${TGT}/usr/share/firstboot/004-puppet-service.sh <<STOP
#!/bin/sh

if [ -x /bin/systemctl ]; then
  systemctl enable puppet
  systemctl start puppet
elif [ -x /usr/bin/service ]; then
  service puppet enable
  service puppet start
else
  echo "Cannot find service control"
  exit 0
fi
STOP

chmod 755 ${TGT}/usr/share/firstboot/004-puppet-service.sh
