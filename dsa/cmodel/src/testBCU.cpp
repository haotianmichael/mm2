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
    sc_signal<sc_uint<32> > ri[InputLaneWIDTH];
    sc_signal<sc_uint<32> > qi[InputLaneWIDTH];
    sc_signal<sc_uint<32> > w[InputLaneWIDTH];
    sc_signal<sc_uint<32> > result;

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

    Hlane hlane("hlane");
    hlane.clk(clk);
    hlane.rst(rst);
    hlane.inputA->ri(ri[LaneWIDTH]);
    hlane.inputA->qi(qi[LaneWIDTH]);
    hlane.inputA->W(w[LaneWIDTH]);
    hlane.inputB->ri(ri[0]);
    hlane.inputB->qi(qi[0]);
    hlane.inputB->W(w[0]);
    hlane.computeResult(result);

    sc_trace(fp, clk, "clk");
    sc_trace(fp, rst, "rst");
    sc_trace(fp, hlane.compute->riX, "HriX");
    sc_trace(fp, hlane.compute->riY, "HriY");
    sc_trace(fp, hlane.compute->qiX, "HqiX");
    sc_trace(fp, hlane.compute->qiY, "qiY");
    sc_trace(fp, hlane.compute->result, "Hresult");

/*
    BCU bcu("BCU"); 
    bcu.clk(clk);
    bcu.rst(rst);
    for(int i = 0; i < InputLaneWIDTH; i ++) {
        bcu.riArray[i](ri[i]);
        bcu.qiArray[i](qi[i]);
    }
    bcu.W(w[0]);  // span remain same.
    bcu.Hout(result);

    sc_trace(fp, clk, "clk");
    sc_trace(fp, rst, "rst");
    sc_trace(fp, result, "result");
    sc_trace(fp, bcu.W, "span");
    sc_trace(fp, bcu.riArray[64], "riArray64");
    sc_trace(fp, bcu.riArray[63], "riArray63");
    sc_trace(fp, bcu.qiArray[64], "qiArray64");
    sc_trace(fp, bcu.qiArray[63], "qiArray63");

    sc_trace(fp, bcu.Hout, "Hout");
    */

    sc_start(1000, SC_NS); 
    sc_close_vcd_trace_file(fp); 
    return 0;
}
