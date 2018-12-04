#!/usr/bin/python
# -*- coding: utf-8 -*-

#
# Copyright (C) 2018, Qijun Gu, Texas State University, qijun@txstate.edu
#
# Unless required by applicable law or agreed to in writing, software
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.

import struct

DEVICE = "/dev/ttyS1"
BAUDRATE = 57600
VEHICLE_FILE = "/tmp/vehicle.socket"
BUFSIZE = 16
PACKETSIZE = 256
PAYLOADSIZE = PACKETSIZE - 1

CMD_NUM_MAX = chr(127)
CMD = {
    'CMD_NONE' : chr(0),
    # from controller to sensor
    'CMD_GET' : 'G',                 #/* G/g : get data             */
    'CMD_EXIT' : 'X',                #/* X/x : exit                 */
    # from sensor to controller
    'CMD_DATA' : 'D',                #/* D/d : data                 */
    'CMD_WAIT' : 'W',                #/* W/w : waiting for data     */
    'CMD_UNKNOWN' : 'U',             #/* U/u : unknow command       */
    'CMD_ERROR' : CMD_NUM_MAX,        #/*     : error in received command */
}

class VRead():
    def __init__(self):
        self.ts = 0
        self.tus = 0
        self.gpsts = 0
        self.gpstus = 0
        self.altitude = float(0)
        self.latitude = float(0)
        self.longitude = float(0)
        self.pitch = float(0)
        self.roll = float(0)
        self.yaw = float(0)
        self.vx = float(0)
        self.vy = float(0)
        self.vz = float(0)
        self.battery = float(0)
        self.voltage = float(0)
        self.current = float(0)
        self.airspeed = float(0)
        self.groundspeed = float(0)
        self.mode = 'unknown'
        self.status = 'unknown'
    
    def set(self, time=None, gpstime=None, gps=None, att=None, vel=None, bat=None, asp=None, gsp=None, mod=None, sta=None):
        if time is None or gpstime is None or gps is None or att is None or vel is None or bat is None or asp is None or gsp is None or mod is None or sta is None:
            raise Exception('Not all data are provided.') 
        time = time or 0
        self.ts = int(time)
        self.tus = int((time-self.ts)*1000000)
        self.gpsts = int(gpstime)
        self.gpstus = int((gpstime-self.gpsts)*1000000)
        self.altitude = gps.alt or 0
        self.latitude = gps.lat or 0
        self.longitude = gps.lon or 0
        self.pitch = att.pitch or 0
        self.roll = att.roll or 0
        self.yaw = att.yaw or 0
        self.vx = vel[0] or 0
        self.vy = vel[1] or 0
        self.vz = vel[2] or 0
        self.battery = bat.level or 0
        self.voltage = bat.voltage or 0
        self.current = bat.current or 0
        self.airspeed = asp or 0
        self.groundspeed = gsp or 0
        self.mode = mod or 'unknown'
        self.status = sta or 'unknown'
    
    def pack(self):
        return struct.pack('<QIQIffffffffffffff', self.ts, self.tus, self.gpsts, self.gpstus, self.altitude, self.latitude, self.longitude, self.pitch, self.roll, self.yaw, self.vx, self.vy, self.vz, self.battery, self.voltage, self.current, self.airspeed, self.groundspeed)+self.mode+','+self.status
    
    def show(self, idx=None):
        print idx, self.ts, self.tus, self.gpsts, self.gpstus, self.altitude, self.latitude, self.longitude, self.pitch, self.roll, self.yaw, self.battery, self.voltage, self.current, self.airspeed, self.mode, self.status
    
vehicle = None
bufindex = 0
gpst = 0
vbuf = [VRead() for _ in xrange(BUFSIZE)]
