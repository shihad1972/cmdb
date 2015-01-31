#!/bin/sh
#
#
# (C) 2009 Iain Conochie
#
#
# Patch report script for hobbit
#
# Supported OS's:
#
# Redhat (4.x, 5.x)
#
# Centos (4.x, 5.x)
#
# Ubuntu and Debian and any other apt-get based system should also work
#
# $Id: hobbit-patch.sh,v 1.11 2010/05/18 14:42:58 iain Exp iain $
#
STATUS=green
APTGET=/usr/bin/apt-get
UP2DATE=/usr/sbin/up2date
YUM=/usr/bin/yum
SECUP=${BBTMP}/security.$$
BUGUP=${BBTMP}/updates.$$
DEBUP=${BBTMP}/debupdates.$$
SUDO=/usr/bin/sudo

uptodate()
{
    ARCH=`uname -i`
    $SUDO $UP2DATE -l > $BUGUP
    if cat $BUGUP | grep -v Obsoletes |grep -v mirror | grep -v dell| grep $ARCH || grep noarch $BUGUP; then
        STATUS=yellow
    fi

    echo "Up2date Status: " >> ${BUGUP}.new
    echo " " >> ${BUGUP}.new
    cat ${BUGUP} >> ${BUGUP}.new
    if [ $STATUS = green ]; then
        echo "No updates available" >> ${BUGUP}.new
    fi
    $BB $BBDISP "status+7h $MACHINE.patches $STATUS `date` Patches output

`cat ${BUGUP}.new`"

    rm ${BUGUP}
    rm ${BUGUP}.new

}
yumold()
{
    CHECKUP="check-update"
    $YUM $CHECKUP -d 0 2>/dev/null > $BUGUP

    if [ -s $BUGUP ]; then
        STATUS=yellow
    fi

    echo "Yum Updates Available" >> ${BUGUP}.new
    echo " " >> ${BUGUP}.new
    cat ${BUGUP} >> ${BUGUP}.new
    if [ $STATUS = green ]; then
        echo "No updates available" >> ${BUGUP}.new
    fi

    $BB $BBDISP "status+7h $MACHINE.patches $STATUS `date` Patches output

`cat ${BUGUP}.new`"

    rm ${BUGUP}
    rm ${BUGUP}.new

}

yum()
{

    LISTSEC="list-sec security"
    CHKSEC="--security"
    CHECKUP="check-update"

    if grep CentOS /etc/redhat-release; then
        $YUM $CHKSEC $CHECKUP | grep CESA>> $SECUP
    else
        $SUDO $YUM $LISTSEC | grep RHSA>> $SECUP
    fi

    if grep CentOS /etc/redhat-release; then
        $YUM $CHECKUP -q >> $BUGUP
    else
        $SUDO $YUM $CHECKUP -q >> $BUGUP
    fi

    if [ -s $BUGUP ]; then
        STATUS=yellow
    fi

    if [ -s $SECUP ]; then
        STATUS=red
    fi

    echo "Yum Security Updates" >> ${SECUP}.new
    echo " " >> ${SECUP}.new
    cat $SECUP >> ${SECUP}.new
    echo "Yum Bug Fixes" >> ${BUGUP}.new
    echo " " >> ${BUGUP}.new
    cat $BUGUP >> ${BUGUP}.new
    if [ $STATUS = green ]; then
        echo "No updates available" >> ${BUGUP}.new
    fi

    $BB $BBDISP "status+7h $MACHINE.patches $STATUS `date` Patches output

`cat ${SECUP}.new; echo; cat ${BUGUP}.new`"

    rm ${SECUP}
    rm ${SECUP}.new
    rm ${BUGUP}
    rm ${BUGUP}.new

}

aptget()
{

    SUDO=/usr/bin/sudo

    if [ ! -x $SUDO ]; then
        STATUS=red
        $BB $BBDISP "status+7h $MACHINE.patches $STATS `date` Patches output

No Sudo installed! We need sudo to run"

        exit 0
    fi
    $SUDO $APTGET update > /dev/null 2>&1
    $SUDO $APTGET upgrade -s -q=2 |grep Inst > $DEBUP
    grep Debian-Security $DEBUP > $SECUP
    grep -v Debian-Security $DEBUP > $BUGUP

    if [ -s $BUGUP ]; then
        STATUS=yellow
    fi

    if [ -s $SECUP ]; then
        STATUS=red
    fi

    echo "Apt-get Security Updates" >> ${SECUP}.new
    echo " " >> ${SECUP}.new
    cat $SECUP >> ${SECUP}.new

    if [ $STATUS != red ]; then
        echo "No Security updates" >> ${SECUP}.new
    fi

    echo "Apt-get Bug Fixes" >> ${BUGUP}.new
    echo " " >> ${BUGUP}.new
    cat $BUGUP >> ${BUGUP}.new

    if [ $STATUS = green ]; then
        echo "No updates available" >> ${BUGUP}.new
    fi

    $BB $BBDISP "status+7h $MACHINE.patches $STATUS `date` Patches output

`cat ${SECUP}.new; echo; cat ${BUGUP}.new`"

    rm ${BUGUP}
    rm ${BUGUP}.new
    rm ${SECUP}
    rm ${SECUP}.new
    rm $DEBUP

}

if [ -x $APTGET ]; then
    aptget
elif [ -x $YUM ]; then
    if $YUM -q list yum ; then
        yum
    else
        if grep CentOS /etc/redhat-release; then
            yumold
        else
            uptodate
        fi
    fi
else
    uptodate
fi

