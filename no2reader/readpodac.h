/*
# Copyright (C) 2018, Qijun Gu, Texas State University, qijun@txstate.edu
#
# Unless required by applicable law or agreed to in writing, software
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.
# */

#ifndef _READPODAC_H_
#define _READPODAC_H_

#include <stdbool.h>
#include "hidapi.h"
#include "hidr.h"

bool readNo2toBuf(hid_device* hid, NO2Read* r);

// process the data from PODAC
/***
   data is the data array
   len is the size of data in bytes
   mode is BIN_CMD or CHAR_CMD
***/
void processDATA(COMMAND* cmd, NO2Read* r, hid_device* dev);
void ackData(hid_device* dev);

#endif // _READPODAC_H_


