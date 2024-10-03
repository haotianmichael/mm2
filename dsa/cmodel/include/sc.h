#ifndef ScCompute_H
#define ScCompute_H

#include <systemc.h>
#include "ilog2.h"
#include "ifloat.h"
#define WIDTH 32

SC_MODULE(ScCompute) {

    sc_in<bool> clk, rst;
    sc_in<sc_int<WIDTH> > riX, riY, qiX, qiY;
    sc_in<sc_int<WIDTH> > W, W_avg;
    sc_out<sc_int<WIDTH> > result;

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
    sc_signal<sc_uint<16> >  absDiff, min;
    sc_signal<sc_uint<16> > mult;
    void tQR();
    void tRQ();
    void abs();
    void mult();


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
        ilog2_cal.clk(clk);
        ilog2_cal.rst(rst);
        ilog2_cal->in(absDiff);
        ilog2_cal->outlog2(log2_val);

        /*FLoatLUT*/
        flut = new FloatLUT("FloatLookUpTable"); 


        SC_THREAD(rXY());
        sensitive_pos << clk;
        async_reset_signal_is(rst, true); 

        SC_THREAD(rYX());
        sensitive_pos << clk;
        async_reset_signal_is(rst, true); 

        SC_THREAD(gapR());
        sensitive_pos << clk;
        async_reset_signal_is(rst, true); 

        SC_THREAD(qXY());
        sensitive_pos << clk;
        async_reset_signal_is(rst, true); 

        SC_THREAD(qXY());
        sensitive_pos << clk;
        async_reset_signal_is(rst, true); 


        SC_THREAD(gapQ());
        sensitive_pos << clk;
        async_reset_signal_is(rst, true); 

        SC_THREAD(tRQ());
        sensitive_pos << clk;
        async_reset_signal_is(rst, true); 

        SC_THREAD(tQR());
        sensitive_pos << clk;
        async_reset_signal_is(rst, true); 

        SC_THREAD(abs());
        sensitive_pos << clk;
        async_reset_signal_is(rst, true); 

        SC_THREAD(mult());
        sensitive_pos << clk;
        async_reset_signal_is(rst, true); 

        SC_THREAD(log_result());
        sensitive_pos << clk;
        async_reset_signal_is(rst, true); 

        SC_THREAD(get_partialSum);
        sensitive_pos << clk;
        async_reset_signal_is(rst, true);

        SC_THREAD(getA);
        sensitive_pos << clk;
        async_reset_signal_is(rst, true);

        SC_THREAD(getB);
        sensitive_pos << clk;
        async_reset_signal_is(rst, true);

        SC_THREAD(compute);
        sensitive_pos << clk;
        async_reset_signal_is(rst, true);

    }


};

#endif   // SC_MODULE_H