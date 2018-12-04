/*
# Copyright (C) 2018, Qijun Gu, Texas State University, qijun@txstate.edu
#
# Unless required by applicable law or agreed to in writing, software
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.
# */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>
#include "hidapi.h"
#include "readpodac.h"
#include "datamap.h"

DATAITEM sample[CHAN_NUM];
COMMAND sample_cmd;
COMMAND out_cmd;

bool readNo2toBuf(hid_device* hid, NO2Read* r) {
    unsigned char buf[64];
    int len;
	len = hid_read(hid, buf, 64); // read PODAC
	
    /*int i;
    unsigned char* c = buf;
    printf("CMD (%3d): ", len);
    for (i = 0; i < len; i++) 
       printf("%02X", c[i]);
    printf("\n");
    fflush(NULL);*/
    
	if (len <= 0) return false;
	COMMAND* cmd = (COMMAND*)buf;
    if (IS_CHAR_CMD(cmd->cmd.value)) { // char command
        if (UPPERCASE(cmd->cmd.type) == 'D') processDATA(cmd, r, hid);
    } else { // binary command
        if (cmd->cmd.type == CMD_DATA) processDATA(cmd, r, hid); // process data
    }
    return true;
}

// process the data from PODAC
void processDATA(COMMAND* cmd, NO2Read* r, hid_device* hid) {
	PAYLOAD_DATA* pd = (PAYLOAD_DATA*)cmd->payload;
	
	if (pd->dindex == 1) { // the first half of a data packet
		memcpy(&sample[0], pd->d, CHAN_NUM / 2 * sizeof(DATAITEM)); // get the first half of data
		memcpy(&sample_cmd, cmd, BIN_CMD_HEADER_LEN); // get the command header
	} else if (pd->dindex == 2 && sample_cmd.seq == cmd->seq) { // the second half of a data packet
		memcpy(&sample[CHAN_NUM / 2], pd->d, CHAN_NUM / 2 * sizeof(DATAITEM)); // get the second half of data
		float v1 = ch2real(sample[23].channel, sample[23].value);
		float v2 = ch2real(sample[22].channel, sample[22].value);
		float we = volt2no2(v1);
		float ae = volt2no2(v2);
		printf("DATA: OP1(WE) %03X (%.3fV, %.1fppb), OP2(AE) %03X (%.3fV,%.1fppb)\n", 
			sample[23].value, v1, we, 
			sample[22].value, v2, ae);
		struct timeval t;
		gettimeofday(&t, NULL);
		r->ts = t.tv_sec;
		r->tus = t.tv_usec;
		r->d.we = we;
		r->d.ae = ae;
	}

	ackData(hid);
}

void ackData(hid_device* dev) {
    COMMAND cmd_out;
    cmd_out.cmd.value = CMD_DATA_ACK;
    hid_write(dev, (unsigned char*)&cmd_out, BIN_CMD_HEADER_LEN);
}
