#include "baseline.h"

/*
    SystemC Feature: 
    the difference of binding...() and write()/read()
    the difference of sc_in/sc_out and sc_signal

*/
void BCU::process() {
    while(true) {
        wait();
        if(rst.read()){
            for(int i =0; i < LaneWIDTH; i ++) {
                regBiggerScore->write(constLastCmp);
                constLastCmp.write(static_cast<sc_uint<32> >(-1));
            }
        }else {
            for(int i = 0; i < LaneWIDTH; i ++) {
                hlane[i]->lastCmp.write(regBiggerScore[i]);
                if(i > 0) {
                    regBiggerScore[i].write(hlane[i-1]->biggerScore);
                }
            }
        }
    }
}