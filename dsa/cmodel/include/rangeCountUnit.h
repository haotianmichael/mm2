#ifndef RC_UNIT_H
#define RC_UNIT_H

#include <systemc.h>
#include "hcu.h"
#include "inputDispatcher.h"

SC_MODULE(RangeCountUnit) {

    sc_in<bool> clk;
    sc_in<bool> rst;
    sc_out<bool> isLT64;
    sc_out<sc_int<WIDTH>> 
    SC_CTOR(RangeCountUnit) {
    }

};

#endif