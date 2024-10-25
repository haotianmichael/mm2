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
            tl.type = i == 0 ? 1 : 0;  //mcu & ecu
            tl.SBase = i * 65;
            tl.LBase = newW.upperBound;
            tl.hcuID = -1;
             // allocation cycle:
             //  C(startTime)
             //  C(startTime + 65)
             //  C(startTime + 130) ....
            tl.start_duration= sc_time(i * 650, SC_NS);    
        }
    }
    // fill at real allocationTime
    sItem.startTime  = sc_time(0, SC_NS); 
    sItem.endTime = sc_time(0, SC_NS);
    sItem.TimeList.push_back(tl);
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
    tl.start_duration= sc_time(0, SC_NS);
    tl.hcuID = -1; 
    tl.type = 1;
    tl.SBase = 0;
    tl.LBase = newW.upperBound;
    // fill at real allocationTime
    sItem.startTime = sc_time(0, SC_NS);  
    sItem.endTime = sc_time(0, SC_NS);
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



int allocateHCU(HCU *hcuPool[HCU_NUM], const sc_int<WIDTH>& readID, const sc_int<WIDTH>& segID, const sc_time &startTime, const sc_time &endTime, SchedulerTime &item) {
    
    int allo = -1;
    for(int i = 0; i < HCU_NUM; i ++) {
        if(hcuPool[i]->currentReadID.read() == -1) {
            hcuPool[i]->currentReadID.write(readID);
            hcuPool[i]->currentSegID.write(segID);
            hcuPool[i]->LowerBound.write(item.SBase);
            hcuPool[i]->UpperBound.write(item.LBase);
            hcuPool[i]->type.write(item.type);
            hcuPool[i]->executeTime(startTime);
            hcuPool[i]->freeTime.write(endTime);
            item.hcuID = i;
            allo = i;
            break;
        }
    }
    // Core Algorightm: Replacement Policy
    if(allo == HCU_NUM) {
        if(item.type) {  // if mcu allocation failed, wait
            allo = -2;   
        }else {    // if ecu allocation failed, replacing
            for(int i = 0; i < HCU_NUM; i ++) {
                //if()  是否有刚启动不久的hcu（UB<=65）的，直接重新弄分配
                if(!hcuPool[i]->type.read()) {
                    hcuPool[i]->currentReadID.write(readID);
                    hcuPool[i]->currentSegID.write(segID);
                    hcuPool[i]->LowerBound.write(item.SBase);
                    hcuPool[i]->UpperBound.write(item.LBase);
                    hcuPool[i]->type.write(item.type);
                    hcuPool[i]->executeTime(startTime);
                    hcuPool[i]->freeTime.write(endTime);
                    item.hcuID = i;
                    allo = i;
                    break;
                } 
            }
        }
    }
    return allo;
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
                   for(; timeIt != it->TimeList.rend(); ++timeIt) {
                        // allocation cycle: C(startTime + 65)  C(startTime + 130) ...
                        if(timeIt->hcuID == -1 && st - it->startTime == timeIt->start_duration) {
                            if(!(timeIt->hcuID = allocateHCU(hcuPool, it->readID, it->segmentID, it->startTime + sc_time(10, SC_NS), it->endTime, *timeIt))) {
                                assert(timeIt->hcuID == -1 && it->TimeList.size() > 1 && "Error: Cannot stop ecu allocation util its over!");
                            }
                        }
                    }
                }else {
                    // mcu allocation
                    it->startTime = sc_time_stamp();   
                    it->endTime = sc_time_stamp() + sc_time((it->UB + 1) * 10, SC_NS); // endTime = startTime + (newW.upperbound + 1)
                    assert(timeIt->hcuID && "Error: mcu already allocated!");
                    assert(!timeIt->type && " Error: mcu allocator cannot operates ecu!");
                    if((timeIt->hcuID = allocateHCU(hcuPool, it->readID, it->segmentID, it->startTime + sc_time(10, SC_NS), it->endTime, *timeIt)) == -2) {
                        std::cout << "mcu allocation failed, wait for some time..." << std::endl;
                        wait(40, SC_NS);
                    }
                    assert(timeIt->hcuID == -1&& " Error: mcu allocator return the wrong value!");
                    it->issued = 1;
                    if(it->HCU_Total_NUM > 1) it--;   // finish mcu, then continue allocate ecu of one segment.
                }
            } 
        }
    }
}



