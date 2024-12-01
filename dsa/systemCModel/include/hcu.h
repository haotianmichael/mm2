#ifndef HCU_H 
#define HCU_H

#include "hlane.h"

/*Hybrid Chaining Unit*/
SC_MODULE(MCU) {

    sc_in<bool> clk, rst;
    sc_signal<bool> en;
    sc_in<sc_int<WIDTH> > riArray[LaneWIDTH + 1];
    sc_in<sc_int<WIDTH> > qiArray[LaneWIDTH + 1];
    sc_in<sc_int<WIDTH> > W[LaneWIDTH + 1];
    sc_in<sc_int<WIDTH> > Idx[LaneWIDTH + 1];

    // Wiring
    sc_signal<sc_int<WIDTH>> riArray_sig[LaneWIDTH+1];
    sc_signal<sc_int<WIDTH>> qiArray_sig[LaneWIDTH+1];
    sc_signal<sc_int<WIDTH>> W_sig[LaneWIDTH+1];
    sc_signal<sc_int<WIDTH>> Idx_sig[LaneWIDTH+1];

    // For Scheduler
    sc_signal<sc_int<WIDTH> > currentReadID;   
    sc_signal<sc_int<WIDTH> > currentSegID;
    sc_signal<sc_int<WIDTH> > LowerBound;
    sc_signal<sc_int<WIDTH> > UpperBound;
    sc_signal<sc_time> executeTime;
    sc_signal<sc_time>  freeTime;  
    sc_signal<sc_int<WIDTH> > addr;
    sc_signal<bool> type;

    /* HCU has 65 Lane, 65 InputAnchor for mcu, 66 InputAnchor for ecu*/
    HLane* hlane[LaneWIDTH];
    /* Registers for staging Lane's output for 1 cycle*/
    sc_signal<sc_int<WIDTH> >  regBiggerScore[LaneWIDTH + 1];
    /*predecessor*/
    sc_signal<sc_int<WIDTH> > predecessor[LaneWIDTH + 1];
  
    /*Constant Value*/
    sc_signal<sc_int<WIDTH> > tmpI;
    sc_signal<sc_int<WIDTH> > constLastCmp;

    void updateRegBiggerScoreAndPredecessor() {
        while(true) {
            wait();
            if(!rst.read()) {
                if(en.read()) {
                    for(int i = 0; i < LaneWIDTH; i ++){
                          int tmp = hlane[i]->computeResult.read() + regBiggerScore[0].read();
                          if(tmp >= regBiggerScore[i+1].read()) {
                                regBiggerScore[i].write(static_cast<sc_int<WIDTH>>(tmp));
                                predecessor[i].write(hlane[i]->index_top_out.read());
                          }else {
                                regBiggerScore[i].write(regBiggerScore[i+1].read());
                          }
                    }
                }else {
                    for(int i = 0; i < LaneWIDTH + 1; i ++) {
                       regBiggerScore[i].write(static_cast<sc_int<WIDTH> >(-1));
                       predecessor[i].write(static_cast<sc_int<WIDTH>>(-1));
                    }
                }
            }else if(rst.read()){
                for(int i = 0; i < LaneWIDTH + 1; i ++) {
                   regBiggerScore[i].write(static_cast<sc_int<WIDTH> >(-1));
                   predecessor[i].write(static_cast<sc_int<WIDTH>>(-1));
                }
            }
        }
    }
    
    void updateValue() {
        while(true) {
            wait();
            for(int i = 0; i < LaneWIDTH; i ++) {
                riArray_sig[i].write(riArray[i].read()); 
                qiArray_sig[i].write(qiArray[i].read());
                W_sig[i].write(W[i].read());
                Idx_sig[i].write(Idx[i].read());
            }
        }
    }

    SC_CTOR(MCU) {
        std::ostringstream hlaneName;
        for(int i = 0; i < LaneWIDTH; i ++) {

            // initialize Hlane
            hlaneName << "HLane" << i;
            hlane[i] = new HLane(hlaneName.str().c_str());
            hlane[i]->clk(clk);
            hlane[i]->rst(rst);
            hlane[i]->en(en);
            hlane[i]->id(tmpI);
            tmpI.write(static_cast<sc_int<WIDTH> >(i));
            // MCU Wiring
            hlane[i]->inputA_ri(riArray_sig[0]);
            hlane[i]->inputA_qi(qiArray_sig[0]);
            hlane[i]->inputA_w(W_sig[0]);
            hlane[i]->inputB_ri(riArray_sig[i+1]);
            hlane[i]->inputB_qi(qiArray_sig[i+1]);
            hlane[i]->inputB_w(W_sig[i+1]);
            hlane[i]->index_top(Idx_sig[0]);
            hlane[i]->index_self(Idx_sig[i+1]);
            hlaneName.str("");

        }


        SC_THREAD(updateRegBiggerScoreAndPredecessor);
        sensitive << clk.pos();

        SC_THREAD(updateValue);
        sensitive << clk.pos();

    }

};


