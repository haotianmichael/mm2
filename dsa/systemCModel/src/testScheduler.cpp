#include "scheduler.h"

SC_MODULE(Tb) {

    sc_in<bool> clk;
    sc_out<bool> reset;

    SC_CTOR(Tb) {
        SC_THREAD(reset_generator);
    }

    void reset_generator() {
        reset.write(1);
        wait(10, SC_NS);
        reset.write(0);
    }
};

int sc_main(int argc, char* argv[]) {

    sc_clock clk("clk", 10, SC_NS);
    sc_signal<bool> rst;

    sc_trace_file *fp;   // Create VCD file
    fp = sc_create_vcd_trace_file("wave");   // open(fp), create wave.vcd file
    fp->set_time_unit(1, SC_NS);  // set tracing resolution to ns

    Tb tb("Tb");
    tb.clk(clk);
    tb.reset(rst);


    Scheduler scheduler("chainScheduler");
    scheduler.clk(clk);
    scheduler.rst(rst);
    sc_trace(fp, clk, "clk");
    sc_trace(fp, rst, "rst");
    
    sc_trace(fp, scheduler.rc->anchorNum[0].read(), "anchorNum");

    sc_start(20000, SC_NS); 
    sc_close_vcd_trace_file(fp); 
    return 0;
}
