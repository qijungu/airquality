#!/bin/bash
if [ "$LOGNAME" != dronesite ] ; then
  echo "Must run as dronesite"
  exit 1
fi
#if ! $(ps -u dronesite | grep -i vehicledata.py) ; then
  nohup /srv/dronedata/vehstatus/vehicledata.py --connect=/dev/ttyS1 1>/dev/null 2>/var/log/dronesite/vehicledata.log &
#fi
#if ! $(ps -u dronesite | grep -i opcread) ; then
  nohup /srv/dronedata/opcreader/opcread /dev/ttyACM0 1>/dev/null 2>/var/log/dronesite/opcread.log &
#fi
#if ! $(ps -u dronesite | grep -i hidr) ; then
  nohup /srv/dronedata/no2reader/hidr 1>/dev/null 2>/var/log/dronesite/no2read.log &
#fi
sleep 30
#if ! $(ps -u dronesite | grep -i dronedata.py) ; then
  nohup /srv/dronedata/dronedata/dronedata.py 1>/dev/null 2>/var/log/dronesite/dronedata.log &
#fi
