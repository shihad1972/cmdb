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

LOG=$1

if [ -d /target ]; then
  TGT=/target
fi

DIR=${TGT}/etc/rsyslog.d

if [ ! -d ${DIR} ]; then
  echo "$DIR does not exist"
  exit 0
fi

if [ -n "${LOG}" ]; then
  cat >${DIR}/remote.conf <<STOP
# ### begin forwarding rule ###
# Remote Logging To ${LOG}
#
# An on-disk queue is created for this action. If the remote host is
# down, messages are spooled to disk and sent when it is up again.
\$WorkDirectory /var/lib/rsyslog # where to place spool files
\$ActionQueueFileName fwdRule1 # unique name prefix for spool files
\$ActionQueueMaxDiskSpace 1g   # 1gb space limit (use as much as possible)
\$ActionQueueSaveOnShutdown on # save messages to disk on shutdown
\$ActionQueueType LinkedList   # run asynchronously
\$ActionResumeRetryCount -1    # infinite retries if host is down
*.* @@${LOG}:514
# ### end of the forwarding rule ###

STOP
else
  echo "No logging server passed to the script"
fi

