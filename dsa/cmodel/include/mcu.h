#ifndef MCU_H 
#define MCU_H

#include <hcu.h>

/*
    @Baseline of FCCM
    @Main Chaining Unit for every anchor 
*/
struct MCU : public HCU{
    
     /*Constant Value*/
    sc_signal<sc_int<WIDTH> > tmpI;
    sc_signal<sc_int<32> > constLastCmp;

    sc_event score_updated;

    void initializeRegBiggerScore() {
        while(true){
            wait();
            if(rst.read()) {
                for(int i = 0; i < LaneWIDTH + 1; i ++) {
                    regBiggerScore[i].write(static_cast<sc_int<WIDTH> >(-1));
                }
            }
        }
    }
    void monitor_sc_updates() {
        while(true) {
            wait();
            if(!rst.read()) {
                score_updated.notify();
            }
        }
    }

    void updateRegBiggerScore() {
        while(true) {
            wait();
            if(!rst.read()) {
                for(int i = 0; i < LaneWIDTH; i ++){
                      regBiggerScore[i].write(hlane[i]->biggerScore.read());
                }
            }
        }
    }

    SC_CTOR(MCU) : HCU("HCU") {
        std::ostringstream hlaneName;
        for(int i = 0; i < LaneWIDTH; i ++) {
            // initialize Hlane
            hlaneName << "HLane" << i;
            hlane[i] = new HLane(hlaneName.str().c_str());
            hlane[i]->clk(clk);
            hlane[i]->rst(rst);
            hlane[i]->id(tmpI);
            tmpI.write(static_cast<sc_int<32> >(i));
            // MCU Wiring
            hlane[i]->inputA.ri(riArray[0]);
            hlane[i]->inputA.qi(qiArray[0]);
            hlane[i]->inputA.W(W[0]);
            hlane[i]->inputB.ri(riArray[i+1]);
            hlane[i]->inputB.qi(qiArray[i+1]);
            hlane[i]->inputB.W(W[i+1]);

            hlane[i]->lastCmp(regBiggerScore[i+1]); 
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