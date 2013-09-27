#!/bin/sh
#
#  Script to generate firstboot scripts and run them on firstboot
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

if [ -d /target ]; then
   TGT=/target
fi
mkdir ${TGT}/usr/share/firstboot

cat >${TGT}/etc/rc.local <<EOF
#!/bin/sh -e

for i in /usr/share/firstboot/*; do

    if [ -x \$i ]; then

        \$i >>/usr/share/firstboot/scripts.log 2>&1
    fi

done

chmod 644 /usr/share/firstboot/*.sh

exit 0
EOF

chmod 755 ${TGT}/etc/rc.local
