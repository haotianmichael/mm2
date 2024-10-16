#ifndef HCU_H 
#define HCU_H

#include "hlane.h"

/*Hybrid Chaining Unit*/
SC_MODULE(HCU){

    sc_in<bool> clk, rst;
    sc_in<sc_int<WIDTH> > riArray[LaneWIDTH + 1];
    sc_in<sc_int<WIDTH> > qiArray[LaneWIDTH + 1];
    sc_in<sc_int<WIDTH> > W[LaneWIDTH + 1];

    /* HCU has 64 Lane, 65 InputAnchor*/
    HLane* hlane[LaneWIDTH];
    /* Registers for staging Lane's output for 1 cycle*/
    sc_signal<sc_int<WIDTH> >  regBiggerScore[LaneWIDTH + 1];
  
    /*Constant Value*/
    sc_signal<sc_int<WIDTH> > tmpI;
    sc_signal<sc_int<WIDTH> > constLastCmp;

    /*Driven signal*/
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

    SC_CTOR(HCU){
        std::ostringstream hlaneName;
        for(int i = 0; i < LaneWIDTH; i ++) {
            // initialize Hlane
            hlaneName << "HLane" << i;
            hlane[i] = new HLane(hlaneName.str().c_str());
            hlane[i]->clk(clk);
            hlane[i]->rst(rst);
            hlane[i]->id(tmpI);
            tmpI.write(static_cast<sc_int<WIDTH> >(i));
            // HCU Wiring
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