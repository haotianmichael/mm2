#ifndef ScCompute_H
#define ScCompute_H

#include <systemc.h>
#include "ilog2.h"
#include "ifloat.h"
#define WIDTH 32

SC_MODULE(ScCompute) {

    sc_in<bool> clk, rst;
    sc_in<sc_uint<WIDTH> > riX, riY, qiX, qiY;
    sc_in<sc_uint<WIDTH> > W, W_avg;
    sc_out<sc_uint<WIDTH> > result;

    /*pipeline port*/
    sc_signal<sc_uint<WIDTH> > tmpRXY, tmpRYX;
    sc_signal<sc_uint<WIDTH> > tmpQXY, tmpQYX;
    sc_signal<sc_uint<WIDTH> > diffR, diffQ;
    void rXY();
    void rYX();
    void gapR();
    void qXY();
    void qYX();
    void gapQ();

    sc_signal<sc_uint<WIDTH> > tmpQR, tmpRQ;
    sc_signal<sc_uint<WIDTH> >  absDiff, min;
    sc_signal<sc_uint<16> > mult;
    void tQR();
    void tRQ();
    void abs();
    void get_mult();


    sc_signal<sc_uint<5> > log2_val;
    sc_signal<sc_uint<32> > log_res;
    sc_signal<sc_uint<32> > partialSum;
    sc_signal<sc_uint<32> > A;
    sc_signal<sc_uint<32> > B;
    void get_log();
    void get_partialSum();
    void getA();
    void getB();
    void compute();
    
    ilog2 *ilog2_cal;
    FloatLUT *flut;

    SC_CTOR(ScCompute) {

        /*ilog2*/
        ilog2_cal = new ilog2("Log2Calculator");
        ilog2_cal->clk(clk);
        ilog2_cal->rst(rst);
        ilog2_cal->in(absDiff);
        ilog2_cal->outlog2(log2_val);

        /*FLoatLUT*/
        flut = new FloatLUT("FloatLookUpTable"); 


        SC_THREAD(rXY);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true); 

        SC_THREAD(rYX);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true); 

        SC_THREAD(gapR);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true); 

        SC_THREAD(qXY);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true); 

        SC_THREAD(qYX);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true); 


        SC_THREAD(gapQ);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true); 

        SC_THREAD(tRQ);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true); 

        SC_THREAD(tQR);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true); 

        SC_THREAD(abs);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true); 

        SC_THREAD(get_mult);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true); 

        SC_THREAD(get_log);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true); 

        SC_THREAD(get_partialSum);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true);

        SC_THREAD(getA);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true);

        SC_THREAD(getB);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true);

        SC_THREAD(compute);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true);

    }


};

#endif   // SC_MODULE_H