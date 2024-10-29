#include <systemc.h>
#include "sc.h"

SC_MODULE(Tb) {
/*
    ScComupte Module:
        @generate result after 15cycles of input (same as rtl-version)
*/
    sc_in<bool> clk;
    sc_out<bool> reset;
    sc_out<sc_uint<32>> riX, riY, qiX, qiY;
    sc_out<sc_uint<32>> W, W_avg;


    SC_CTOR(Tb) {
        
        SC_THREAD(input_generator);
        sensitive_neg << clk;
        SC_THREAD(reset_generator);
    }

    void input_generator() {
            wait(20, SC_NS);
            riX.write(100);
            riY.write(30);
            qiX.write(50);
            qiY.write(20);
            W.write(15);
            W_avg.write(15);
            wait(80, SC_NS);
    }

    void reset_generator() {
        reset.write(0);
        wait(5, SC_NS);
        reset.write(1);
        wait(10, SC_NS); 
        reset.write(0);
        wait(100, SC_NS);
    }

};

int sc_main(int argc, char* argv[]) {
    sc_clock clk("clk", 10, SC_NS);
    //generating input signal for ilog2
    sc_signal<bool> rst;
    sc_signal<sc_uint<32> > riX, riY, qiX, qiY;
    sc_signal<sc_uint<32> > W, W_avg;
    sc_signal<sc_uint<32> > result;

    sc_trace_file *fp;   // Create VCD file
    fp = sc_create_vcd_trace_file("wave");   // open(fp), create wave.vcd file
    fp->set_time_unit(1, SC_NS);  // set tracing resolution to ns

    ScCompute sc_core("ScCompute_Core");
    sc_core.clk(clk);
    sc_core.rst(rst);
    sc_core.riX(riX);
    sc_core.riY(riY);
    sc_core.qiX(qiX);
    sc_core.qiY(qiY);
    sc_core.W(W);
    sc_core.W_avg(W_avg);
    sc_core.result(result);


    Tb tb("Tb");
    tb.clk(clk);
    tb.reset(rst);
    tb.riX(riX);
    tb.riY(riY);
    tb.qiX(qiX);
    tb.qiY(qiY);
    tb.W(W);
    tb.W_avg(W_avg);

    sc_trace(fp, clk, "clk");
    sc_trace(fp, rst, "rst");
    sc_trace(fp, riX, "riX");
    sc_trace(fp, riY, "riY");
    sc_trace(fp, qiX, "qiX");
    sc_trace(fp, qiY, "qiY");
    sc_trace(fp, W, "W");
    sc_trace(fp, result, "result");


    sc_start(1000, SC_NS); 
    sc_close_vcd_trace_file(fp); 
    return 0;
}
