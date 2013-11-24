#!/bin/bash

. /etc/rc.conf
. /etc/rc.d/functions

case "$1" in
  start)
    stat_busy "Starting CLD Daemon"
    [ ! -d /var/run/cldd ] && install -d -g 45 -o 45 /var/run/cldd
    /usr/bin/cldd /etc/cldd.conf &> /dev/null
    if [ $? -gt 0 ]; then
      stat_fail
    else
      add_daemon cldd
      stat_done
    fi
    ;;
  stop)
    stat_busy "Stopping CLD Daemon"
    /usr/bin/cldd --kill /etc/cldd.conf &> /dev/null
    if [ $? -gt 0 ]; then
      stat_fail
    else
      rm_daemon cldd
      stat_done
    fi
    ;;
  restart)
    $0 stop
    sleep 1
    $0 start
    ;;
  *)
    echo "usage: $0 {start|stop|restart}"
esac
exit 0
