#ifndef INPUT_DISPATCH_H
#define INPUT_DISPATCH_H

#include <systemc.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include "helper.h"

SC_MODULE(InputDispatcher) {
    sc_in<bool> clk;      
    sc_in<bool> rst;
    sc_in<sc_int<WIDTH> > UpperBound; 
    sc_signal<sc_int<WIDTH> > ri[MAX_SEGLENGTH];
    sc_signal<sc_int<WIDTH> > qi[MAX_SEGLENGTH];
    sc_signal<sc_int<WIDTH> > w[MAX_SEGLENGTH];

    sc_out<sc_int<WIDTH>> ri_out[InputLaneWIDTH];
    sc_out<sc_int<WIDTH>> qi_out[InputLaneWIDTH];
    sc_out<sc_int<WIDTH>> w_out[InputLaneWIDTH];

    int cycle_count;
    SC_CTOR(InputDispatcher) : cycle_count(0) {
    }
};
 
struct mcuInputDispatcher : InputDispatcher {

    void initialize_data();
    void shift_data();
    SC_CTOR(mcuInputDispatcher) :
    InputDispatcher("InputDispatcher") {
        SC_THREAD(shift_data);
        sensitive << clk.pos();

        SC_THREAD(initialize_data);
        sensitive << rst.pos();
    }
};
 

struct ecuInputDispatcher : InputDispatcher {

    sc_in<sc_int<WIDTH> > SBase;
    sc_in<sc_int<WIDTH> > LBase; 

    void initialize_data();
    void shift_data();
    SC_CTOR(ecuInputDispatcher) :
     InputDispatcher("InputDispatcher"){
        SC_THREAD(shift_data);
        sensitive << clk.pos();

        SC_THREAD(initialize_data);
        sensitive << rst.pos();
    }
};
#endif