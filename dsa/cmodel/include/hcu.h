#ifndef HCU_H
#define HCU_H

#include <sstream>
#include <string.h>
#include "sc.h"
#include "inputgenerator.h"

#define RegFileNum 64

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
    sc_signal<sc_uint<WIDTH> > computeResult; // result of ScCompute

    /*pipeline*/
    Anchor* inputA;  
    Anchor* inputB;
    ScCompute *compute;
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
    sc_in<sc_uint<WIDTH> > riXArray[RegFileNum + 1];
    sc_in<sc_uint<WIDTH> > riYArray[RegFileNum + 1];
    sc_in<sc_uint<WIDTH> > qiXArray[RegFileNum + 1];
    sc_in<sc_uint<WIDTH> > qiYArray[RegFileNum + 1];
    sc_in<sc_uint<WIDTH> > W;
    sc_out<sc_uint<WIDTH> > Hout;

    /* HCU has 64 Lane, 65 InputAnchor*/
    HLane* hlane[RegFileNum];
    //InputGenerator* inputGenerator; 


    SC_CTOR(HCU){}
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