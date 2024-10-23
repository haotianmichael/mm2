#ifndef PARTIAL_SCORE_QUEUE_H
#define PARTIAL_SCORE_QUEUE_H

#include "helper.h"

struct PSTable {

    sc_int<WIDTH> segID;
    sc_int<WIDTH> NumUB;
    sc_int<TableWIDTH> REQ;
    sc_int<WIDTH> Priority;
};


SC_MODULE(PartialScoreQueue) {


    // FIFO and fill the PSQTable
    sc_in<bool> clk, rst;
    PSTable psTable[MAX_SEG_NUM];   

};

#endif