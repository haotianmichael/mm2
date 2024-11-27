#include "chain.h"

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


    Chain chain("simChain");
    chain.clk(clk);
    chain.rst(rst);
    sc_trace(fp, clk, "clk");
    sc_trace(fp, rst, "rst");
    
    sc_trace(fp, chain.start.read(), "cutDone");
    std::ostringstream mri_name, eri_name;
    for(int i =0; i < HCU_NUM; i ++) {
            mri_name << "MCU" << i << "Input";
            eri_name << "ECU" << i << "Input";
            sc_trace(fp, chain.mri[i][0].read(), mri_name.str());
            sc_trace(fp, chain.eri[i][0].read(), eri_name.str());
            mri_name.str("");
            eri_name.str("");
    }

    std::stringstream resultName;
    for(int i = 0; i < RESULT_NUM; i ++) {
        resultName << "result" << i;
        sc_trace(fp, chain.resultArray[i].read(), resultName.str());
        sc_trace(fp, chain.resultArray[i].read(), resultName.str());
        resultName.str("");
    }
    
    //sc_trace(fp, scheduler.mcuIODisPatcherPool[0]->en.read(), "en");    std::cout's time is 1cycle later from simulation result
    //sc_start(20000, SC_NS); 
    sc_start(11698000, SC_NS); 
    sc_close_vcd_trace_file(fp); 
    return 0;
}
