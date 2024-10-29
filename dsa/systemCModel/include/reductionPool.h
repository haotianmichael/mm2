#ifndef REDUCTION_POOL_H
#define REDUCTION_POOL_H

#include "helper.h"

SC_MODULE(ReductionPool) {

    /*
        32 kinds of ReductionTree, each has two, totally 64 ReductionTree
        1,2  2-path Reduction
        3,4  4-path Reduction
        5,6  6-path Reduction
        ...
        63,64  64-path Reduciton
    
    */
    sc_in<bool> clk, rst;
    sc_in<sc_int<WIDTH> > reductionArray[Reduction_NUM];
    //sc_out<sc_bigint<TableWIDTH> > ROCC;

    void execute() {

    }


    SC_CTOR(ReductionPool){

        SC_THREAD(execute); 
        sensitive << clk.pos(); 

    }


};


#endif