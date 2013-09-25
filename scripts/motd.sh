#!/bin/sh
#
#  Edit motd to display legal banner
cat <<EOF>> /etc/motd

               THIS DEVICE IS PART OF A PRIVATE NETWORK

 *********************************************************************
 * Unauthorised access or use of this equipment is  prohibited and   *
 *    constitutes an offence under the Computer Misuse Act  1990.    *
 * This system is being monitored and logs will be used as evidence  *
 *     in court. If you are not authorised to use this system,       *
 *                  terminate this session now!                      *
 *********************************************************************
EOF
