# -*- coding: utf-8 -*-

#
# Copyright (C) 2018, Qijun Gu, Texas State University, qijun@txstate.edu
#
# Unless required by applicable law or agreed to in writing, software
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.

import os, subprocess, sys, threading
from config import cfg

class Download(object):
    
    def __init__(self):
        return
    
    def getfiles(self):
        args = {}
        try:
            dfs = sorted(os.listdir(cfg.datafolder), reverse=True)
            args['datafiles'] = []
            for df in dfs:
				sz = os.path.getsize(cfg.datafolder+df)
				args['datafiles'] += [{'name':df, 'size':sz}]
        except Exception as e:
            sys.stderr.write('Download.getfiles : ' + str(e))
        return args
    
downloader = Download()