SC_MODULE(ECU){

    sc_in<sc_int<WIDTH> > ecu_ri[LaneWIDTH+1];
    sc_in<sc_int<WIDTH> > ecu_qi[LaneWIDTH+1];
    sc_in<sc_int<WIDTH> > ecu_w[LaneWIDTH+1];
    sc_in<sc_int<WIDTH> > ecu_idx[LaneWIDTH+1];

    sc_in<bool> clk, rst;
    sc_signal<bool> en;
    sc_in<sc_int<WIDTH> > riArray[LaneWIDTH + 1];
    sc_in<sc_int<WIDTH> > qiArray[LaneWIDTH + 1];
    sc_in<sc_int<WIDTH> > W[LaneWIDTH + 1];
    sc_in<sc_int<WIDTH> > Idx[LaneWIDTH + 1];

    //Wiring
    sc_signal<sc_int<WIDTH>> riArray_sig[LaneWIDTH];
    sc_signal<sc_int<WIDTH>> qiArray_sig[LaneWIDTH];
    sc_signal<sc_int<WIDTH>> W_sig[LaneWIDTH];
    sc_signal<sc_int<WIDTH>> Idx_sig[LaneWIDTH];
    sc_signal<sc_int<WIDTH>> ecu_ri_sig[LaneWIDTH];
    sc_signal<sc_int<WIDTH>> ecu_qi_sig[LaneWIDTH];
    sc_signal<sc_int<WIDTH>> ecu_w_sig[LaneWIDTH];
    sc_signal<sc_int<WIDTH>> ecu_idx_sig[LaneWIDTH];

    // For Scheduler
    sc_signal<sc_int<WIDTH> > currentReadID;   
    sc_signal<sc_int<WIDTH> > currentSegID;
    sc_signal<sc_int<WIDTH> > LowerBound;
    sc_signal<sc_int<WIDTH> > UpperBound;
    sc_signal<sc_time> executeTime;
    sc_signal<sc_time>  freeTime;  
    sc_signal<sc_int<WIDTH> > addr;
    sc_signal<bool> type;

    /* HCU has 65 Lane, 65 InputAnchor for mcu, 66 InputAnchor for ecu*/
    HLane* hlane[LaneWIDTH];
    /* Registers for staging Lane's output for 1 cycle*/
    sc_signal<sc_int<WIDTH> >  regBiggerScore[LaneWIDTH + 1];
    /*predecessor*/
    sc_signal<sc_int<WIDTH> > predecessor[LaneWIDTH + 1];
  
    /*Constant Value*/
    sc_signal<sc_int<WIDTH> > tmpI;
    sc_signal<sc_int<WIDTH> > constLastCmp;

    void updateRegBiggerScoreAndPredecessor() {
        while(true) {
            wait();
            if(!rst.read()) {
                if(en.read()) {
                    for(int i = 0; i < LaneWIDTH; i ++){
                          int tmp = hlane[i]->computeResult.read() + regBiggerScore[0].read();
                          if(tmp >= regBiggerScore[i+1].read()) {
                                regBiggerScore[i].write(static_cast<sc_int<WIDTH>>(tmp));
                          }else {
                                regBiggerScore[i].write(regBiggerScore[i+1].read());
                          }
                    }
                }else {
                    for(int i = 0; i < LaneWIDTH; i ++) {
                        regBiggerScore[i].write(static_cast<sc_int<WIDTH>>(-1));
                    }
                }
            }else if(rst.read()){
                for(int i = 0; i < LaneWIDTH + 1; i ++) {
                   regBiggerScore[i].write(static_cast<sc_int<WIDTH> >(-1));
                }
            }
        }
    }
 
    void updateValue() {
        for(int i = 0; i < LaneWIDTH; i ++) {
            ecu_ri_sig[i].write(ecu_ri[i].read());
            ecu_qi_sig[i].write(ecu_qi[i].read());
            ecu_w_sig[i].write(ecu_w[i].read());
            riArray_sig[i].write(riArray[i].read()); 
            qiArray_sig[i].write(qiArray[i].read());
            W_sig[i].write(W[i].read());
            Idx_sig[i].write(Idx[i].read());
            ecu_idx_sig[i].write(ecu_idx[i].read());
        }
    }

    SC_CTOR(ECU){
        std::ostringstream hlaneName;
        for(int i = 0; i < LaneWIDTH; i ++) {
            // initialize Hlane
            hlaneName << "HLane" << i;
            hlane[i] = new HLane(hlaneName.str().c_str());
            hlane[i]->clk(clk);
            hlane[i]->rst(rst);
            hlane[i]->en(en);
            hlane[i]->id(tmpI);
            tmpI.write(static_cast<sc_int<WIDTH> >(i));
            // ECU Wiring
            hlane[i]->inputA_ri(ecu_ri_sig[i]);
            hlane[i]->inputA_qi(ecu_qi_sig[i]);
            hlane[i]->inputA_w(ecu_w_sig[i]);
            hlane[i]->inputB_ri(riArray_sig[i]);
            hlane[i]->inputB_qi(qiArray_sig[i]);
            hlane[i]->inputB_w(W_sig[i]);

            hlane[i]->index_top(ecu_idx_sig[0]);
            hlane[i]->index_self(ecu_idx_sig[i]);
            hlaneName.str("");
        }

        SC_THREAD(updateRegBiggerScoreAndPredecessor);
        sensitive << clk.pos();

        SC_THREAD(updateValue);
        sensitive << clk.pos();
        
    }

};

#endif