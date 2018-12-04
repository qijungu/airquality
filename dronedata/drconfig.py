# -*- coding: utf-8 -*-

#
# Copyright (C) 2018, Qijun Gu, Texas State University, qijun@txstate.edu
#
# Unless required by applicable law or agreed to in writing, software
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.

VEHICLE_PORT = '/dev/ttyS1'
#VEHICLE_PORT = '/dev/ttyACM0'
#VEHICLE_PORT = '/dev/ttyUSB0'
OPCREADER_PORT = '/dev/ttyACM0'
NO2READER_PORT = '/dev/hidraw0'

OPCREADER_FILE = "/tmp/opcreader.socket"
NO2READER_FILE = "/tmp/no2reader.socket"
VEHICLE_FILE = "/tmp/vehicle.socket"
DRONEDATA_FILE = "/tmp/dronedata.socket"
PACKETSIZE = 256
BUFSIZE = 16
READPERIOD = 1   # read every 2 seconds

SockVeh = None   # vehicle socket
SockOpc = None   # opc socket
SockNo2 = None   # no2 socket
connect = True

bufindex = 0
drbuf = ['' for _ in xrange(BUFSIZE)]

LOG = '/srv/dronedata/log/'     # directory to store csv data
datafd = None    
csvwriter = None
csvheader = ['Time','PM1','PM2.5','PM10','NO2WE','NO2AE','Mode','Status','Altitude','Latitude','Longitude','Battery','Voltage','Current','AirSpeed','GroundSpeed','Pitch','Roll','Yaw','Vx','Vy','Vz','TimeOpc','TimeVeh','Bin0','Bin1','Bin2','Bin3','Bin4','Bin5','Bin6','Bin7','Bin8','Bin9','Bin10','Bin11','Bin12','Bin13','Bin14','Bin15','MToF0','MToF1','MToF2','MToF3','FlowRate','TempPress','SampPeriod','Sum']
