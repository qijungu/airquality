# -*- coding: utf-8 -*-

#
# Copyright (C) 2018, Qijun Gu, Texas State University, qijun@txstate.edu
#
# Unless required by applicable law or agreed to in writing, software
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.

secret_key = u'c4b46d59dff2ef1131ec86349888ea1ee08f42b6fe3d8423c25185b5f2dec7b353'
DRONEDATA_FILE = "/tmp/dronedata.socket"
SockDrone = None
PACKETSIZE = 128

class Config(object):
    DEVELOPMENT = False
    DEBUG = False
    TESTING = False

class ProductionConfig(Config):
    DEVELOPMENT = False
    DEBUG = False
    datafolder = '/srv/dronedata/log/'

class DevelopmentConfig(Config):
    DEVELOPMENT = True
    DEBUG = True
    datafolder = '../log/'

class TestingConfig(Config):
    TESTING = True

#cfg = DevelopmentConfig()
cfg = ProductionConfig()
