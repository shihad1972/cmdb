#!/bin/sh
#
#  Update the /etc/nsswitch.conf of a new installation to use ldap
#
#  (C) Iain M Conochie <iain-AT-thargoid-DOT-co-DOT-uk> 2013
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
  TGT=/target/root
else
  TGT=/root
fi

DIR=`pwd`

if [ $DIR != $TGT ]; then
  cd $TGT
fi

mv ../etc/nsswitch.conf ../etc/nsswitch.BAK
cat ../etc/nsswitch.BAK | sed -e s/compat/files\ ldap/g > ../etc/nsswitch.conf