void Scheduler::scheduler_hcu_execute() {
    while(true) {
        wait();
        if(start.read()) {
            for(int i = 0; i < HCU_NUM; i ++) {
                HCU *it = hcuPool[i]; 
                if(it->currentReadID.read() // one cycle after hcu allocation
                      && it->currentSegID 
                      && sc_time_stamp() == it->executeTime) {
                      sc_signal<sc_int<WIDTH> > ri[InputLaneWIDTH];
                      sc_signal<sc_int<WIDTH> > qi[InputLaneWIDTH];
                      sc_signal<sc_int<WIDTH> > w[InputLaneWIDTH];
                      assert(!it->type && "Error: it->type cannot be negative!");
                       if(it->type) {   // mcu
                            mcuIODispatcher ing("mcuIODispatcher");
                            ing.clk(clk);
                            ing.rst(rst);
                            assert(it->LowerBound && "mcu's LowerBound must be 0!");
                            ing.LowerBound(it->LowerBound);
                            ing.UpperBound(it->UpperBound);
                            for(int i = 0; i < InputLaneWIDTH; i ++) {
                                    ing.ri_out[i](ri[i]);
                                    ing.qi_out[i](qi[i]);
                                    ing.w_out[i](w[i]);
                            }
                            it->clk(clk);
                            it->rst(rst);
                            for(int j = 0; j < InputLaneWIDTH; j ++) {
                                it->riArray[j](ri[j]);
                                it->qiArray[j](qi[j]);
                                it->W[j](w[j]);
                            }
                       }else {   // ecu
                           ecuIODispatcher ing("ecuIODispatcher");
                           ing.clk(clk);
                           ing.rst(rst);
                           assert(!it->LowerBound && "ecu's LowerBound must not be 0!");
                           ing.LowerBound(it->LowerBound);
                           ing.UpperBound(it->UpperBound);
                           for(int i = 0; i < InputLaneWIDTH; i ++) {
                                   ing.ri_out[i](ri[i]);
                                   ing.qi_out[i](qi[i]);
                                   ing.w_out[i](w[i]);
                           }
                           it->clk(clk);
                           it->rst(rst);
                           for(int j = 0; j < InputLaneWIDTH; j ++) {
                               it->riArray[j](ri[j]);
                               it->qiArray[j](qi[j]);
                               it->W[j](w[j]);
                           }
                       }
                }else if(it->currentReadID 
                    && it->currentSegID 
                    && it->freeTime == sc_time_stamp()) {   // free hcu

                        it->currentReadID.write(sc_int<sc_int<WIDTH> >(-1));
                        it->currentSegID.write(sc_int<sc_int<WIDTH> >(-1));
                        it->freeTime.write(static_cast<sc_int<WIDTH> >(-1));
                        it->executeTime.write(static_cast<sc_int<WIDTH> >(-1));
                        it->type.write(static_cast<sc_int<WIDTH> >(-1));
                        it->LowerBound.write(static_cast<sc_int<WIDTH> >(-1));
                        it->UpperBound.write(static_cast<sc_int<WIDTH> >(-1));
                        for(int i = 0; i < InputLaneWIDTH; i ++) {
                                ri[i].write(static_cast<sc_int<WIDTH> >(-1));
                                qi[i].write(static_cast<sc_int<WIDTH> >(-1));
                                w[i].write(static_cast<sc_int<WIDTH> >(-1));
                        }
                }  
            }            
        }
    }
}


//查表，如果有满足条件的直接分配RT
void Scheduler::scheduler_rt_checkTable() {


}
