# -*- coding: utf-8 -*-

#
# Copyright (C) 2018, Qijun Gu, Texas State University, qijun@txstate.edu
#
# Unless required by applicable law or agreed to in writing, software
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.

from flask import Flask, request, redirect, flash, send_from_directory
from flask_bootstrap import Bootstrap
from flask.templating import render_template
from flask.helpers import url_for
import json, subprocess
from config import cfg, secret_key
from datafeed import datafeed
from download import downloader

app = Flask(__name__)
app.config.from_object(cfg)
app.secret_key = secret_key
Bootstrap(app)

@app.route('/')
def welcome():
    return redirect(url_for('data'))

@app.route('/data/')
def data():
    return render_template('data.html')

@app.route('/datafeed/', methods=['POST'])
def dfeed():
    return json.dumps(datafeed.gen())

@app.route('/download/', methods=['POST','GET'])
def download():
    args = downloader.getfiles()
    return render_template('download.html', args = args)

@app.route('/datafiles/<filename>', methods=['POST','GET'])
def datafiles(filename):
    return send_from_directory(cfg.datafolder, filename, as_attachment=True)

@app.route('/datafiles/remove/<filename>', methods=['POST','GET'])
def removedatafiles(filename):
    subprocess.call(['rm', '-f', cfg.datafolder+filename])
    return json.dumps(True)

@app.route('/datafiles/download/<filename>', methods=['POST','GET'])
def downloaddatafiles(filename):
    return send_from_directory(cfg.datafolder, filename, as_attachment=True)

### main ###
if __name__ == '__main__':
    app.run(host='0.0.0.0', threaded=True)
