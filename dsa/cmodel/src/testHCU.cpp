#include <systemc.h>
#include "hcu.h"
#include "ioDispatcher.h"

SC_MODULE(Tb) {

    sc_in<bool> clk;
    sc_out<bool> reset;
    sc_out<sc_int<32> > UB;

    SC_CTOR(Tb) {
        SC_THREAD(reset_generator);
        sensitive << clk.neg();
    }

    void reset_generator() {
        reset.write(0);
        wait(5, SC_NS);
        reset.write(1);
        /*
            in2.txt: UpperBound  = 1695
            in.txt: UpperBound = 65
            1. change UpperBound
            2. change filename
        */
        UB.write(static_cast<sc_int<32> >(1659));
        wait(10, SC_NS);
        reset.write(0);
        wait(100, SC_NS);
    }
};

/*MCU Module*/
int sc_main(int argc, char* argv[]) {
    sc_clock clk("clk", 10, SC_NS);
    sc_signal<bool> rst;
    sc_signal<sc_int<32> > ri[InputLaneWIDTH];
    sc_signal<sc_int<32> > qi[InputLaneWIDTH];
    sc_signal<sc_int<32> > w[InputLaneWIDTH];
    sc_signal<sc_int<32> > tmpUB;

    sc_trace_file *fp;   // Create VCD file
    fp = sc_create_vcd_trace_file("wave");   // open(fp), create wave.vcd file
    fp->set_time_unit(1, SC_NS);  // set tracing resolution to ns

    Tb tb("Tb");
    tb.clk(clk);
    tb.reset(rst);
    tb.UB(tmpUB);

    mcuIODispatcher ing("mcuIODispatcher");    
    ing.clk(clk);
    ing.rst(rst);
    ing.UpperBound(tmpUB);
    for(int i = 0; i < InputLaneWIDTH; i ++) {
        ing.ri_out[i](ri[i]);
        ing.qi_out[i](qi[i]);
        ing.w_out[i](w[i]);
    }


    HCU hcu("HCU"); 
    hcu.clk(clk);
    hcu.rst(rst);
    for(int i = 0; i < InputLaneWIDTH; i ++) {
        hcu.riArray[i](ri[i]);
        hcu.qiArray[i](qi[i]);
        hcu.W[i](w[i]);
    }

   sc_trace(fp, clk, "clk");
   sc_trace(fp, rst, "rst");
   for(int i =0 ;  i < LaneWIDTH + 1; i ++) {
        std::ostringstream pe_name;
        pe_name << "riArray(" << i << ")";
        sc_trace(fp,hcu.riArray[i],pe_name.str());
        pe_name.str("");
    }
   for(int i =0 ;  i < LaneWIDTH + 1; i ++) {
        std::ostringstream pe_name;
        pe_name << "qiArray(" << i << ")";
        sc_trace(fp,hcu.qiArray[i],pe_name.str());
        pe_name.str("");
    }
    for(int i =0 ;  i < LaneWIDTH + 1; i ++) {
        std::ostringstream pe_name;
        pe_name << "regBiggerScore(" << i << ")";
        sc_trace(fp,hcu.regBiggerScore[i],pe_name.str());
        pe_name.str("");
    }
    for(int i =0 ;  i < LaneWIDTH; i ++) {
        std::ostringstream pe_name;
        pe_name << "result(" << i << ")";
        sc_trace(fp,hcu.hlane[i]->biggerScore,pe_name.str());
        pe_name.str("");
    }
    sc_trace(fp, hcu.score_updated, "score_updated");


    sc_start(20000, SC_NS); 
    sc_close_vcd_trace_file(fp); 
    return 0;
}
