#include <systemc.h>
#include "ilog2.h"

SC_MODULE(Tb) {
    /*Signal Generator For Testing Simulator-Module*/
    sc_in<bool> clk;
    sc_out<bool> reset;
    sc_out<sc_uint<32>> input_signal;


    SC_CTOR(Tb) {
        
        SC_THREAD(input_generator);
        sensitive_neg << clk;
        SC_THREAD(reset_generator);
    }

    void input_generator() {
            input_signal.write(0);
            wait(10, SC_NS);
            input_signal.write(28);
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
    sc_signal<sc_uint<32> > input;
    sc_signal<sc_uint<5> > output;

    sc_trace_file *fp;   // Create VCD file
    fp = sc_create_vcd_trace_file("wave");   // open(fp), create wave.vcd file
    fp->set_time_unit(1, SC_NS);  // set tracing resolution to ns

    ilog2 ilog2_cal("ilog2_cal");
    ilog2_cal.clk(clk);
    ilog2_cal.rst(rst);
    ilog2_cal.in(input);
    ilog2_cal.outlog2(output);

    Tb tb("Tb");
    tb.clk(clk);
    tb.reset(rst);
    tb.input_signal(input);

    sc_trace(fp, clk, "clk");
    sc_trace(fp, rst, "rst");
    sc_trace(fp, input, "input");
    sc_trace(fp, output, "output");
    sc_start(1000, SC_NS); 
    sc_close_vcd_trace_file(fp); 
    return 0;
}
