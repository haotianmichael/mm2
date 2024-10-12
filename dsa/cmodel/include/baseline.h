#ifndef BASELINE_H
#define BASELINE_H

#include <hcu.h>

/*Baseline of FCCM*/
struct BCU : public HCU{
    
     /*Constant Value*/
    sc_signal<sc_int<WIDTH> > tmpI;
    sc_signal<sc_uint<32> > constLastCmp;

    sc_event ri_qi_updated;

    void monitor_ri_qi_updates() {
        while(true) {
            wait();
            ri_qi_updated.notify();
        }
    }

    void updateRegBiggerScore() {
        for(int i = 0; i < LaneWIDTH - 1; i ++){
            regBiggerScore[i + 1] = hlane[i]->biggerScore.read();
        }
    }

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
            hlane[i]->inputA.ri(riArray[LaneWIDTH]);
            hlane[i]->inputA.qi(qiArray[LaneWIDTH]);
            hlane[i]->inputA.W(W[LaneWIDTH]);
            hlane[i]->inputB.ri(riArray[i]);
            hlane[i]->inputB.qi(qiArray[i]);
            hlane[i]->inputB.W(W[LaneWIDTH]);

            hlane[i]->lastCmp(regBiggerScore[i]); 
            if(i > 0) {
                hlane[i-1]->biggerScore(regBiggerScore[i]);
            }
            if(i == 63) {
                hlane[i]->biggerScore(regBiggerScore[i+1]);
            }
            hlaneName.str("");
        }

        SC_THREAD(monitor_ri_qi_updates);
        for(int i =0; i < LaneWIDTH; i ++) {
            sensitive << riArray[i] << qiArray[i];
        }

        void updateRegBiggerScore();
        SC_METHOD(updateRegBiggerScore);
        sensitive << ri_qi_updated;
    }
};

#endif