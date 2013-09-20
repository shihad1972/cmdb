#!/bin/sh
#
# Setup NTP on a kickstart install
#
# Copyright (C) 2013 Iain M Conochie <iain-AT-thargoid-DOT-co-DOT-uk>
#
# This copyrighted material is made available to anyone who wishes to use,
# modify, copy or redistribute is subject to the terms and conditions of the
# GNU General Public License V2
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation Inc.,
# 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
#

if [ -f /etc/ntp.conf ]; then
  mv /etc/ntp.conf /etc/ntp.conf-INSTALL
fi

if [ ! -d /etc/ntp ]; then
  mkdir /etc/ntp
fi

NTPSERVER=$1

if [ -n ${NTPSERVER} ]; then
  cat > /etc/ntp.conf <<EOF
server $NTPSERVER

EOF

  cat > /etc/ntp/step-tickers <<EOF
$NTPSERVER

EOF

  /sbin/chkconfig ntpd on
  /sbin/chkconfig ntpdate on
fi