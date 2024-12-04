#ifndef INPUT_DISPATCH_H
#define INPUT_DISPATCH_H

#include <systemc.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include "helper.h"

/* IODispatcher for Each Segment */
SC_MODULE(mcuIODispatcher){

    sc_in<bool> clk;      
    sc_in<bool> rst;
    sc_signal<bool> en;
    sc_signal<sc_int<WIDTH> > UpperBound; 
    sc_signal<sc_int<WIDTH> > LowerBound;
    sc_signal<sc_int<WIDTH> > ri[MAX_SEGLENGTH];
    sc_signal<sc_int<WIDTH> > qi[MAX_SEGLENGTH];
    sc_signal<sc_int<WIDTH> > w[MAX_SEGLENGTH];
    sc_signal<sc_int<WIDTH> > idx[MAX_SEGLENGTH];

    sc_out<sc_int<WIDTH>> ri_out[MCUInputLaneWIDTH + 1];
    sc_out<sc_int<WIDTH>> qi_out[MCUInputLaneWIDTH + 1];
    sc_out<sc_int<WIDTH>> w_out[MCUInputLaneWIDTH + 1];
    sc_out<sc_int<WIDTH>> idx_out[MCUInputLaneWIDTH + 1];

    int cycle_count;
    void shift_data();
    SC_CTOR(mcuIODispatcher) : cycle_count(0) {
        SC_THREAD(shift_data);
        sensitive << clk.pos();
    }
};
 

SC_MODULE(ecuIODispatcher) {

    sc_in<bool> clk;      
    sc_in<bool> rst;
    sc_signal<bool> en;
    sc_signal<sc_int<WIDTH> > UpperBound; 
    sc_signal<sc_int<WIDTH> > LowerBound;
    sc_signal<sc_int<WIDTH> > ri[MAX_SEGLENGTH];
    sc_signal<sc_int<WIDTH> > qi[MAX_SEGLENGTH];
    sc_signal<sc_int<WIDTH> > w[MAX_SEGLENGTH];
    sc_signal<sc_int<WIDTH> > idx[MAX_SEGLENGTH];
    sc_signal<sc_int<WIDTH> > final_score[MAX_SEGLENGTH];

    int cycle_count;
    int base_count;
    sc_out<sc_int<WIDTH>> ri_out[ECUInputLaneWIDTH + 1];
    sc_out<sc_int<WIDTH>> qi_out[ECUInputLaneWIDTH + 1];
    sc_out<sc_int<WIDTH>> w_out[ECUInputLaneWIDTH + 1];
    sc_out<sc_int<WIDTH>> idx_out[ECUInputLaneWIDTH + 1];
    sc_out<sc_int<WIDTH> > ecu_ri_out;
    sc_out<sc_int<WIDTH> > ecu_qi_out;
    sc_out<sc_int<WIDTH> > ecu_w_out;
    sc_out<sc_int<WIDTH> > ecu_idx_out;
    sc_out<sc_int<WIDTH>> ecu_score_out;

    void shift_data();
    SC_CTOR(ecuIODispatcher) : cycle_count(LowerBound.read()), base_count(0){
        SC_THREAD(shift_data);
        sensitive << clk.pos();
    }
};
#endif