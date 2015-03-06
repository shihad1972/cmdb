#!/bin/sh
#
#
#
#
while getopts "u:p:c:" opt; do
    case $opt in
        u  ) user=$OPTARG ;;
        p  ) password=$OPTARG ;;
        c  ) client=$OPTARG ;;
        \? ) echo 'Usage: $0 [ -u username ] [ -p password ] [ -c full path to client ]'
             exit 1
    esac
done


mv bb-mysql.pl /opt/hobbit/client/ext

if grep mysql /opt/hobbit/client/etc/clientlaunch.cfg; then

  echo "Not adding to /opt/hobbit/client/etc/clientlaunch.cfg. Already there"

else

    echo "
[mysql]
        ENVFILE \$HOBBITCLIENTHOME/etc/hobbitclient.cfg
        CMD \$HOBBITCLIENTHOME/ext/bb-mysql.pl
        LOGFILE \$HOBBITCLIENTHOME/logs/bb-mysql.log
        INTERVAL 5m

" >> /opt/hobbit/client/etc/clientlaunch.cfg

fi

if [ $password ]; then

  if [ $user ]; then
  
    if [ $client ]; then
  
      echo "mysqlclient=$client
auth= -u${user} -p${password}" >> /opt/hobbit/client/etc/bb-mysql.cfg

    else
    
      echo "mysqlclient=/usr/bin/mysql
auth= -u${user} -p${password}" >> /opt/hobbit/client/etc/bb-mysql.cfg

    fi

  else
  
    if [ $client ]; then

      echo "mysqlclient=$client
auth= -uroot -p${password}" >> /opt/hobbit/client/etc/bb-mysql.cfg

    else
    
      echo "mysqlclient=/usr/bin/mysql
auth= -uroot -p${password}" >> /opt/hobbit/client/etc/bb-mysql.cfg

    fi

  fi
  
else

  if [ $user ]; then
  
    if [ $client ]; then
  
      echo "mysqlclient=$client
auth= -u${user}" >> /opt/hobbit/client/etc/bb-mysql.cfg

    else
    
      echo "mysqlclient=/usr/bin/mysql
auth= -u${user}" >> /opt/hobbit/client/etc/bb-mysql.cfg

    fi

  else
  
    if [ $client ]; then

      echo "mysqlclient=$client
auth= -uroot" >> /opt/hobbit/client/etc/bb-mysql.cfg

    else
    
      echo "mysqlclient=/usr/bin/mysql
auth= -uroot" >> /opt/hobbit/client/etc/bb-mysql.cfg

    fi
    
  fi
    
fi
