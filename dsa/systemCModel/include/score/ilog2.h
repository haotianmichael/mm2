#ifndef ILOG2_H
#define ILOG2_H

#include <systemc.h>
#define FULL_WIDTH 32
#define LOGWIDTH 5

SC_MODULE(ilog2) {

    sc_in<bool> clk, rst;
    sc_in<sc_uint<FULL_WIDTH> > in;  // input
    sc_out<sc_uint<LOGWIDTH> > outlog2;  // output

    // pipeline port
    sc_signal<sc_uint<FULL_WIDTH> > stage1_v;
    sc_signal<bool> valid;
    sc_signal<sc_uint<16> > stage2_v;
    sc_signal<sc_uint<5> > stage2_log2;

    sc_signal<sc_uint<8> > stage3_indx;
    sc_signal<sc_uint<8> > stage3_log2;

    sc_signal<sc_uint<5> > stage4_logtable_value;
    sc_signal<sc_uint<5> > stage5_log2;
   
    sc_uint<4>  LogTable256[256] = {
        0, 0, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7
    };

    SC_CTOR(ilog2) {
        SC_THREAD(initialize);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true);

        SC_THREAD(stage1);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true);

        SC_THREAD(stage2);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true);

        SC_THREAD(stage3);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true);

        SC_THREAD(stage4);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true);

        SC_THREAD(end);
        sensitive << clk.pos();
        async_reset_signal_is(rst, true);
    }

    void initialize();
    void stage1();
    void stage2();
    void stage3();
    void stage4();
    void end();
};

#endif