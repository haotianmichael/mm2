#ifndef HCU_H
#define HCU_H

#include <sstream>
#include <string.h>
#include "sc.h"

#define LaneWIDTH  64

/*Anchor (do only within one read)*/
SC_MODULE(Anchor) {

    sc_in<sc_uint<WIDTH> > ri, qi;
    sc_in<sc_uint<WIDTH> > W;
    // seg_id when come to inter-read.
    SC_CTOR(Anchor) {}
};

/*Comparatot*/
SC_MODULE(Comparator) {

    sc_in<bool> clk, rst;
    sc_in<sc_uint<WIDTH> > cmpA, cmpB;
    sc_out<sc_uint<WIDTH> > bigger;

    void compare(){
          while(true) {
             wait();
            if(rst.read()) {
                 bigger.write(0);
            }else {
                 bigger.write(cmpA.read() > cmpB.read() 
                    ? cmpA.read() : cmpB.read());
            }
        } 
    }
    SC_CTOR(Comparator) {
        SC_THREAD(compare);
        sensitive << clk.pos();
    } 
};

/*Computing Lane for One-Pair of Anchors*/
SC_MODULE(HLane) {
    
    sc_in<bool> clk, rst;
    sc_in<sc_int<32> > id;   // Id of each Lane within one HCU (1-65)
    sc_in<sc_uint<32> > lastCmp;  // input of this lane's  comparator
    sc_out<sc_uint<WIDTH> > biggerScore;  // output of this lane/input of next lane's comparator

    /*pipeline*/
    Anchor* inputA;  
    Anchor* inputB;
    ScCompute *compute;
    sc_signal<sc_uint<WIDTH> > computeResult; // result of ScCompute
    Comparator *comparator;

    void process();
    SC_CTOR(HLane) {
        
        inputA = new Anchor("inputA");
        inputB = new Anchor("inputB");

        compute = new ScCompute("compute");
        compute->clk(clk);
        compute->rst(rst);
        compute->riX(inputA->ri);
        compute->riY(inputB->ri);
        compute->qiX(inputA->qi);
        compute->qiY(inputB->qi);
        compute->W(inputA->W);   // inputA has the same span with inputB
        compute->W_avg(inputA->W);
        compute->result(computeResult);

        comparator = new Comparator("comparator");
        comparator->clk(clk);
        comparator->rst(rst);
        comparator->cmpA(computeResult);
        comparator->cmpB(lastCmp);
        comparator->bigger(biggerScore);

        SC_THREAD(process);
        sensitive << clk.pos();
    }
};



/*Hybrid Chaining Unit*/
SC_MODULE(HCU) {

    sc_in<bool> clk, rst;
    sc_in<sc_uint<WIDTH> > riArray[LaneWIDTH + 1];
    sc_in<sc_uint<WIDTH> > qiArray[LaneWIDTH + 1];
    sc_in<sc_uint<WIDTH> > W;
    sc_out<sc_uint<WIDTH> > Hout;

    /* HCU has 64 Lane, 65 InputAnchor*/
    HLane* hlane[LaneWIDTH];
    /* Registers for staging Lane's output for 1 cycle*/
    sc_signal<sc_uint<WIDTH> >  regBiggerScore[LaneWIDTH + 1];
    /*Constant Value*/
    sc_signal<sc_int<WIDTH> > tmpI;
    sc_signal<sc_uint<32> > constLastCmp;

    SC_CTOR(HCU){
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
            if(i == 0) {
                hlane[i]->lastCmp(constLastCmp); 
                constLastCmp.write(static_cast<sc_uint<32> >(-1));
            }else {
                /*
                @Simulation Spot:
                    biggerScore[i-1] and biggerScore[i] output at the same cycle, so biggerScore[i] use the value of biggerScore[i-1] which is in last 1 cycle.
                */
                hlane[i]->lastCmp(regBiggerScore[i-1]);
            }
            hlane[i]->biggerScore(regBiggerScore[i]);
            hlaneName.str("");
       }
       Hout(regBiggerScore[LaneWIDTH]);
    }
};

/*Main Chaining Unit for every anchor */
struct MCU : public HCU{

    void process();

    SC_CTOR(MCU) : HCU("HCU"){
        SC_THREAD(process);
        sensitive << clk.pos();
    }
};

/*Extensive Chaining Unit for every anchor whose range exceeds 65*/
struct ECU : public HCU{

    SC_CTOR(ECU) : HCU("HCU") {

    }
};

#endif