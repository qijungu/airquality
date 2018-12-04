# -*- coding: utf-8 -*-

#
# Copyright (C) 2018, Qijun Gu, Texas State University, qijun@txstate.edu
#
# Unless required by applicable law or agreed to in writing, software
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.

import socket, os, struct, time, threading
import config

class Datafeed(object):
    
    def __init__(self):
        self.svdrone = None
        self.connect = True
        rt = threading.Thread(target=self.connectDrone)
        rt.daemon = True
        rt.start()
    
    def __del__(self):
        self.connect = False
        if self.svdrone is not None:
            self.svdrone.close()
        self.svdrone = None
    
    def connectDrone(self):
        while self.connect:
            if self.svdrone is None:
                while not os.path.exists(config.DRONEDATA_FILE):
                    print "Wating for drone ..."
                    time.sleep(3)
                try:
                    self.svdrone = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM, 0)
                    self.svdrone.connect(config.DRONEDATA_FILE)
                    print "Connected to drone"
                except:
                    self.svdrone = None
                    print "Connection error to drone ..."
                    time.sleep(3)
            else:
                time.sleep(3)
    
    def gen(self):
        
        try:
            if self.svdrone is not None:
                self.svdrone.sendall("G")
                data = self.svdrone.recv(config.PACKETSIZE)
                ts, tus, pm1, pm2, pm10, no2we, no2ae, alt, lat, lon, bat, asp, pit, rol, yaw = struct.unpack('<QIfffffffffffff', data[:64])
                mod,sta = data[64:].split(',')
                status = True
            else:
                ts, tus, pm1, pm2, pm10, no2we, no2ae, alt, lat, lon, bat, asp, pit, rol, yaw = 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
                mod,sta = 'unknown','unknown'
                status = False
        except:
            ts, tus, pm1, pm2, pm10, no2we, no2ae, alt, lat, lon, bat, asp, pit, rol, yaw = 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
            mod,sta = 'unknown','unknown'
            status = False
            self.svdrone = None
        
        return [status, {
            'ts': ts,
            'tus': tus,
            'pm1': pm1,
            'pm2': pm2,
            'pm10': pm10,
            'no2we': no2we,
            'no2ae': no2ae,
            'mode': mod,
            'status': sta,
            'altitude': alt,
            'latitude': lat,
            'longitude': lon,
            'battery': bat,
            'airspeed': asp,
            'pitch': pit,
            'roll': rol,
            'yaw': yaw,
            }]

datafeed = Datafeed()
