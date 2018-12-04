#!/usr/bin/python
# -*- coding: utf-8 -*-

#
# Copyright (C) 2018, Qijun Gu, Texas State University, qijun@txstate.edu
#
# Unless required by applicable law or agreed to in writing, software
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.

import sys, socket, struct, time, subprocess, os, threading, SocketServer, csv
import drconfig

def run():

    sys.stderr.write(time.ctime() + " : Dronedata started\n")

    if not os.path.exists(drconfig.VEHICLE_PORT):
        sys.stderr.write(time.ctime() + " : No vehicle\n")
        #exit()
    
    if not os.path.exists(drconfig.OPCREADER_PORT):
        sys.stderr.write(time.ctime() + " : No opc\n")
        #exit()
    
    if not os.path.exists(drconfig.NO2READER_PORT):
        sys.stderr.write(time.ctime() + " : No no2\n")
        #exit()

    try:
        os.unlink(drconfig.DRONEDATA_FILE)
    except:
        pass
    
    #subprocess.Popen(['/usr/bin/python', 'vehstatus/vehicledata.py', '--connect', drconfig.VEHICLE_PORT])
    #subprocess.Popen(['opcreader/opcread', drconfig.OPCREADER_PORT])
    
    time.sleep(3)
    
    rt = threading.Thread(target=connectNo2)
    rt.daemon = True
    rt.start()

    rt = threading.Thread(target=connectOpc)
    rt.daemon = True
    rt.start()
    
    rt = threading.Thread(target=connectVeh)
    rt.daemon = True
    rt.start()

    rt = threading.Thread(target=readthread)
    rt.daemon = True
    rt.start()

    sv = SocketServer.ThreadingUnixStreamServer(drconfig.DRONEDATA_FILE, servthread)
    sv.serve_forever()
 
    drconfig.SockVeh.send("X")
    drconfig.SockVeh.close()
    drconfig.SockOpc.send("X")
    drconfig.SockOpc.close()
    drconfig.SockNo2.send("X")
    drconfig.SockNo2.close()

def connectNo2():
    while drconfig.connect:
        if drconfig.SockNo2 is None:
            while not os.path.exists(drconfig.NO2READER_FILE):
                sys.stderr.write(time.ctime() + " : Wating for no2 sensor ...\n")
                time.sleep(3)
            try:
                drconfig.SockNo2 = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET, 0)
                drconfig.SockNo2.connect(drconfig.NO2READER_FILE)
                sys.stderr.write(time.ctime() + " : Connected to no2 sensor\n")
            except:
                drconfig.SockNo2 = None
                sys.stderr.write(time.ctime() + " : Connection error to no2 sensor ...\n")
                time.sleep(3)
        else:
            time.sleep(3)

def connectOpc():
    while drconfig.connect:
        if drconfig.SockOpc is None:
            while not os.path.exists(drconfig.OPCREADER_FILE):
                sys.stderr.write(time.ctime() + " : Wating for opc sensor ...\n")
                time.sleep(3)
            try:
                drconfig.SockOpc = socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET, 0)
                drconfig.SockOpc.connect(drconfig.OPCREADER_FILE)
                sys.stderr.write(time.ctime() + " : Connected to opc sensor\n")
            except:
                drconfig.SockOpc = None
                sys.stderr.write(time.ctime() + " : Connection error to opc sensor ...\n")
                time.sleep(3)
        else:
            time.sleep(3)

def connectVeh():
    while drconfig.connect:
        if drconfig.SockVeh is None:
            while not os.path.exists(drconfig.VEHICLE_FILE):
                sys.stderr.write(time.ctime() + " : Wating for drone controller ...\n")
                time.sleep(3)
            try:
                drconfig.SockVeh = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM, 0)
                drconfig.SockVeh.connect(drconfig.VEHICLE_FILE)
                sys.stderr.write(time.ctime() + " : Connected to drone controller\n")
            except:
                drconfig.SocketVeh = None
                sys.stderr.write(time.ctime() + " : Connection error to drone controller ...\n")
                time.sleep(3)
        else:
            time.sleep(3)

