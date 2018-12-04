/*
# Copyright (C) 2018, Qijun Gu, Texas State University, qijun@txstate.edu
#
# Unless required by applicable law or agreed to in writing, software
# is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
# OF ANY KIND, either express or implied.
# */

#include "datamap.h"

float ch2real(int ch, int val) {
  return val * SAM;
}

float volt2no2(float volt) {
  return (volt * 1000.0 - ZERO) / COEF * 1000.0;
}
