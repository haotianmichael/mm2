#ifndef RC_UNIT_H
#define RC_UNIT_H

#include <systemc.h>
#include "hcu.h"

SC_MODULE(RangeCountUnit) {

    sc_in<bool> rst;
    /* 
        @max read length: 30000
        @max segments length: 5000
    */
    sc_out<bool> cutDone;
    sc_out<sc_int<WIDTH> > anchorNum[ReadNumProcessedOneTime];
    sc_out<sc_int<WIDTH> > anchorRi[ReadNumProcessedOneTime][MAX_READ_LENGTH];
    sc_out<sc_int<WIDTH> > anchorQi[ReadNumProcessedOneTime][MAX_READ_LENGTH];
    sc_out<sc_int<WIDTH> > anchorW[ReadNumProcessedOneTime][MAX_READ_LENGTH];
    sc_out<sc_int<WIDTH>> anchorSuccessiveRange[ReadNumProcessedOneTime][MAX_READ_LENGTH];

    void takeReadsAndCut();
    SC_CTOR(RangeCountUnit) {

        SC_THREAD(takeReadsAndCut);
        sensitive << rst;
    }

};

#endif