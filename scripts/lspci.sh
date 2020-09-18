#!/bin/bash
#
# lspci test script
#
#
LSPCI=/usr/bin/lspci
TOUCH=/bin/touch
LSPCITMP=$XYMONTMP/lspci.$$
$TOUCH $LSPCITMP || echo "Cannot write to $XYMONTMP directory"

$LSPCI > $LSPCITMP
echo "" > $LSPCITMP

$XYMON $XYMSRV "status+8h $MACHINE.lspci green `date` lspci output

`cat $LSPCITMP`"

rm $LSPCITMP

