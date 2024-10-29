#ifndef HCU_H 
#define HCU_H

#include "hlane.h"

/*Hybrid Chaining Unit*/
SC_MODULE(HCU){

    sc_in<bool> clk, rst;
    sc_in<sc_int<WIDTH> > riArray[LaneWIDTH + 1];
    sc_in<sc_int<WIDTH> > qiArray[LaneWIDTH + 1];
    sc_in<sc_int<WIDTH> > W[LaneWIDTH + 1];

    // For Scheduler
    sc_signal<sc_int<WIDTH> > currentReadID;   
    sc_signal<sc_int<WIDTH> > currentSegID;
    sc_signal<sc_int<WIDTH> > LowerBound;
    sc_signal<sc_int<WIDTH> > UpperBound;
    sc_signal<sc_time> executeTime;
    sc_signal<sc_time>  freeTime;  
    sc_signal<sc_int<WIDTH> > addr;
    sc_signal<sc_bit> type;

    /* HCU has 65 Lane, 65 InputAnchor for mcu, 66 InputAnchor for ecu*/
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
                currentReadID.write(static_cast<sc_int<WIDTH> >(-1));
                currentSegID.write(static_cast<sc_int<WIDTH> >(-1));
                LowerBound.write(static_cast<sc_int<WIDTH> >(-1));
                UpperBound.write(static_cast<sc_int<WIDTH> >(-1));
                type.write(static_cast<sc_bit>(0));
                executeTime.write(sc_time(0, SC_NS));
                freeTime.write(sc_time(0, SC_NS));
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

    SC_CTOR(HCU) {

        SC_THREAD(monitor_sc_updates);
        for(int i =0; i < LaneWIDTH; i ++) {
           sensitive << hlane[i]->biggerScore;
        }

        SC_THREAD(updateRegBiggerScore);
        sensitive << score_updated;

        SC_THREAD(initializeRegBiggerScore);
    }
};

struct MCU : HCU {

    SC_CTOR(MCU) : HCU("MCU"){
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
    }

};


struct ECU : HCU {

    sc_in<sc_int<WIDTH> > ecu_ri;
    sc_in<sc_int<WIDTH> > ecu_qi;
    sc_in<sc_int<WIDTH> > ecu_W;

    SC_CTOR(ECU) : HCU("ECU") {
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
            hlane[i]->inputA.ri(ecu_ri);
            hlane[i]->inputA.qi(ecu_qi);
            hlane[i]->inputA.W(ecu_W);
            hlane[i]->inputB.ri(riArray[i]);
            hlane[i]->inputB.qi(qiArray[i]);
            hlane[i]->inputB.W(W[i]);

            hlane[i]->lastCmp(regBiggerScore[i+1]); 
            hlaneName.str("");
        }
    }

};

#endif