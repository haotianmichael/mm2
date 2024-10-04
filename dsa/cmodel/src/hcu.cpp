#include "hcu.h"

void Comparator::compare() {
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

void HLane::process() {


}

void MCU::process() {


}