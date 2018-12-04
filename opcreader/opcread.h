/*
# Copyright (C) 2018, Qijun Gu, Texas State University, qijun@txstate.edu
#
# Unless required by applicable law or agreed to in writing, software
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.
# */

#ifndef _OPCREAD_H_
#define _OPCREAD_H_

#include <time.h>
#include "opcn2.h"

#define BUFSIZE (16)
#define SOCKET_FILE "/tmp/opcreader.socket"
#define DEVICE "/dev/ttyACM0"
#define READPERIOD 1500000 // in us

typedef struct _PMBuf {
    SPI* spi;
    PMRead pmread[BUFSIZE];  // data buf
    int bufindex;    // index to write data to buf
    int count;
} __attribute__((packed)) PMBuf;

extern PMBuf pmbuf;

void* servThread(void* arg);
void* manageReadThread(void* arg);
void* readThread(void* arg);

void perrortime(char* err);

#define PACKETSIZE (128)
#define PAYLOADSIZE (PACKETSIZE-1)

#define CMD_NUM_MAX 127            /* max number of commands     */
typedef enum {
    CMD_NONE = 0,
    // from controller to sensor
    CMD_GET = 'G',                 /* G/g : get data             */
    CMD_EXIT = 'X',                /* X/x : exit                 */
    // from sensor to controller
    CMD_DATA = 'D',                /* D/d : data                 */
    CMD_WAIT = 'W',                /* W/w : waiting for data     */
    CMD_UNKNOWN = 'U',             /* U/u : unknow command       */
    CMD_ERROR = CMD_NUM_MAX        /*     : error in received command */
} CMD;

typedef struct _Packet {
    uint8_t cmd;
    uint8_t payload[PAYLOADSIZE];
} __attribute__((packed)) Packet;
#endif
