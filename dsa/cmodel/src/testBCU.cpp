#include <systemc.h>
#include "baseline.h"
#include "inputgenerator.h"

/*BCU Module*/
int sc_main(int argc, char* argv[]) {
    sc_clock clk("clk", 10, SC_NS);
    sc_signal<bool> rst;

    sc_signal<sc_uint<32> > ri[WIDTH];
    sc_signal<sc_uint<32> > qi[WIDTH];
    sc_signal<sc_uint<32> > w[WIDTH];
    sc_signal<sc_uint<32> > result;

    sc_trace_file *fp;   // Create VCD file
    fp = sc_create_vcd_trace_file("wave");   // open(fp), create wave.vcd file
    fp->set_time_unit(1, SC_NS);  // set tracing resolution to ns

    InputGenerator ing("InputGenerator");    
    ing.clk(clk);
    ing.rst(rst);
    for(int i = 0; i < WIDTH; i ++) {
        ing.ri_out[i](ri[i]);
        ing.qi_out[i](qi[i]);
        ing.w_out[i](w[i]);
    }

    BCU bcu("BCU"); 
    bcu.clk(clk);
    bcu.rst(rst);
    for(int i = 0; i < WIDTH; i ++) {
        bcu.riArray[i](ri[i]);
        bcu.qiArray[i](qi[i]);
    }
    bcu.W(w[0]);  // span remain same.
    bcu.Hout(result);

    sc_trace(fp, clk, "clk");
    sc_trace(fp, rst, "rst");
    sc_trace(fp, result, "result");
    sc_trace(fp, bcu.W, "span");
    for(int i = 0; i < WIDTH; i ++) {
        std::ostringstream riName;
        riName << "riArray-" << i;
        sc_trace(fp, bcu.riArray[i], riName.str());
        riName.str("");
    }
     
    for(int i = 0; i < WIDTH; i ++) {
        std::ostringstream qiName;
        qiName << "riArray-" << i;
        sc_trace(fp, bcu.qiArray[i], qiName.str());
        qiName.str("");
    }

    sc_trace(fp, bcu.Hout, "Hout");

    sc_start(1000, SC_NS); 
    sc_close_vcd_trace_file(fp); 
    return 0;
}
