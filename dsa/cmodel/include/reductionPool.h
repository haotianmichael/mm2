#ifndef REDUCTION_POOL_H
#define REDUCTION_POOL_H

#include "helper.h"

SC_MODULE(ReductionPool) {

    sc_in<bool> clk, rst;
    sc_in<sc_int<WIDTH> > reductionArray[Reduction_NUM];
    sc_out<sc_bigint<TableWIDTH> > ROCC;

    void execute() {


    }


    SC_CTOR(ReductionPool){

        SC_THREAD(execute); 
        sensitive << clk.pos(); 

    }


};


#endif