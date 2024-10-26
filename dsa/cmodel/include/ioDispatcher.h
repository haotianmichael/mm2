#ifndef INPUT_DISPATCH_H
#define INPUT_DISPATCH_H

#include <systemc.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include "helper.h"

/* IODispatcher for Each Segment */
SC_MODULE(IODispatcher) {
    sc_in<bool> clk;      
    sc_in<bool> rst;
    sc_in<sc_int<WIDTH> > UpperBound; 
    sc_in<sc_int<WIDTH> > LowerBound;
    sc_signal<sc_int<WIDTH> > ri[MAX_SEGLENGTH];
    sc_signal<sc_int<WIDTH> > qi[MAX_SEGLENGTH];
    sc_signal<sc_int<WIDTH> > w[MAX_SEGLENGTH];

   
    SC_CTOR(IODispatcher){}
};
 
struct mcuIODispatcher : IODispatcher {

    sc_out<sc_int<WIDTH>> ri_out[MCUInputLaneWIDTH];
    sc_out<sc_int<WIDTH>> qi_out[MCUInputLaneWIDTH];
    sc_out<sc_int<WIDTH>> w_out[MCUInputLaneWIDTH];

    int cycle_count;
    void shift_data();
    SC_CTOR(mcuIODispatcher) :
    IODispatcher("mcuIODispatcher"), cycle_count(0) {
        SC_THREAD(shift_data);
        sensitive << clk.pos();
    }
};
 

struct ecuIODispatcher : IODispatcher {

    sc_in<sc_int<WIDTH> > SBase;
    sc_in<sc_int<WIDTH> > LBase; 

    sc_out<sc_int<WIDTH>> ri_out[ECUInputLaneWIDTH + 1];
    sc_out<sc_int<WIDTH>> qi_out[ECUInputLaneWIDTH + 1];
    sc_out<sc_int<WIDTH>> w_out[ECUInputLaneWIDTH + 1];

    void shift_data();
    SC_CTOR(ecuIODispatcher) :
    IODispatcher("ecuIODispatcher"){
        SC_THREAD(shift_data);
        sensitive << clk.pos();
    }
};
#endif