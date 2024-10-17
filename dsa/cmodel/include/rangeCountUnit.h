#ifndef RC_UNIT_H
#define RC_UNIT_H

#include <systemc.h>
#include "hcu.h"
#include "inputDispatcher.h"

SC_MODULE(RangeCountUnit) {

    sc_in<bool> rst;
    /* 
        @max read length: 30000
        @max segments length: 5000
    */
    sc_out<bool> cutDone;
    sc_out<sc_int<WIDTH> > anchorNum;
    sc_out<sc_int<WIDTH> > anchorRi[MAX_READ_LENGTH];
    sc_out<sc_int<WIDTH> > anchorQi[MAX_READ_LENGTH];
    sc_out<sc_int<WIDTH> > anchorW[MAX_READ_LENGTH];
    sc_out<sc_int<WIDTH>> anchorSuccessiveRange[MAX_READ_LENGTH];

    void takeOneReadAndCut();
    SC_CTOR(RangeCountUnit) {
        SC_THREAD(takeOneReadAndCut);
        sensitive << rst.pos();
    }

};

#endif