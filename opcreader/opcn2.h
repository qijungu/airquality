/*
# Copyright (C) 2018, Qijun Gu, Texas State University, qijun@txstate.edu
#
# Unless required by applicable law or agreed to in writing, software
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.
# */

#ifndef _OPCN2_H_
#define _OPCN2_H_

#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

#include "usbissspi.h"

#define OPCINFOLEN 60
#define OPCSTATUSLEN 4
#define OPCSERIALLEN 60
#define OPCSFIRMVERLEN 2

#define OPCPMLEN 12
typedef union _PMData {
    struct {
        float PM1;        // PM1 in ug/m3
        float PM2;        // PM2.5 in ug/m3
        float PM10;       // PM10 in ug/m3
    } __attribute__((packed));
    uint8_t bytes[OPCPMLEN];
} PMData;

#define OPCHISTLEN 62
typedef union _HistData {
    struct {
        uint16_t Bin[16];
        uint8_t MToF[4];  // average amount of time that particles sized in the stated bin took to corss SPS's laser bean. The unit is us/3, i.e. a value of 10 represents 3.33us.
        float SFR;        // sample flow rate, ml/s.
        uint32_t TP;      // temperature and pressure alternating. Temperature is in C multiplied by 10. Pressure is in Pascals.
        float SP;         // sampling period, in second
        uint16_t sum;     // histogram bin sum
        float PM1;        // PM1 in ug/m3
        float PM2;        // PM2.5 in ug/m3
        float PM10;       // PM10 in ug/m3
    } __attribute__((packed));
    uint8_t bytes[OPCHISTLEN];
} HistData;

#define OPCDATALEN 74
typedef union _READData {
    struct {
        float PM1;        // PM1 in ug/m3
        float PM2;        // PM2.5 in ug/m3
        float PM10;       // PM10 in ug/m3
		uint16_t Bin[16]; // bin counts
		float MToF[4];    // average amount of time in us that particles sized in the stated bin took to corss SPS's laser bean
		float SFR;        // sample flow rate, ml/s.
		uint32_t TP;      // temperature and pressure alternating. Temperature is in C multiplied by 10. Pressure is in Pascals.
        float SP;         // sampling period, in second
		uint16_t sum;     // histogram bin sum
    } __attribute__((packed));
    uint8_t bytes[OPCDATALEN];
} READData;


typedef struct _PMRead {
    uint64_t ts;
    uint32_t tus;
    READData d;
} __attribute__((packed)) PMRead;

bool sendDataCmd(byte cmd, SPI* spi);
bool sendControlCmd(byte* cmd, int len, SPI* spi);
bool recvData(byte* data, int len, SPI* spi);

bool readIssInfo(SPI* spi);
bool setSpiMode(SPI* spi);
bool readInfo(SPI* spi);
bool readStatus(SPI* spi, uint8_t* fan, uint8_t* laser);
bool setFanLaser(SPI* spi, uint8_t mode);
bool readSerial(SPI* spi);
bool readFirmVer(SPI* spi);
bool readHist2Buf(SPI* spi, PMRead* r);
bool readPm2File(SPI* spi, FILE* fp);
bool readPm2Buf(SPI* spi, PMRead* r);

#endif // _OPCN2_H_
