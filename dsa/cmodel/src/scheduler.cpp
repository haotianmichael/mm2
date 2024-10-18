#include <cassert>
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
           int segStart = 0, tmpSegLongNum = 0,tmpSegShortNum = 0, i = 0;
           riSegment newRi;
           qiSegment newQi;
           wSegment newW;
           //FIXME: there are still a few UpperBound situations which is not covered here. 
           for(; i < anchorNum.read(); i ++) {
               assert(anchorSuccessiveRange[i].read() == 0 && "Error: successiveRange cannot be -1.");
               if(anchorSuccessiveRange[i].read() != 1
                   && anchorSuccessiveRange[i].read() != -1) {
                   newRi.data[segStart] = anchorRi[i].read();
                   newQi.data[segStart] = anchorQi[i].read();
                   newW.data[segStart] = anchorW[i].read();
                   segStart++;
                }else if(anchorSuccessiveRange[i].read() == 1){   // range = 1 means segments ends with this anchor 
                   // ending anchor still added
                   newRi.data[segStart] = anchorRi[i].read();
                   newQi.data[segStart] = anchorQi[i].read();
                   newW.data[segStart] = anchorW[i].read();
                   segStart++;

                   newRi.upperBound = segStart;
                   newQi.upperBound = segStart;
                   newW.upperBound = segStart;
                   if(newW.upperBound <= InputLaneWIDTH) {
                        riSegsShort.write(newRi);
                        qiSegsShort.write(newQi);
                        wSegsShort.write(newW);
                        tmpSegShortNum++;
                   }else {
                        riSegsLong.write(newRi);
                        qiSegsLong.write(newQi);
                        wSegsLong.write(newW);
                        tmpSegLongNum++;
                   }
                   segStart = 0;
                }
           } 
           /* 
           //the last segment
           // base on the RCUnit's algirighm, UpperBound of last anchor is always 0,
           // UpperBound of penultimate anchor is 1 if gap(last, penu)<5000, so they are in one segment.
           // UpperBound of penultimate anchor is 0 if gap(last, penu)>5000, so they are in two segments.
           if(i == anchorNum.read()) {
               newRi.upperBound = segStart;
               newQi.upperBound = segStart;
               newW.upperBound = segStart;
               if(newW.upperBound <= InputLaneWIDTH) {
                    riSegsShort.write(newRi);
                    qiSegsShort.write(newQi);
                    wSegsShort.write(newW);
                    tmpSegShortNum++;
                }else {
                    riSegsLong.write(newRi);
                    qiSegsLong.write(newQi);
                    wSegsLong.write(newW);
                    tmpSegLongNum++;
                }
           }
           */
           segNumLong.write(static_cast<sc_int<WIDTH> >(tmpSegLongNum)); 
           segNumShort.write(static_cast<sc_int<WIDTH> >(tmpSegShortNum)); 
        }else {
           segNumLong.write(0);
           segNumShort.write(0);
        }
    }
}


void Scheduler::scheduler_allocate() {
    // 调度分配都是基于调度表, 把testHCU.cpp中的思路放到这里
    while(true) {
        wait();
        if(start.read()) {
            riSegment newRi;
            qiSegment newQi;
            wSegment newW; 
           
            newRi = riSegsLong.read();
            newQi = qiSegsLong.read();
            newW = wSegsLong.read();
            sc_int<WIDTH> upperbound = newW.upperBound;

        }else {

        }
    }
}


void Scheduler::scheduler_execute() {



}