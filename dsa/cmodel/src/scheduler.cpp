#include "scheduler.h"


void Scheduler::scheduler_top() {
    while(true) {
        wait();
        if(rst.read()) {
            
        }else {


        }

    }
}


void Scheduler::scheduler_pre() {
    while(true) {
        wait();
        if(start.read()) {
           int segStart = 0, tmpSegNum = 0, i = 0;
           for(; i < anchorNum.read(); i ++) {
                riSegment newRi;
                qiSegment newQi;
                wSegment newW;
                if(anchorSuccessiveRange[i].read() != 0) {
                   newRi.data[segStart] = anchorRi[i].read();
                   newQi.data[segStart] = anchorQi[i].read();
                   newW.data[segStart] = anchorW[i].read();
                   segStart++;
                }else {
                    newRi.upperBound = segStart;
                    newQi.upperBound = segStart;
                    newW.upperBound = segStart;
                    tmpSegNum++;
                    segStart = 0;
                }
                // the last segments
                if(i == anchorNum.read()) {
                    newRi.upperBound = segStart;
                    newQi.upperBound = segStart;
                    newW.upperBound = segStart;
                }
           } 
           segNum.write(static_cast<sc_int<WIDTH> >(tmpSegNum)); 
        }else {
            segNum.write(0);
        }
    }
}


void Scheduler::scheduler_allocate() {
    while(true) {
        wait();
        if(start.read()) {
            riSegment newRi;
            qiSegment newQi;
            wSegment newW; 
            newRi = riSegs.read();
            newQi = qiSegs.read();
            newW = wSegs.read();
        }else {

        }
    }
}


void Scheduler::scheduler_execute() {



}