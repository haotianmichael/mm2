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
    
    sc_trace(fp, scheduler.start.read(), "cutDone");
    std::ostringstream ri_name;
    for(int i =0; i < HCU_NUM; i ++) {
            ri_name << "riArray" << i;
            sc_trace(fp, scheduler.mcuPool[i]->riArray[0].read(), ri_name.str());
            ri_name.str("");
    }

    std::ostringstream name;
    for(int i = 0; i < HCU_NUM; i ++) {
        name << "Output" << i;
        sc_trace(fp, scheduler.mcuPool[i]->regBiggerScore[0].read(), name.str());
        name.str("");
    }

    std::stringstream IOname;
    for(int i = 0; i < HCU_NUM; i ++) {
        IOname << "IO" << i;
        sc_trace(fp, scheduler.mcuIODisPatcherPool[i]->ri[0].read(), IOname.str());
        IOname.str("");
    }
    

    sc_start(70, SC_NS); 
    sc_close_vcd_trace_file(fp); 
    return 0;
}
