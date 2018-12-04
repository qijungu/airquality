/*
# Copyright (C) 2018, Qijun Gu, Texas State University, qijun@txstate.edu
#
# Unless required by applicable law or agreed to in writing, software
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.
# */

#ifndef _USBISSSPI_H_
#define _USBISSSPI_H_

#include <stdbool.h>
#include <stdint.h>
#include <termios.h>

#define SPIBUFSIZE 16

typedef uint8_t byte;

typedef struct _spi {
    byte obuf[SPIBUFSIZE];  // output buffer
    byte ibuf[SPIBUFSIZE];  // input buffer
    struct termios defaults;         // to store initial default port settings
    struct termios config;           // These will be our new settings
    int fd;
} SPI;

bool openSPI(const char* dev, SPI* spi);
void closeSPI(const SPI* spi);

bool outSPI(SPI* spi, int len);
int inSPI(byte* data, int len, SPI* spi);
bool delayus(unsigned int usec);

#endif // _USBISSSPI_H_
