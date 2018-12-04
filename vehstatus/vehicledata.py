#!/usr/bin/python
# -*- coding: utf-8 -*-

#
# Copyright (C) 2018, Qijun Gu, Texas State University, qijun@txstate.edu
#
# Unless required by applicable law or agreed to in writing, software
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.

from dronekit import connect
import argparse, threading, SocketServer, time, os, sys
import vconfig

def run():
    try:
        os.unlink(vconfig.VEHICLE_FILE)
    except:
        pass
    
    parser = argparse.ArgumentParser(description='Print out vehicle state information. Connects to SITL on local PC by default.')
    parser.add_argument('--connect', help="vehicle connection target string. If not specified, SITL automatically started and used.")
    parser.add_argument('--baudrate', help="vehicle serial data baudrate. Default is 57600.")
    args = parser.parse_args()
    
    connection = args.connect
    baudrate = args.baudrate
    if not connection:
        connection = vconfig.DEVICE
    if not baudrate:
        baudrate = vconfig.BAUDRATE
    else:
        baudrate = int(baudrate)
    
    sys.stderr.write(time.ctime() + " : Connecting to vehicle on: %s\n" % connection)
    vconfig.vehicle = connect(connection, wait_ready=False, baud=baudrate)
    #print time.time()
    vconfig.vehicle.wait_ready(True, timeout=300)
    #print time.time()
    @vconfig.vehicle.on_message('SYSTEM_TIME')
    def listener(self, name, message):
        vconfig.gpst = message.time_unix_usec/1000000.0
    
    rt = threading.Thread(target=readthread)
    rt.daemon = True
    rt.start()
    
    sv = SocketServer.ThreadingUnixStreamServer(vconfig.VEHICLE_FILE, servthread)
    sv.serve_forever()

def readthread():
    bidx = 0
    while True:
        t = time.time()
        gps = vconfig.vehicle.location.global_frame
        attitude = vconfig.vehicle.attitude
        velocity = vconfig.vehicle.velocity
        battery = vconfig.vehicle.battery
        airspeed = vconfig.vehicle.airspeed
        groundspeed = vconfig.vehicle.groundspeed
        mode = vconfig.vehicle.mode.name
        status = vconfig.vehicle.system_status.state
        bidx = vconfig.bufindex
        vconfig.vbuf[bidx].set(t, vconfig.gpst, gps, attitude, velocity, battery, airspeed, groundspeed, mode, status)
        vconfig.bufindex = (bidx+1) % vconfig.BUFSIZE
        #vconfig.vbuf[bidx].show(bidx)
        time.sleep(0.5)

class servthread(SocketServer.BaseRequestHandler):
    def handle(self):
        while True:
            data = self.request.recv(vconfig.PACKETSIZE)
            if len(data) == 0:
                break
            cmd = data[0]
            if cmd == vconfig.CMD['CMD_GET']:
                pkts = bytearray()
                pkts.append(vconfig.CMD['CMD_DATA'])
                bidx = (vconfig.bufindex - 1 + vconfig.BUFSIZE) % vconfig.BUFSIZE
                payload = vconfig.vbuf[bidx].pack()
                for pb in payload:
                    pkts.append(pb)
                self.request.sendall(pkts)
            elif cmd == vconfig.CMD['CMD_EXIT']:
                break
            else:
                pkts = bytearray()
                pkts.append(vconfig.CMD['CMD_UNKNOWN'])
                self.request.sendall(pkts)

if __name__ == "__main__":
    run()
