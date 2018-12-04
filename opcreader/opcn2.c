/*
# Copyright (C) 2018, Qijun Gu, Texas State University, qijun@txstate.edu
#
# Unless required by applicable law or agreed to in writing, software
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.
# */

#include <stdio.h>
#include <time.h>

#include "usbissspi.h"
#include "opcn2.h"

bool sendDataCmd(byte cmd, SPI* spi) {
    spi->obuf[0] = 0x61;  // SPI command
    spi->obuf[1] = cmd;
    return outSPI(spi, 2);
}

bool sendControlCmd(byte* cmd, int len, SPI* spi) {
    spi->obuf[0] = 0x5A;  // SPI command
    for (int i=0; i<len; i++) spi->obuf[i+1] = cmd[i];
    return outSPI(spi, len+1);
}

/*
 * read len bytes to data (not raw data)
 * in the raw data, the first byte should be 0xFF ack, and is removed from data when return.
 */
bool recvData(byte* data, int len, SPI* spi) {
    if (SPIBUFSIZE < len+1) {
        perror("recvData SPIBUFSIZE < len+1");
        return false;
    }
    byte buf[SPIBUFSIZE];
    int ilen = inSPI(buf, SPIBUFSIZE, spi);
    if (ilen < 0) {
        perror("recvData ilen<0");
        return false;
    }
    if (ilen != len+1) {
        perror("recvData ilen!=len+1");
        return false;
    }
    if (buf[0] != 0xFF) {
        perror("recvData[0] is not 0xFF");
        return false;
    }
    for (int i=0; i<len; i++) data[i] = buf[i+1];
    return true;    
}

/*
 * read len bytes to data as raw data.
 */
bool recvRawData(byte* data, int len, SPI* spi) {
    if (SPIBUFSIZE < len) {
        perror("recvRawData SPIBUFSIZE < len");
        return false;
    }
    byte buf[SPIBUFSIZE];
    int ilen = inSPI(buf, SPIBUFSIZE, spi);
    if (ilen < 0) {
        perror("recvRawData ilen<0");
        return false;
    }
    if (ilen != len) {
        perror("recvRawData ilen!=len");
        return false;
    }
    for (int i=0; i<len; i++) data[i] = buf[i];
    return true;    
}

bool readIssInfo(SPI* spi) {
    byte cmd[] = {0x01};
    if (!sendControlCmd(cmd, 1, spi)) return false;

    byte data[3];
    if (!recvRawData(data, 3, spi)) return false;

    printf("USB-ISS Module ID: %u \n", data[0]);
    printf("USB-ISS Software v: %u \n", data[1]);
    printf("USB-ISS Software r: %u \n", data[2]);
	delayus(10000);
    return true;
}

bool setSpiMode(SPI* spi) {
    byte cmd[] = {0x02, 0x92, 11};
    if (!sendControlCmd(cmd, 3, spi)) return false;

    byte data[1];
    if (!recvData(data, 1, spi)) return false;

    printf("SPI set\n");
	delayus(10000);
    return true;
}

bool readInfo(SPI* spi) {
    if (!sendDataCmd(0x3F, spi)) return false;
    if (!delayus(10000)) return false;
    byte data[1];
    if (!recvData(data, 1, spi)) return false;
    if (data[0] != 0xF3) {
        perror("readInfo not 0xF3");
        return false;
    }
    
    byte info[OPCINFOLEN];
    for (int i=0; i<OPCINFOLEN; i++) {
        sendDataCmd(0x00, spi);
        recvData(info+i, 1, spi);
    }
    printf("OPCN2 Info : ");
    for (int i=0; i<OPCINFOLEN; i++) printf("%c", info[i]);
    printf("\n");
        
	delayus(10000);
    return true;
}

bool readStatus(SPI* spi, uint8_t* fan, uint8_t* laser) {
    if (!sendDataCmd(0x13, spi)) return false;
    if (!delayus(10000)) return false;
    byte data[1];
    if (!recvData(data, 1, spi)) return false;
    if (data[0] != 0xF3) {
        perror("readStatus not 0xF3");
        return false;
    }
    
    byte status[OPCSTATUSLEN];
    for (int i=0; i<OPCSTATUSLEN; i++) {
        sendDataCmd(0x00, spi);
        recvData(status+i, 1, spi);
    }
    printf("OPCN2 Status : ");
    for (int i=0; i<OPCSTATUSLEN; i++) printf("%02X", status[i]);
    printf(" : Fan %s, Laser %s, FanDAC %02X, LaserDAC %02X\n", status[0]?"On":"Off", status[1]?"On":"Off", status[2], status[3]);
    *fan = status[0];
    *laser = status[1];

	delayus(10000);
    return true;
}

// mode=1 is on, mode=0 is off
bool setFanLaser(SPI* spi, uint8_t mode) {
	if (!sendDataCmd(0x03, spi)) return false;
	if (!delayus(10000)) return false;
	byte data[1];
	if (!recvData(data, 1, spi)) return false;
	if (data[0] != 0xF3) {
	    perror("readStatus not 0xF3");
	    return false;
	}

	if (!sendDataCmd(mode?0x00:0x01, spi)) return false;
	if (!delayus(10000)) return false;
	if (!recvData(data, 1, spi)) return false;
	if (data[0] != 0x03) {
	    perror("readStatus not 0x03");
	    return false;
	}

	delayus(10000);
	return true;
}

