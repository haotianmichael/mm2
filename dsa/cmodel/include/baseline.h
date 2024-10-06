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
    SC_CTOR(BCU) : HCU("HCU") {
        std::ostringstream hlaneName;
        sc_signal<sc_int<WIDTH> > tmpI;
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
            if(i == 0) {
                sc_signal<sc_uint<32> > constLastCmp;
                hlane[i]->lastCmp(constLastCmp); 
                constLastCmp.write(static_cast<sc_uint<32> >(-1));
            }else {
                /*
                @Simulation Spot:
                    biggerScore[i-1] and biggerScore[i] output at the same cycle, so biggerScore[i] use the value of biggerScore[i-1] which is in last 1 cycle.
                */
                hlane[i]->lastCmp(biggerScore[i-1]);
            }
            hlane[i]->biggerScore(biggerScore[i]);
       }
       Hout(biggerScore[LaneWIDTH]);
    }

};

#endif