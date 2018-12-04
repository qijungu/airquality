# -*- coding: utf-8 -*-
import sys, logging
sys.path.insert(0, '/srv/dronedata/dronesite')
logging.basicConfig(stream=sys.stderr)

from dronesite import app as application

