#ifndef ScCompute_H
#define ScCompute_H

#include <systemc.h>
#define WIDTH 32

SC_MODULE(ScCompute) {

    sc_in<bool> clk, rst;
    sc_in<sc_int<WIDTH> > riX, riY, qiX, qiY;
    sc_in<sc_int<WIDTH> > W, W_avg;
    sc_out<sc_int<WIDTH> > result;

    SC_CTOR(ScCompute) {
        SC_METHOD(compute);
    }

    void compute();

};

#endif   // SC_MODULE_H