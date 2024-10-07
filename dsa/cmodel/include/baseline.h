#ifndef BASELINE_H
#define BASELINE_H

#include <hcu.h>

/*Baseline of FCCM*/
struct BCU : public HCU{
    
     /*Constant Value*/
    sc_signal<sc_int<WIDTH> > tmpI;
    sc_signal<sc_uint<32> > constLastCmp;

    void process();
    SC_CTOR(BCU) : HCU("HCU") {
        std::ostringstream hlaneName;
        for(int i = 0; i < LaneWIDTH; i ++) {
            // initialize Hlane
            hlaneName << "HLane" << i;
            hlane[i] = new HLane(hlaneName.str().c_str());
            hlane[i]->clk(clk);
            hlane[i]->rst(rst);
            hlane[i]->id(tmpI);
            tmpI.write(static_cast<sc_int<32> >(i));
            // BCU Wiring
            hlane[i]->inputA->ri(riArray[LaneWIDTH]);
            hlane[i]->inputA->qi(qiArray[LaneWIDTH]);
            hlane[i]->inputA->W(W);
            hlane[i]->inputB->ri(riArray[i]);
            hlane[i]->inputB->qi(qiArray[i]);
            hlane[i]->inputB->W(W);
            hlaneName.str("");
       }

       void process();
       sensitive << clk.pos();
    }
};

#endif