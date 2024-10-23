#ifndef HELP_H
#define HELP_H

#include <systemc.h>
#include <sstream>
#include <string.h>
#include "hcu.h"

#define WIDTH 32
#define TableWIDTH 64
#define HCU_NUM 64
#define Reduction_NUM 64
#define LaneWIDTH 64
#define InputLaneWIDTH 65
#define MAX_SEGLENGTH 5000
#define MAX_READ_LENGTH 30000
#define MAX_SEG_NUM 15000
#define IdleThreshLow 4
#define ReadNumProcessedOneTime  1

int countZerosInBigInt(const sc_dt::sc_bigint<TableWIDTH>& num) {
    int zeroCount = 0;
    for (int i = 0; i < TableWIDTH; ++i) {
        if (num[i] == 0) {
            ++zeroCount;
        }
    }
    return zeroCount;
}

int newHCU(const HCU &hcuPool) {
    int allo;
    for(int i = 0; i < HCU_NUM; i ++) {
        if(hcuPool[i].currentReadID == -1) {
            allo = i;
            break;
        }
    } 
    if(allo == HCU_NUM) return -1;
    return allo;
}

#endif