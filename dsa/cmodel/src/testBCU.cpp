#include <systemc.h>
#include "baseline.h"
#include "inputgenerator.h"

SC_MODULE(Tb) {

    sc_in<bool> clk;
    sc_out<bool> reset;

    SC_CTOR(Tb) {
        SC_THREAD(reset_generator);
        sensitive << clk.neg();
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

/*BCU Module*/
int sc_main(int argc, char* argv[]) {
    sc_clock clk("clk", 10, SC_NS);
    sc_signal<bool> rst;
    sc_signal<sc_int<32> > ri[InputLaneWIDTH];
    sc_signal<sc_int<32> > qi[InputLaneWIDTH];
    sc_signal<sc_int<32> > w[InputLaneWIDTH];

    sc_trace_file *fp;   // Create VCD file
    fp = sc_create_vcd_trace_file("wave");   // open(fp), create wave.vcd file
    fp->set_time_unit(1, SC_NS);  // set tracing resolution to ns

    Tb tb("Tb");
    tb.clk(clk);
    tb.reset(rst);

    InputGenerator ing("InputGenerator");    
    ing.clk(clk);
    ing.rst(rst);
    for(int i = 0; i < InputLaneWIDTH; i ++) {
        ing.ri_out[i](ri[i]);
        ing.qi_out[i](qi[i]);
        ing.w_out[i](w[i]);
    }


    BCU bcu("BCU"); 
    bcu.clk(clk);
    bcu.rst(rst);
    for(int i = 0; i < InputLaneWIDTH; i ++) {
        bcu.riArray[i](ri[i]);
        bcu.qiArray[i](qi[i]);
        bcu.W[i](w[i]);
    }

   sc_trace(fp, clk, "clk");
   sc_trace(fp, rst, "rst");
   for(int i =0 ;  i < LaneWIDTH + 1; i ++) {
        std::ostringstream pe_name;
        pe_name << "riArray(" << i << ")";
        sc_trace(fp,bcu.riArray[i],pe_name.str());
        pe_name.str("");
    }
    for(int i =0 ;  i < LaneWIDTH; i ++) {
        std::ostringstream pe_name;
        pe_name << "result(" << i << ")";
        sc_trace(fp,bcu.hlane[i]->biggerScore,pe_name.str());
        pe_name.str("");
    }
    for(int i =0 ;  i < LaneWIDTH + 1; i ++) {
        std::ostringstream pe_name;
        pe_name << "regBiggerScore(" << i << ")";
        sc_trace(fp,bcu.regBiggerScore[i],pe_name.str());
        pe_name.str("");
    }
    sc_trace(fp, bcu.score_updated, "score_updated");


    sc_start(1000, SC_NS); 
    sc_close_vcd_trace_file(fp); 
    return 0;
}
