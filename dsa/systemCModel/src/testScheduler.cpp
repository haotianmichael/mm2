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
    std::ostringstream mri_name, eri_name;
    for(int i =0; i < HCU_NUM; i ++) {
            mri_name << "mriArray" << i;
            eri_name << "eriArray" << i;
            sc_trace(fp, scheduler.mcuPool[i]->riArray[0].read(), mri_name.str());
            sc_trace(fp, scheduler.ecuPool[i]->riArray[0].read(), eri_name.str());
            mri_name.str("");
            eri_name.str("");
    }

    std::stringstream mIOOutname, eIOOutname, eIOOutnameI;
    for(int i = 0; i < HCU_NUM; i ++) {
        mIOOutname << "mIODisOut" << i;
        eIOOutname << "eIODisOut" << i;
        eIOOutnameI << "eIODisOutInitializedRi" << i;
        sc_trace(fp, scheduler.mcuIODisPatcherPool[i]->ri_out[0].read(), mIOOutname.str());
        sc_trace(fp, scheduler.ecuIODisPatcherPool[i]->ri_out[0].read(), eIOOutname.str());
        sc_trace(fp, scheduler.ecuIODisPatcherPool[i]->ecu_ri_out.read(), eIOOutnameI.str());
        mIOOutname.str("");
        eIOOutname.str("");
        eIOOutnameI.str("");
    }

    std::ostringstream mname, ename;
    for(int i = 0; i < HCU_NUM; i ++) {
        mname << "MCU" << i << "out";
        ename << "ECU" << i << "out";
        sc_trace(fp, scheduler.mcuPool[i]->regBiggerScore[0].read(), mname.str());
        sc_trace(fp, scheduler.ecuPool[i]->regBiggerScore[0].read(), ename.str());
        mname.str("");
        ename.str("");
    }

    std::stringstream mIOInname, eIOInname;
    for(int i = 0; i < HCU_NUM; i ++) {
        mIOInname << "mIODisIn" << i;
        eIOInname << "eIODisin" << i;
        sc_trace(fp, scheduler.mcuIODisPatcherPool[i]->ri[0].read(), mIOInname.str());
        sc_trace(fp, scheduler.ecuIODisPatcherPool[i]->ri[0].read(), eIOInname.str());
        mIOInname.str("");
        eIOInname.str("");
    }
    
    //sc_trace(fp, scheduler.mcuIODisPatcherPool[0]->en.read(), "en");    std::cout's time is 1cycle later from simulation result
    //sc_start(20000, SC_NS); 
    sc_start(290000, SC_NS); 
    sc_close_vcd_trace_file(fp); 
    return 0;
}
