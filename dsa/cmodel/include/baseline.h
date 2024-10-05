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
        sc_signal<sc_uint<WIDTH> > tmpIn;
        for(int i = 0; i < RegFileNum; i ++) {
            // initialize inputArray into 0
            riXArray[i](tmpIn);
            qiXArray[i](tmpIn);
            riYArray[i](tmpIn);
            qiYArray[i](tmpIn);
            Hout(tmpIn);
            W(tmpIn);
            tmpIn.write(static_cast<sc_uint<WIDTH> >(-1));
            // initialize Hlane
            hlaneName << "HLane" << i;
            sc_signal<sc_int<WIDTH> > tmpI;
            sc_signal<sc_uint<WIDTH> > laneResult[RegFileNum];
            hlane[i] = new HLane(hlaneName.str().c_str());
            hlane[i]->clk(clk);
            hlane[i]->rst(rst);
            hlane[i]->id(tmpI);
            tmpI.write(static_cast<sc_int<32> >(i));
            // BCU Wiring
            hlane[i]->inputA->ri(riXArray[RegFileNum]);
            hlane[i]->inputA->qi(qiXArray[RegFileNum]);
            hlane[i]->inputA->W(W);
            hlane[i]->inputB->ri(riYArray[i]);
            hlane[i]->inputB->qi(qiYArray[i]);
            hlane[i]->inputB->W(W);
            sc_signal<sc_uint<32> > constLastCmp;
            if(i == 0) {
                hlane[i]->lastCmp(constLastCmp); 
                constLastCmp.write(static_cast<sc_uint<32> >(-1));
            }else {
                hlane[i]->lastCmp(hlane[i-1]->biggerScore);
            }
            // BCU's out every cycle
            if(i == 63) {
                Hout(hlane[i]->biggerScore); 
            }
        }
      
    }

};

#endif