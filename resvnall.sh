#!/bin/bash

if [ $EUID -ne 0 ]; then
  echo "Must run as root"
  exit 1
fi

pymodules=vehstatus dronedata dronesite
cmodules=opcreader no2reader

rm -rf $pymodules $cmodules

for m in $pymodules $cmodules; do
  svn svn+ssh://qijun@fuxi.cs.txstate.edu/home/repo/dronedata/$m $m
done

for m in $cmodules; do
  cd $m
  make
  cd ..
done

chown -R dronesite:dronesite .
