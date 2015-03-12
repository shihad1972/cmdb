#!/bin/bash
#
# lspci test script
#
#
LSPCI=/sbin/lspci
TOUCH=/bin/touch
LSPCITMP=$XYMONTMP/lspci.$$
$TOUCH $LSPCITMP || echo "Cannot write to $XYMONTMP directory"

$LSPCI > $LSPCITMP
echo "" > $LSPCITMP

$XYMON $XYMSRV "status $MACHINE.lspci green `date` lspci output

`cat $LSPCITMP`"

rm $LSPCITMP

