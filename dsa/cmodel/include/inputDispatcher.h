#ifndef INPUT_DISPATCH_H
#define INPUT_DISPATCH_H

#include <systemc.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include "helper.h"

/* InputDispatcher for Each Segment */
SC_MODULE(InputDispatcher) {
    sc_in<bool> clk;      
    sc_in<bool> rst;
    sc_in<sc_int<WIDTH> > UpperBound; 
    sc_signal<sc_int<WIDTH> > ri[MAX_SEGLENGTH];
    sc_signal<sc_int<WIDTH> > qi[MAX_SEGLENGTH];
    sc_signal<sc_int<WIDTH> > w[MAX_SEGLENGTH];

    void initialize_data() {
   
    }
    SC_CTOR(InputDispatcher){
        SC_THREAD(initialize_data);
        sensitive << rst.pos();
    }
};
 
struct mcuInputDispatcher : InputDispatcher {

    sc_out<sc_int<WIDTH>> ri_out[InputLaneWIDTH];
    sc_out<sc_int<WIDTH>> qi_out[InputLaneWIDTH];
    sc_out<sc_int<WIDTH>> w_out[InputLaneWIDTH];

    int cycle_count;
    void shift_data();
    SC_CTOR(mcuInputDispatcher) :
    InputDispatcher("mcuInputDispatcher"), cycle_count(0) {
        SC_THREAD(shift_data);
        sensitive << clk.pos();
    }
};
 

struct ecuInputDispatcher : InputDispatcher {

    sc_in<sc_int<WIDTH> > SBase;
    sc_in<sc_int<WIDTH> > LBase; 

    sc_out<sc_int<WIDTH>> ri_out[InputLaneWIDTH + 1];
    sc_out<sc_int<WIDTH>> qi_out[InputLaneWIDTH + 1];
    sc_out<sc_int<WIDTH>> w_out[InputLaneWIDTH + 1];

    void shift_data();
    SC_CTOR(ecuInputDispatcher) :
     InputDispatcher("ecuInputDispatcher"){
        SC_THREAD(shift_data);
        sensitive << clk.pos();
    }
};
#endif