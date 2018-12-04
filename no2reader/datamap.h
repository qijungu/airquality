/*
# Copyright (C) 2018, Qijun Gu, Texas State University, qijun@txstate.edu
#
# Unless required by applicable law or agreed to in writing, software
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.
# */

#ifndef _DATAMAP_
#define _DATAMAP_

/* === do not modify anything below === */

/* Sensor board configuration
 * All resistances are in Ohm
 */

#define AMP        1.0
#define ZERO     230.0  // (mv)
#define COEF     165.0  // (mv/ppm)

/* sample amplication */
#define SAM  (3.3 / 2048.0 / AMP)

float ch2real(int ch, int val);  // return voltage
float volt2no2(float volt);     // return ppb of no2

#endif // _DATAMAP_
