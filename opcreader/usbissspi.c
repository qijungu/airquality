/*
# Copyright (C) 2018, Qijun Gu, Texas State University, qijun@txstate.edu
#
# Unless required by applicable law or agreed to in writing, software
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.
# */

#include <stdbool.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>

#include "usbissspi.h"

bool openSPI(const char* device, SPI* spi) {

    int fd = open(device, O_RDWR | O_NOCTTY);
    if(fd < 0) {
        perror("failed to open port" );
        return false;
    }
    if (tcgetattr(fd, &spi->defaults) < 0) {
        perror("openSPI tcgetattr");          // Grab snapshot of current settings  for port
        return false;
    }
    cfmakeraw(&spi->config);            // make options for raw data
    if (tcsetattr(fd, TCSANOW, &spi->config) < 0) {
        perror("openSPI tcsetattr");       // Set options for port
        return false;
    }
    
    spi->fd = fd;
    return true;
}

void closeSPI(const SPI* spi) {
    if (tcsetattr(spi->fd, TCSANOW, &spi->defaults) < 0) {
        perror("closeSPI tcsetattr");    // Restore port default before closing
    }
    close(spi->fd);
}

/* 
 * spi has the command already.
 * len is the number of bytes in the command.
 */
bool outSPI(SPI* spi, int len) {
    if (write(spi->fd, spi->obuf, len) < 0) {
        perror("outSPI write");    // Write data to USB-ISS
        return false;
    }
    if (tcdrain(spi->fd) < 0) {
        perror("outSPI tcdrain");
        return false;
    }
    return true;
}

/* 
 * read to data with at most len bytes
 * return the number of bytes read
 */
int inSPI(byte* data, int len, SPI* spi) {
    int rlen, i;
    if ((rlen = read(spi->fd, spi->ibuf, len)) < 0) {
        perror("inSPI read");        // Read back data
        return -1;
    }
    for (i=0; i<rlen; i++) data[i]=spi->ibuf[i];
    return rlen;    
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

