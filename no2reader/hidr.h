#ifndef _HIDR_H_
#define _HIDR_H_

#include <stdint.h>
#include <stdbool.h>
#include "hidapi.h"

#define BUFSIZE (16)
#define SOCKET_FILE "/tmp/no2reader.socket"
#define READPERIOD 250000 // in us

#define NO2DATALEN 8
typedef union _READData {
    struct {
        float we;        // WE in ppb
        float ae;        // AE in ppb
    } __attribute__((packed));
    uint8_t bytes[NO2DATALEN];
} READData;

typedef struct _NO2Read {
    uint64_t ts;
    uint32_t tus;
    READData d;
} __attribute__((packed)) NO2Read;

typedef struct _NO2Buf {
    hid_device* hid;
    NO2Read no2read[BUFSIZE];  // data buf
    int bufindex;    // index to write data to buf
    int count;
} __attribute__((packed)) NO2Buf;

extern NO2Buf no2buf;

void perrortime(char* err);
bool delayus(unsigned int usec);

void* servThread(void* arg);
void* manageReadThread(void* arg);
void* readThread(void* arg);

#define PACKETSIZE (128)
#define PAYLOADSIZE (PACKETSIZE-1)

typedef struct _Packet {
    uint8_t cmd;
    uint8_t payload[PAYLOADSIZE];
} __attribute__((packed)) Packet;

#define CMD_NUM_MAX 127            /* max number of commands     */
#define CHAN_NUM     32            /* the number of channels     */

#define CMD_LEN_MAX (64)           /* max length of command, including header, because the max block size of USB is 64B      */

/***
 * A command has two modes: binary or char
***/
typedef enum {
    CHAR_CMD = 0,               /* if command is char, its body must be char      */
    BIN_CMD  = 1,               /* if command is binary, its body must be binary  */
} CMD_MODE;

#define IS_BIN_CMD(cmd)   ((cmd & 0x80) != 0)
#define IS_CHAR_CMD(cmd)  ((cmd & 0x80) == 0)

#define UPPERCASE(x)  (x & 0x5F)

/***
 * A command is associated with a char and a binary.
 * If no char for a command, the command is binary only.
 * The char of a command is NOT case sensitive.
***/
typedef enum {
    CMD_NONE = 0,

    CMD_REBOOT = 'R',              /* R/r : reboot PODAC                             */
    CMD_FREQ = 'F',                /* F/f : change the sampling frequency in PODAC   */
    CMD_PAUSE = 'P',               /* P/p : pause sampling in PODAC                  */
    CMD_CONTINUE = 'C',            /* C/c : continue sampling in PODAC               */
    CMD_TIME = 'T',                /* T/t : set time in PODAC                        */
    CMD_CHANNEL = 'H',             /* H/h : set channel in PODAC                     */

    CMD_GET = 'G',                 /* G/g : get data             */
    CMD_DATA = 'D',                /* D/d : data from PODAC                          */
    CMD_EXIT = 'X',                /* X/x : exit                 */
    CMD_WAIT = 'W',                /* W/w : waiting for data     */
    CMD_UNKNOWN = 'U',             /* U/u : unknow command       */

    CMD_DATA_ACK,
    CMD_RESEND,                    /*     : resend received command                  */
    CMD_ERROR = CMD_NUM_MAX        /*     : error in received command                */
    
} CMD_TYPE;

/***
 * A command is one byte. The most significant bit is the mode.
***/
typedef union {
    uint8_t value;
    struct {
        uint8_t type : 7;            /* must be of CMD_TYPE                            */
        uint8_t mode : 1;            /* BIN_CMD (1) or CHAR_CMD (0)                    */
    } __attribute__((packed));
} CMD;

/***
 * The structure is a variation of rtccTimeDate defined in rtcc.h
***/
typedef union {
    struct {
        uint8_t sec;        // BCD codification for seconds, 00-59
        uint8_t min;        // BCD codification for minutes, 00-59
        uint8_t hour;       // BCD codification for hours, 00-24
        uint8_t mon : 5;    // BCD codification for month, 01-12
        uint8_t wday: 3;    // BCD codification for day of week, 00-06
        uint8_t mday;       // BCD codification for day of the month, 01-31
        uint8_t year;       // BCD codification for year, 00-99
    } __attribute__((packed));
    uint8_t t[6];
} RTCCTIME;

/***
 * The structure of a binary command header.
***/
#define BIN_CMD_HEADER_LEN (12)        /* header size is fixed at 8                     */
#define BIN_CMD_PAYLOAD_LEN_MAX (CMD_LEN_MAX - BIN_CMD_HEADER_LEN)   /* payload size varies, but max at 48            */
typedef struct _command_bin_ {
    CMD cmd;                     /* command, same as COMMAND_CHAR                 */
    uint8_t len;                 /* length of the complete command, including the header, must be smaller than CMD_LEN_MAX */
    uint16_t seq;                /* sequence number, assuming fewer than 65536 commands per second */
    RTCCTIME time;               /* time stamp with second precision. it use the type of rtccTimeDate declared in rtcc.h   */
    uint16_t crc;                /* check sum of the command body, including itself as 0 */
    uint8_t payload[BIN_CMD_PAYLOAD_LEN_MAX];  /* command body                    */
} __attribute__((packed)) COMMAND;

/***
 * The structure of a data item, two bytes.
***/
typedef union {
    uint16_t data;
    struct {
        uint16_t value : 11;         /* value of data                                  */
        uint16_t channel : 5;        /* channel of data                                */
    } __attribute__((packed));
} DATAITEM;

/***
 * The structure of data payload
***/
#define PAYLOAD_DATA_LEN (2 + CHAN_NUM / 2 * sizeof(DATAITEM))
typedef struct _payload_data_ {
    uint16_t dindex; // the first or second half of data: 1 or 2
    DATAITEM d[CHAN_NUM / 2];
} __attribute__((packed)) PAYLOAD_DATA;

/***
 * The structure of a char command
***/
#define CHAR_CMD_HEADER_LEN (1)        /* header                  */
#define CHAR_CMD_PAYLOAD_LEN_MAX (CMD_LEN_MAX - CHAR_CMD_HEADER_LEN)  /* payload size varies     */

typedef struct _command_char_ {
    CMD cmd;                                       /* char command            */
    uint8_t body[CHAR_CMD_PAYLOAD_LEN_MAX];  /* payload, char only      */
} __attribute__((packed)) COMMAND_CHAR;

#endif // _HIDR_H_
