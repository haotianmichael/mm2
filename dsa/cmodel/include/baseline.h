#ifndef BASELINE_H
#define BASELINE_H

#include <hcu.h>

/*Baseline of FCCM*/
struct BCU : public HCU{
    
     /*Constant Value*/
    sc_signal<sc_int<WIDTH> > tmpI;
    sc_signal<sc_uint<32> > constLastCmp;

    sc_event score_updated;

    void initializeRegBiggerScore() {
        while(true){
            wait();
            if(rst.read()) {
                for(int i = 0; i < LaneWIDTH + 1; i ++) {
                    regBiggerScore[i].write(static_cast<sc_uint<WIDTH> >(-1));
                }
            }
        }
    }
    void monitor_sc_updates() {
        while(true) {
            wait();
            if(!rst.read()) {
                ri_qi_updated.notify();
            }
        }
    }

    void updateRegBiggerScore() {
        for(int i = 0; i < LaneWIDTH; i ++){
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
            hlane[i]->inputB.W(W[i]);

            hlane[i]->lastCmp(regBiggerScore[i]); 
            hlaneName.str("");
        }

        SC_THREAD(monitor_sc_updates);
        for(int i =0; i < LaneWIDTH; i ++) {
            sensitive << hlane[i]->biggerScore;
        }

        SC_THREAD(updateRegBiggerScore);
        sensitive << score_updated;

        SC_THREAD(initializeRegBiggerScore);
    }
};

#endif