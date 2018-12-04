/*
# Copyright (C) 2018, Qijun Gu, Texas State University, qijun@txstate.edu
#
# Unless required by applicable law or agreed to in writing, software
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.
# */

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <time.h>

#include "usbissspi.h"
#include "opcn2.h"
#include "opcread.h"

// data shared to all threads
PMBuf pmbuf;
char device[100];
SPI spi;

int main(int argc, char *argv[]) {

    pmbuf.spi = &spi;
    
    perrortime("opcread started");

    unlink(SOCKET_FILE);
    
    if (argc == 1) {
    	strncpy(device, DEVICE, 100);
    } else if (argc == 2) {
    	strncpy(device, argv[1], 100);
    } else {
    	perrortime("./opcread [device]");
    	exit(1);
    }

    // thread to read sensor
    pthread_t managereadtid = -1;
    if (pthread_create(&managereadtid, NULL, manageReadThread, NULL) != 0) {
        perrortime("manage read pthread_create error");
        exit(1);
    }
    /*pthread_t readtid = -1;
	if (pthread_create(&readtid, NULL, readThread, NULL) != 0) {
		perrortime("read pthread_create error");
		sleep(1);
	}*/
    
    // serve inquiries with a unix socket SOCKET_FILE
    int sck = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (sck < 0) {
        perrortime("socket error");
        exit(1);
    }
    struct sockaddr_un address;
    memset(&address, 0, sizeof(struct sockaddr_un));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_FILE, sizeof(address.sun_path) - 1);
    if (bind(sck, (const struct sockaddr *)&address, sizeof(struct sockaddr_un)) < 0) {
        perrortime("bind error");
        exit(EXIT_FAILURE);
    }
    if (listen(sck, 20) < 0) {
        perrortime("listen error");
        exit(EXIT_FAILURE);
    }
    
    // thread to serve inquries
    pthread_t servtid = -1;
    int dsck = -1;
    for (;;) {
        dsck = accept(sck, NULL, NULL);
        if (dsck < 0) {
            perrortime("accept error");
            exit(1);
        }
        if (pthread_create(&servtid, NULL, servThread, &dsck) != 0) {
            perrortime("serv pthread_create error");
            close(dsck);
            continue;
        }
        while (dsck > 0) delayus(1000);
        pthread_detach(servtid);
    }
    
    close(sck);
    return 0;
}

void* servThread(void* arg) {
    int dsck = *((int*)arg);
    *((int*)arg) = -1;   // indicate to the parent that the socket descriptor is copied
    
    int dlen = 0;
    Packet pktr, pkts;
    for (;;) {
        memset(&pktr, 0, PACKETSIZE);
        dlen = read(dsck, &pktr, PACKETSIZE);
        if (dlen <= 0) {
            perrortime("read error");
            break;
        }
        #ifdef DEBUG
        printf("%c\n", (char)pktr.cmd);
        #endif
        if (pktr.cmd == (uint8_t)CMD_GET) {
            memset(&pkts, 0, PACKETSIZE);
            pkts.cmd = (uint8_t)CMD_DATA;
            int bidx = (pmbuf.bufindex+BUFSIZE-1) % BUFSIZE;
            printf("bidx %d\n", bidx);
            PMRead* d = &pmbuf.pmread[bidx];
            memcpy(pkts.payload, d, sizeof(PMRead)<PAYLOADSIZE?sizeof(PMRead):PAYLOADSIZE);
            dlen = write(dsck, &pkts, PACKETSIZE);
        } else if (pktr.cmd == (uint8_t)CMD_EXIT) {
            break;
        } else {
            memset(&pkts, 0, PACKETSIZE);
            pkts.cmd = (uint8_t)CMD_UNKNOWN;
            dlen = write(dsck, &pkts, PACKETSIZE);
        }
    }
    
    close(dsck);
    return NULL;
}

void* manageReadThread(void* arg) {
    pthread_t readtid = -1;
    uint8_t running = 0;
    int bidx = 0;
    void* ret = NULL;
    uint64_t tslast = 0, ts = 0;
    while (1) {
    	if (!running) {
    		printf("start opc read thread\n");
    	    if (!openSPI(device, &spi)) {
    	        perrortime("openSPI error");
    	        exit(1);
    	    }
    	    // read sensor info
    	    //readIssInfo(&spi);
    	    setSpiMode(&spi);
    	    //readInfo(&spi);
    	    uint8_t fan=0, laser=0;
    	    while (fan==0 && laser==0) {
    	        setFanLaser(&spi, 1);
    	        delayus(500000);
    			readStatus(&spi, &fan, &laser);
    			delayus(500000);
    	    }
    	    //readSerial(&spi);
    	    //readFirmVer(&spi);
			pthread_create(&readtid, NULL, readThread, NULL);
			running = 1;
    	} else {
			bidx = (pmbuf.bufindex+BUFSIZE-1) % BUFSIZE;
			ts = pmbuf.pmread[bidx].ts;
			if (ts - tslast > READPERIOD/2) { // normal
				tslast = ts;
			} else {
				pthread_cancel(readtid);
				pthread_join(readtid, &ret);
				perrortime("opc read thread cancelled.");
				running = 0;
			    closeSPI(&spi);
			}
		}
		sleep(READPERIOD+2);
    }
	return NULL;
}

void* readThread(void* arg) {
    int bidx = 0;
    pmbuf.count = 0;
    for (;;) {
        bidx = pmbuf.bufindex % BUFSIZE;
        if (!readHist2Buf(pmbuf.spi, &(pmbuf.pmread[bidx]))) {
        	perrortime("read data error.");
        	continue;
        }
        pmbuf.bufindex = (bidx+1) % BUFSIZE;
        pmbuf.count ++;
        #ifdef DEBUG
        printf("OPCN2 (ug/m3) %d %2d %llu.%06u: PM1 %7.3f, PM2.5 %7.3f, PM10 %7.3f,",
        	pmbuf.count,
            bidx,
            pmbuf.pmread[bidx].ts,
            pmbuf.pmread[bidx].tus,
            pmbuf.pmread[bidx].d.PM1,
            pmbuf.pmread[bidx].d.PM2,
            pmbuf.pmread[bidx].d.PM10);
        printf(" Bins ");
        for (int i=0; i<16; i++) printf("%d,", pmbuf.pmread[bidx].d.Bin[i]);
        printf(" MToFs ");
        for (int i=0; i<4; i++) printf("%.2f,", pmbuf.pmread[bidx].d.MToF[i]);
        printf(" SFR %.2f, TP %d, SP %.2f, sum %d\n",
            pmbuf.pmread[bidx].d.SFR,
            pmbuf.pmread[bidx].d.TP,
            pmbuf.pmread[bidx].d.SP,
            pmbuf.pmread[bidx].d.sum);
        #endif
        delayus(READPERIOD);
    }
    return NULL;
}
    
void perrortime(char* err) {
    time_t curtime;
    curtime = time(NULL);
    char* ct = ctime(&curtime);
    int len = strlen(ct);
    ct[len-1] = '\0';
    char errmsg[1024];
    len = strlen(ct) + strlen(" : ") + strlen(err);
    if (len > 1023) {
        len = strlen(ct) + strlen(" : ");
        err[1023-len] = '\0';
    }
    sprintf(errmsg, "%s : %s", ct, err); 
    perror(errmsg);
}
