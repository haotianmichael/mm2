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
    sc_signal<sc_int<WIDTH> > ri[MAX_SEGLENGTH];
    sc_signal<sc_int<WIDTH> > qi[MAX_SEGLENGTH];
    sc_signal<sc_int<WIDTH> > w[MAX_SEGLENGTH];

    void initialize_data() {
   
    }
    SC_CTOR(IODispatcher){
        SC_THREAD(initialize_data);
        sensitive << rst.pos();
    }
};
 
struct mcuIODispatcher : IODispatcher {

    sc_out<sc_int<WIDTH>> ri_out[InputLaneWIDTH];
    sc_out<sc_int<WIDTH>> qi_out[InputLaneWIDTH];
    sc_out<sc_int<WIDTH>> w_out[InputLaneWIDTH];

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

    sc_out<sc_int<WIDTH>> ri_out[InputLaneWIDTH + 1];
    sc_out<sc_int<WIDTH>> qi_out[InputLaneWIDTH + 1];
    sc_out<sc_int<WIDTH>> w_out[InputLaneWIDTH + 1];

    void shift_data();
    SC_CTOR(ecuIODispatcher) :
     IODispatcher("ecuIODispatcher"){
        SC_THREAD(shift_data);
        sensitive << clk.pos();
    }
};
#endif