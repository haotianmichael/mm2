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
                        riSegQueueShort.write(newRi);
                        qiSegQueueShort.write(newQi);
                        wSegQueueShort.write(newW);
                        tmpSegShortNum++;
                   }else {
                        riSegQueueLong.write(newRi);
                        qiSegQueueLong.write(newQi);
                        wSegQueueLong.write(newW);
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
                    riSegQueueShort.write(newRi);
                    qiSegQueueShort.write(newQi);
                    wSegQueueShort.write(newW);
                    tmpSegShortNum++;
                }else {
                    riSegQueueLong.write(newRi);
                    qiSegQueueLong.write(newQi);
                    wSegQueueLong.write(newW);
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
    // 调度树如何改变ROCC，调度器依据这个进行调度
    // 考虑调度周期
    while(true) {
        wait();
        if(start.read()) {
            riSegment newRi;
            qiSegment newQi;
            wSegment newW; 
            sc_int<WIDTH> upperbound;
            if(schedulerTable.ROCC <= IdleThreshLow) { 
                 // little idle reduction -> allocate shortPort
                if(segNumShort.read() > 0) {
                    newRi = riSegQueueShort.read();
                    newQi = qiSegQueueShort.read();
                    newW = wSegQueueShort.read();
                    upperbound = newW.upperBound; 
                    segNumShort.write(segNumShort.read() - 1);
                }else if(segNumLong.read() > 0){
                    newRi = riSegQueueLong.read();
                    newQi = qiSegQueueLong.read();
                    newW = wSegQueueLong.read();
                    upperbound = newW.upperBound; 
                    segNumLong.write(segNumLong.read() - 1);
                } else {

                }
            }else {
                // enough idle reduction -> allocate longPort
                if(segNumLong.read() > 0) {
                    newRi = riSegQueueLong.read();
                    newQi = qiSegQueueLong.read();
                    newW = wSegQueueLong.read();
                    upperbound = newW.upperBound;  
                    segNumLong.write(segNumLong.read() - 1);
                }else if(segNumShort.read() > 0){
                    newRi = riSegQueueShort.read();
                    newQi = qiSegQueueShort.read();
                    newW = wSegQueueShort.read();
                    upperbound = newW.upperBound; 
                    segNumShort.write(segNumShort.read() - 1);
                }else {
                    if(segNumLong.read() == 0 
                         && segNumShort.read() == 0) {
                         // allocation over 
                         std::cout << "allocation over!" << std::endl;
                    }
                }
            }       
        }else {

        }
    }
}


void Scheduler::scheduler_execute() {
    // 设计如何映射inputDispatch来实现MCU/ECU的执行
    //如何处理execute和allocate的并行执行


}