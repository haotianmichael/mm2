#include <cassert>
#include "scheduler.h"

void Scheduler::scheduler_hcu_pre() {
    while(true) {
        wait();
        if(readIdx >= ReadNumProcessedOneTime) {
            return;
        }
        if(start.read()) {
           int segStart = 0, tmpSegLongNum = 0,tmpSegShortNum = 0, tmpSegID = 0;
           riSegment newRi;
           qiSegment newQi;
           wSegment newW;
           assert(readIdx >= ReadNumProcessedOneTime && "Error: cutting operation exceeds the maximum");
           //FIXME: there are still a few UpperBound situations which is not covered here. 
           for(int i = 0; i < anchorNum[readIdx].read(); i ++) {
               assert(anchorSuccessiveRange[readIdx][i].read() == 0 && "Error: successiveRange cannot be -1.");
               if(anchorSuccessiveRange[readIdx][i].read() != 1
                   && anchorSuccessiveRange[readIdx][i].read() != -1) {
                   newRi.data[segStart] = anchorRi[readIdx][i].read();
                   newQi.data[segStart] = anchorQi[readIdx][i].read();
                   newW.data[segStart] = anchorW[readIdx][i].read();
                   segStart++;
               }else if(anchorSuccessiveRange[readIdx][i].read() == 1){   // range = 1 means segments ends with this anchor 
                   // ending anchor still added
                   newRi.data[segStart] = anchorRi[readIdx][i].read();
                   newQi.data[segStart] = anchorQi[readIdx][i].read();
                   newW.data[segStart] = anchorW[readIdx][i].read();
                   segStart++;

                   newRi.readID = readIdx;  
                   if(segStart <= InputLaneWIDTH) {
                           newQi.segID = tmpSegID << 1 | 0;   // shortSegments ends with 0
                   }else {
                           newQi.segID = tmpSegID << 1 | 1;
                   }
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
                   tmpSegID++;
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
           readIdx++;
        }else {
           segNumLong.write(0);
           segNumShort.write(0);
        }
    }
}



int fillTableOfLongSegments(std::list<SchedulerItem>& st, sc_fifo<riSegment>& riSegQueueLong, sc_fifo<qiSegment>& qiSegQueueLong, sc_fifo<wSegment>& wSegQueueLong, sc_signal<sc_int<WIDTH>>& segNumLong) {

    riSegment newRi;
    qiSegment newQi;
    wSegment newW; 
    if((!riSegQueueLong.nb_read(newRi)) || (qiSegQueueLong.nb_read(newQi)) || (wSegQueueLong.nb_read(newW))) {
        return -1;
    }
    sc_int<WIDTH> currentNum = segNumLong.read();
    segNumLong.write(currentNum - 1);

    SchedulerItem sItem;
    sItem.issued = 0;
    sItem.readID = newRi.readID;
    sItem.segmentID = newQi.segID >> 1;
    sItem.UB = newW.upperBound;
    assert(newW.upperBound < 66 && "Error: LongQueue's upperbound cannot be less than 66");
    sItem.HCU_Total_NUM = 2 + (newW.upperBound - 66) / 65;
    for(int i = 0; i < sItem.HCU_Total_NUM; i ++) {
        SchedulerTime tl;
        if(i = 0) {
            tl.type = 1;  //mcu
            tl.SBase = 0;
            tl.LBase = sItem.UB;
            tl.hcuID = -1;
            tl.cycle = sc_time(0, SC_NS);   // C1 means same time with startTime
        }else {
            tl.type = 0;  //ecu
            tl.SBase = i * 65;
            if(i == sItem.HCU_Total_NUM - 1) {
                tl.LBase = sItem.UB;
            }else {
                tl.LBase = (i + 1) * 65 - 1 ;
            }
            tl.hcuID = -1;
             //  C2 means 10ns after startTime;
             //  C67 means 660ns after startTime, C132, C197;
            tl.cycle = sc_time(((i - 1) * 65 + 1)*10, SC_NS);    
        }
        sItem.startTime  = sc_time(0, SC_NS);
        sItem.TimeList.push_back(tl);
    }
    st.push_back(sItem);
    return 1;
}

int fillTableOfShortSegments(std::list<SchedulerItem> &st, sc_fifo<riSegment> &riSegQueueShort, sc_fifo<qiSegment> &qiSegQueueShort, sc_fifo<wSegment>& wSegQueueShort, sc_signal<sc_int<WIDTH>>& segNumShort) {

    riSegment newRi;
    qiSegment newQi;
    wSegment newW; 
    if((!riSegQueueShort.nb_read(newRi)) || (qiSegQueueShort.nb_read(newQi)) || (wSegQueueShort.nb_read(newW))) {
        return -1;
    }
    sc_int<WIDTH> currentNum = segNumShort.read();
    segNumShort.write(currentNum - 1);

    SchedulerItem sItem;
    sItem.issued = 0;
    sItem.readID = newRi.readID;
    sItem.segmentID = newQi.segID >> 1;
    sItem.UB = newW.upperBound;
    sItem.HCU_Total_NUM = 1;
    SchedulerTime tl;
    tl.cycle = sc_time(0, SC_NS);;
    tl.hcuID = -1; 
    tl.type = 1;
    tl.SBase = 0;
    tl.LBase = newW.upperBound;
    sItem.startTime = sc_time(0, SC_NS);
    sItem.TimeList.push_back(tl);
    st.push_back(sItem);
    return 1;
}

int countIdle(const ReductionPool& rp) {
    int zeroCount = 0;
    return zeroCount;
}
void Scheduler::scheduler_hcu_fillTable() {
    while(true) {
        wait();
        if(start.read()) {
            riSegment newRi;
            qiSegment newQi;
            wSegment newW; 
            // 加一些延迟，不然一次性都生成到调度表中了
            if(countIdle(*reductionPool) <= IdleThreshLow) { 
                 // little idle reduction -> allocate shortPort
                if(segNumShort.read() > 0) {
                    fillTableOfShortSegments(schedulerTable.schedulerItemList, riSegQueueShort, qiSegQueueShort, wSegQueueShort, segNumShort);
                }else if(segNumLong.read() > 0) {
                    fillTableOfLongSegments(schedulerTable.schedulerItemList, riSegQueueLong, qiSegQueueLong, wSegQueueLong, segNumLong);
                } else {
                    if(segNumLong.read() == 0 
                         && segNumShort.read() == 0) {
                            return ;
                    }
                }
            }else {
                // enough idle reduction -> allocate longPort
                if(segNumLong.read() > 0) {
                    fillTableOfLongSegments(schedulerTable.schedulerItemList, riSegQueueLong, qiSegQueueLong, wSegQueueLong, segNumLong);
                }else if(segNumShort.read() > 0){
                    fillTableOfShortSegments(schedulerTable.schedulerItemList, riSegQueueShort, qiSegQueueShort, wSegQueueShort, segNumShort);
                }else {
                    if(segNumLong.read() == 0 
                         && segNumShort.read() == 0) {
                            return ;
                    }
                }
            }       
        }
    }
}



int allocateHCU(HCU *hcuPool[HCU_NUM], const sc_int<WIDTH>& readID, const sc_int<WIDTH>& segID, const sc_time &startTime, SchedulerTime &item) {
    
    int allo = -1;
    for(int i = 0; i < HCU_NUM; i ++) {
        if(hcuPool[i]->currentReadID.read() == -1) {
            hcuPool[i]->currentReadID.write(readID);
            hcuPool[i]->currentSegID.write(segID);
            hcuPool[i]->type.write(item.type);
            hcuPool[i]->startTime.write(startTime);
            item.hcuID = i;
            allo = i;
            break;
        }
    }
    // Core Algorightm: Replacement Policy
    if(allo == HCU_NUM) {
        if(!item.type) {
            allo = -2;   
        }else {
            for(int i = 0; i < HCU_NUM; i ++) {
                if(!hcuPool[i]->type.read()) {
                    hcuPool[i]->currentReadID.write(readID);
                    hcuPool[i]->currentSegID.write(segID);
                    hcuPool[i]->type.write(item.type);
                    hcuPool[i]->startTime.write(startTime);
                    item.hcuID = i;
                    allo = i;
                    break;
                } 
            }
        }
    }
    return allo;
}

int freeHCU(HCU *hcuPool[HCU_NUM], SchedulerItem &item) {
 
   // currentReadID不为-1以后判断是IODispatch还是释放
   bool isfreeAll = false;
   for(auto timeIt = item.TimeList.rbegin(); timeIt != item.TimeList.rend(); timeIt++) {


   }

   return isfreeAll; 
}

void Scheduler::scheduler_hcu_allocate() {
    while(true) {
        wait();
        if(start.read()) {
            for(auto it = schedulerTable.schedulerItemList.rbegin(); it != schedulerTable.schedulerItemList.rend(); ++it) {

                auto timeIt = it->TimeList.rbegin();
                if(it->issued) {
                   // ecu allocation 
                   sc_time st = sc_time_stamp();
                   bool allAllocated = false;
                   for(; timeIt != it->TimeList.rend(); ++timeIt) {
                        if(timeIt->hcuID == -1) {
                            if(!(timeIt->hcuID = allocateHCU(hcuPool, it->readID, it->segmentID, st, *timeIt))) {
                                assert(timeIt->hcuID == -1 && it->TimeList.size() > 1 && "Error: Cannot stop ecu allocation util its over!");
                            }
                            if(!allAllocated) allAllocated = true;
                        }
                   }
                   if(!allAllocated) {
                       // all TimeLists are allocated. start free.
                       freeHCU(hcuPool, *it);
                  }
                }else {
                    // mcu allocation
                    it->startTime = sc_time_stamp();   
                    assert(timeIt->hcuID && "Error: mcu already allocated!");
                    assert(!timeIt->type && " Error: mcu allocator cannot operates ecu!");
                    if(!(timeIt->hcuID = allocateHCU(hcuPool, it->readID, it->segmentID, it->startTime, *timeIt))) {
                        std::cout << "mcu allocation failed, wait for some time..." << std::endl;
                        wait(40, SC_NS);
                    }
                    it->issued = 1;
                    if(it->HCU_Total_NUM > 1) it--;   // finish mcu, then continue allocate ecu of one segment.
                }
            } 
        }
    }
}



void Scheduler::scheduler_hcu_execute() {
    // 根据调度表中的SBase和LBase来进行HCU和IODis的连接.映射到哈希表中
    // 设计如何映射inputDispatch来实现MCU/ECU的执行
    while(true) {
        wait();
        if(start.read()) {
            for(int i = 0; i < InputLaneWIDTH; i ++) {
                HCU *it = hcuPool[i]; 
                // one cycle after hcu allocation
                if(it->currentReadID.read()) {


                    

                }
            }            

        }
    }
}


//查表，如果有满足条件的直接分配RT
void Scheduler::scheduler_rt_checkTable() {


}
