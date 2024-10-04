#ifndef BASELINE_H
#define BASELINE_H

#include <hcu.h>

/*Baseline of FCCM*/
struct BCU : public HCU{
    
    /*
        Input Module
        Compute Module
        Output Module 
    */
    void BCUInput();
    void BCUFlow();
    void BCUOutput();
    SC_CTOR(BCU) : HCU("HCU") {
        std::ostringstream hlaneName;
        for(int i = 0; i < RegFileNum; i ++) {
            hlaneName << "HLane" << i;
            sc_signal<sc_int<WIDTH> > tmpI;
            sc_signal<sc_uint<WIDTH> > laneResult[RegFileNum];
            hlane[i] = new HLane(hlaneName.str().c_str());
            hlane[i]->clk(clk);
            hlane[i]->rst(rst);
            hlane[i]->id(tmpI);
            tmpI.write(static_cast<sc_int<32> >(i));
        }

        SC_THREAD(BCUInput);
        sensitive << clk.pos();
        SC_THREAD(BCUFlow);
        sensitive << clk.pos();
        SC_THREAD(BCUOutput);
        sensitive << clk.pos();
    }

};

#endif