bool readSerial(SPI* spi) {
    if (!sendDataCmd(0x10, spi)) return false;
    if (!delayus(10000)) return false;
    byte data[1];
    if (!recvData(data, 1, spi)) return false;
    if (data[0] != 0xF3) {
        perror("readSerial not 0xF3");
        return false;
    }
    
    byte serial[OPCSERIALLEN];
    for (int i=0; i<OPCSERIALLEN; i++) {
        sendDataCmd(0x00, spi);
        recvData(serial+i, 1, spi);
    }
    printf("OPCN2 Serial : ");
    for (int i=0; i<OPCSERIALLEN; i++) printf("%c", serial[i]);
    printf("\n");
        
    delayus(10000);
    return true;
}

bool readFirmVer(SPI* spi) {
    if (!sendDataCmd(0x12, spi)) return false;
    if (!delayus(10000)) return false;
    byte data[1];
    if (!recvData(data, 1, spi)) return false;
    if (data[0] != 0xF3) {
        perror("readFirmVer not 0xF3");
        return false;
    }
    
    byte firmver[OPCSFIRMVERLEN];
    for (int i=0; i<OPCSFIRMVERLEN; i++) {
        sendDataCmd(0x00, spi);
        recvData(firmver+i, 1, spi);
    }
    printf("OPCN2 Firmware Version : %02X.%02X\n", firmver[0], firmver[1]);
        
	delayus(10000);
    return true;
}

bool readHist2Buf(SPI* spi, PMRead* r) {
    if (!sendDataCmd(0x30, spi)) return false;
    if (!delayus(10000)) return false;
    byte data[1];
    if (!recvData(data, 1, spi)) return false;
    delayus(10);
    if (data[0] != 0xF3) {
        perror("readData not 0xF3");
        return false;
    }
    
    HistData histdata;
    for (int i=0; i<OPCHISTLEN; i++) {
        sendDataCmd(0x00, spi);
        delayus(10);
        recvData(histdata.bytes+i, 1, spi);
        delayus(10);
    }
    //printf("OPCN2 Histogram : ");
    //for (int i=0; i<OPCHISTLEN; i++) printf("%02X", histdata.bytes[i]);
    //printf("\n");
        
    struct timeval t;
    gettimeofday(&t, NULL);
    r->ts = t.tv_sec;
    r->tus = t.tv_usec;
    r->d.PM1 = histdata.PM1;
    r->d.PM2 = histdata.PM2;
    r->d.PM10 = histdata.PM10;
    for (int i=0; i<16; i++) r->d.Bin[i]=histdata.Bin[i];
    for (int i=0; i<4; i++) r->d.MToF[i]=histdata.MToF[i]/3.0;
    r->d.SFR = histdata.SFR;
    r->d.TP = histdata.TP;
    r->d.SP = histdata.SP;
    r->d.sum = histdata.sum;
	delayus(10000);
    return true;
}

bool readPm2File(SPI* spi, FILE* fp) {
    if (!sendDataCmd(0x32, spi)) return false;
    if (!delayus(10000)) return false;
    byte data[1];
    if (!recvData(data, 1, spi)) return false;
    if (data[0] != 0xF3) {
        perror("readData not 0xF3");
        return false;
    }
    
    PMData pmdata;
    for (int i=0; i<OPCPMLEN; i++) {
        sendDataCmd(0x00, spi);
        recvData(pmdata.bytes+i, 1, spi);
    }
    printf("OPCN2 PM Data (ug/m3) %ld :  PM1 %8.3f,   PM2.5 %8.3f,   PM10 %8.3f\n", time(NULL), pmdata.PM1, pmdata.PM2, pmdata.PM10);
    if (fp) fprintf(fp, "%ld,%8.3f,%8.3f,%8.3f\n", time(NULL), pmdata.PM1, pmdata.PM2, pmdata.PM10);
	delayus(10000);
    return true;
}

bool readPm2Buf(SPI* spi, PMRead* r) {
    if (!sendDataCmd(0x32, spi)) return false;
    delayus(10000);
    byte data[1];
    if (!recvData(data, 1, spi)) return false;
    delayus(10);
    if (data[0] != 0xF3) {
        perror("readData not 0xF3");
        return false;
    }
        
    PMData pmdata;
    for (int i=0; i<OPCPMLEN; i++) {
        sendDataCmd(0x00, spi);
        delayus(10);
        recvData(pmdata.bytes+i, 1, spi);
        delayus(10);
    }
    
    struct timeval t;
    gettimeofday(&t, NULL);
    r->ts = t.tv_sec;
    r->tus = t.tv_usec;
    r->d.PM1 = pmdata.PM1;
    r->d.PM2 = pmdata.PM2;
    r->d.PM10 = pmdata.PM10;
	delayus(10000);
    return true;
}
