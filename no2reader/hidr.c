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
#include <errno.h>
#include "hidapi.h"
#include "readpodac.h"
#include "hidr.h"

// data shared to all threads
NO2Buf no2buf;

int main(int argc, char *argv[]) {
    
    printf("no2read started\n");

    unlink(SOCKET_FILE);

    // thread to read no2 sensor
    pthread_t managereadtid = -1;
    if (pthread_create(&managereadtid, NULL, manageReadThread, NULL) != 0) {
        perrortime("manage read pthread_create error");
        exit(1);
    }
    
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
        if (pktr.cmd == (uint8_t)CMD_GET) {
            memset(&pkts, 0, PACKETSIZE);
            pkts.cmd = (uint8_t)CMD_DATA;
            int bidx = (no2buf.bufindex+BUFSIZE-1) % BUFSIZE;
            printf("bidx %d\n", bidx);
            NO2Read* d = &no2buf.no2read[bidx];
            memcpy(pkts.payload, d, sizeof(NO2Read)<PAYLOADSIZE?sizeof(NO2Read):PAYLOADSIZE);
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
    		printf("start no2 read thread\n");

			no2buf.hid = hid_open(0x04d8, 0x003f, NULL);
			if (no2buf.hid == NULL) {
				perrortime("cannot open hidraw device");
				exit(1);
			}

			hid_set_nonblocking(no2buf.hid, 0);
            ackData(no2buf.hid);

			pthread_create(&readtid, NULL, readThread, NULL);
			running = 1;
    	} else {
			bidx = (no2buf.bufindex+BUFSIZE-1) % BUFSIZE;
			ts = no2buf.no2read[bidx].ts;
			if (ts - tslast > READPERIOD/2) { // normal
				tslast = ts;
			} else {
				pthread_cancel(readtid);
				pthread_join(readtid, &ret);
				perrortime("no2 read thread cancelled.");
				running = 0;
			    hid_close(no2buf.hid);
			}
		}
		sleep(READPERIOD+2);
    }
	return NULL;
}

void* readThread(void* arg) {
    int bidx = 0;
    no2buf.count = 0;
    for (;;) {
        bidx = no2buf.bufindex % BUFSIZE;
        if (!readNo2toBuf(no2buf.hid, &(no2buf.no2read[bidx]))) {
        	perrortime("read data error.");
        	continue;
        }
        no2buf.bufindex = (bidx+1) % BUFSIZE;
        no2buf.count ++;
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

bool delayus(unsigned int usec) {
    struct timespec t;
    t.tv_sec = usec / 1000000;
    t.tv_nsec = (usec - t.tv_sec * 1000000) * 1000;
    if (nanosleep(&t, NULL) < 0) {
        perror("delayus nanosleep");        // Read back data
        return false;
    }
    return true;
}