def readthread():
    bidx = 0
    while True:
        try:
            drconfig.SockVeh.sendall("G")
            dveh = drconfig.SockVeh.recv(drconfig.PACKETSIZE)
        except:
            drconfig.SockVeh = None
            dveh = "X"
        try:
            drconfig.SockOpc.sendall("G")
            dopc = drconfig.SockOpc.recv(drconfig.PACKETSIZE)
        except:
            drconfig.SockOpc = None
            dopc = "X"
            
        try:
            drconfig.SockNo2.sendall("G")
            dno2 = drconfig.SockNo2.recv(drconfig.PACKETSIZE)
        except:
            drconfig.SockNo2 = None
            dno2 = "X"

        t = time.time()
        ts = int(t)
        tus = int((t-ts)*1000000)
        
        if len(dveh) > 0 and dveh[0] == "D":
            cmd, tsveh, tusveh, tsgps, tusgps, alt, lat, lon, pit, rol, yaw, vx, vy, vz, bat, volt, curr, asp, gsp = struct.unpack('<cQIQIffffffffffffff', dveh[:81])
            mod, sta = dveh[81:].split(',')
        else:
            cmd, tsveh, tusveh, tsgps, tusgps, alt, lat, lon, pit, rol, yaw, vx, vy, vz, bat, volt, curr, asp, gsp = '',-1,-1,0,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
            mod, sta = 'unknown', 'unknown'
        tveh = tsveh+tusveh/1000000.0
        if abs(t-tveh) > 1.0:
            alt, lat, lon, pit, rol, yaw, vx, vy, vz, bat, volt, curr, asp, gsp = -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
            mod, sta = 'unknown', 'unknown'
        #print cmd, tsveh, tusveh, alt, lat, lon, pit, rol, yaw, bat
        if dopc[0] == "D":
            cmd, tsopc, tusopc, pm1, pm2, pm10, bin0, bin1, bin2, bin3, bin4, bin5, bin6, bin7, bin8, bin9, bin10, bin11, bin12, bin13, bin14, bin15, mtof0, mtof1, mtof2, mtof3, sfr, tp, sp, sm = struct.unpack('<cQIfffHHHHHHHHHHHHHHHHfffffIfH', dopc[:87])
        else:
            cmd, tsopc, tusopc, pm1, pm2, pm10, bin0, bin1, bin2, bin3, bin4, bin5, bin6, bin7, bin8, bin9, bin10, bin11, bin12, bin13, bin14, bin15, mtof0, mtof1, mtof2, mtof3, sfr, tp, sp, sm = '',-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
        topc = tsopc+tusopc/1000000.0
        if abs(t-topc) > 1.0:
            pm1, pm2, pm10, bin0, bin1, bin2, bin3, bin4, bin5, bin6, bin7, bin8, bin9, bin10, bin11, bin12, bin13, bin14, bin15, mtof0, mtof1, mtof2, mtof3, sfr, tp, sp, sm = -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
        #print cmd, tsopc, tusopc, pm1, pm2, pm10
        
        if dno2[0] == "D":
            cmd, tsno2, tusno2, no2we, no2ae = struct.unpack('<cQIff', dno2[:21])
        else:
            cmd, tsno2, tusno2, no2we, no2ae = '',-1,-1,-1,-1,
        tno2 = tsno2+tusno2/1000000.0
        if abs(t-tno2) > 1.0:
            no2we, no2ae = -1,-1,

        data = [tsgps, tusgps, pm1, pm2, pm10, no2we, no2ae, mod, sta, alt, lat, lon, bat, volt, curr, asp, gsp, pit, rol, yaw, vx, vy, vz, tsopc, tusopc, tsveh, tusveh, bin0, bin1, bin2, bin3, bin4, bin5, bin6, bin7, bin8, bin9, bin10, bin11, bin12, bin13, bin14, bin15, mtof0, mtof1, mtof2, mtof3, sfr, tp, sp, sm]
        if tsgps > 0:
            tgps = tsgps + tusgps/1000000.0
        else:
            tgps = 0
        print data
        
        # save data to csv when drone is active
        #if sta == 'ACTIVE':
        if True:
            if drconfig.datafd is None : # create csv file since it is not created yet
                if tsgps>0 :
                    datafile = drconfig.LOG + time.strftime('%Y-%m-%d-%H:%M:%S.'+'%03d'%(tusgps/1000),time.localtime(tsgps)) + '.csv'
                    drconfig.datafd = open(datafile,'wb')
                    drconfig.csvwriter = csv.writer(drconfig.datafd, delimiter=',', quotechar='"', quoting=True)
                    drconfig.csvwriter.writerow(drconfig.csvheader)
            else:
                csvdata = [tgps, pm1, pm2, pm10, no2we, no2ae, mod, sta, alt, lat, lon, bat, volt, curr, asp, gsp, pit, rol, yaw, vx, vy, vz, topc, tveh, bin0, bin1, bin2, bin3, bin4, bin5, bin6, bin7, bin8, bin9, bin10, bin11, bin12, bin13, bin14, bin15, mtof0, mtof1, mtof2, mtof3, sfr, tp, sp, sm]
                drconfig.csvwriter.writerow(csvdata)
                drconfig.datafd.flush()
                os.fsync(drconfig.datafd.fileno())
        else:
            if drconfig.datafd is not None:
                drconfig.datafd.close()
                drconfig.datafd = None
                drconfig.csvwriter = None
        
        bidx = drconfig.bufindex
        drconfig.drbuf[bidx] = struct.pack('<QIfffffffffffff', tsgps, tusgps, pm1, pm2, pm10, no2we, no2ae, alt, lat, lon, bat, asp, pit, rol, yaw)+mod+','+sta
        drconfig.bufindex = (bidx+1) % drconfig.BUFSIZE
        
        time.sleep(drconfig.READPERIOD)
    
class servthread(SocketServer.BaseRequestHandler):
    def handle(self):
        while True:
            _ = self.request.recv(drconfig.PACKETSIZE)
            bidx = (drconfig.bufindex - 1 + drconfig.BUFSIZE) % drconfig.BUFSIZE
            self.request.sendall(drconfig.drbuf[bidx])

if __name__ == "__main__":
    run()

