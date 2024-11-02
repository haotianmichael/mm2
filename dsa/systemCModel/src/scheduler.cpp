#include <cassert>
#include "scheduler.h"

void Scheduler::scheduler_hcu_pre() {
    while (true) {
        wait();
        if (start.read()) {
            if (readIdx < ReadNumProcessedOneTime) {
                int segStart = 0, tmpSegLongNum = 0, tmpSegShortNum = 0, tmpSegID = 0;
                riSegment *newRi = new riSegment;
                qiSegment *newQi = new qiSegment;
                wSegment *newW = new wSegment;
                assert(readIdx < ReadNumProcessedOneTime && "Error: cutting operation exceeds the maximum");
                // FIXME: there are still a few UpperBound situations which is not covered here.
                for (int i = 0; i < anchorNum[readIdx].read(); i++) {
                    assert(anchorSuccessiveRange[readIdx][i].read() != 0 && "Error: successiveRange cannot be -1.");
                    if(anchorSuccessiveRange[readIdx][i].read() != 1 && anchorSuccessiveRange[readIdx][i].read() != -1) {
                        newRi->data[segStart] = anchorRi[readIdx][i].read();
                        newQi->data[segStart] = anchorQi[readIdx][i].read();
                        newW->data[segStart] = anchorW[readIdx][i].read();
                        segStart++;
                    }else if(anchorSuccessiveRange[readIdx][i].read() == 1 || i == anchorNum[readIdx].read()-1) {
                        // range = 1 means segments ends with this anchor
                        // ending anchor still added
                        newRi->data[segStart] = anchorRi[readIdx][i].read();
                        newQi->data[segStart] = anchorQi[readIdx][i].read();
                        newW->data[segStart] = anchorW[readIdx][i].read();
                        segStart++;

                        newRi->readID = readIdx;
                        if (segStart <= MCUInputLaneWIDTH){
                            newQi->segID = tmpSegID << 1 | 0; // shortSegments ends with 0
                        }else {
                            newQi->segID = tmpSegID << 1 | 1;
                        }
                        newW->upperBound = segStart;
                        if (newW->upperBound <= MCUInputLaneWIDTH){
                            riSegQueueShort.write(*newRi);
                            qiSegQueueShort.write(*newQi);
                            wSegQueueShort.write(*newW);
                            tmpSegShortNum++;
                        }else {
                            riSegQueueLong.write(*newRi);
                            qiSegQueueLong.write(*newQi);
                            wSegQueueLong.write(*newW);
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
                }*/

                segNumLong = tmpSegLongNum;
                segNumShort = tmpSegShortNum;
                readIdx++;
            }
        }else {
            segNumLong = 0;
            segNumShort = 0;
        }
    }
}

int fillTableOfLongSegments(std::mutex& mtx, ram_data *localRAM, int &ramIndex, std::list<SchedulerItem> &st, sc_fifo<riSegment> &riSegQueueLong, sc_fifo<qiSegment> &qiSegQueueLong, sc_fifo<wSegment> &wSegQueueLong, sc_int<WIDTH> &segNumLong) {

    riSegment *newRi = new riSegment;
    qiSegment *newQi = new qiSegment;
    wSegment *newW = new wSegment;
    *newRi = riSegQueueLong.read();
    *newQi = qiSegQueueLong.read();
    *newW = wSegQueueLong.read();
    segNumLong--;

    // store data in ram
    for (int i = 0; i < newW->upperBound; i++) {
        localRAM[ramIndex].data[i] = newRi->data[i];
        localRAM[ramIndex + 1].data[i] = newRi->data[i];
        localRAM[ramIndex + 2].data[i] = newRi->data[i];
    }
    ramIndex = ramIndex + 2;

    SchedulerItem sItem;
    sItem.issued = 0;
    sItem.readID = newRi->readID;
    sItem.segmentID = newQi->segID >> 1;
    sItem.UB = newW->upperBound;
    sItem.addr = static_cast<sc_int<WIDTH>>(ramIndex);

    assert(newW->upperBound >= 66 && "Error: LongQueue's upperbound cannot be less than 66");
    sItem.HCU_Total_NUM = 2 + (newW->upperBound - 66) / 65;
    SchedulerTime tl;
    for (int i = 0; i < sItem.HCU_Total_NUM; i++){
        tl.type = i == 0 ? 1 : 0; // mcu & ecu
        tl.SBase = i * 65;
        tl.LBase = newW->upperBound;
        tl.hcuID = -1;
        // allocation cycle:
        //  C(startTime)
        //  C(startTime + 65)
        //  C(startTime + 130) ....
        tl.start_duration = sc_time(i * 650, SC_NS);
    }
    // fill at real allocationTime
    sItem.startTime = sc_time(0, SC_NS);
    sItem.endTime = sc_time(0, SC_NS);
    sItem.TimeList.push_back(tl);
    {
        std::lock_guard<std::mutex> lock(mtx);
        st.push_back(sItem);
    }
    return 1;
}

int fillTableOfShortSegments(std::mutex& mtx, ram_data *localRAM, int &ramIndex, std::list<SchedulerItem> &st, sc_fifo<riSegment> &riSegQueueShort, sc_fifo<qiSegment> &qiSegQueueShort, sc_fifo<wSegment> &wSegQueueShort, sc_int<WIDTH> &segNumShort) {
    riSegment *newRi = new riSegment;
    qiSegment *newQi = new qiSegment;
    wSegment *newW = new wSegment;

    *newRi = riSegQueueShort.read();
    *newQi = qiSegQueueShort.read();
    *newW = wSegQueueShort.read();
    segNumShort--;
    // store data in ram
    for (int i = 0; i < newW->upperBound; i++){
        localRAM[ramIndex].data[i] = newRi->data[i];
        localRAM[ramIndex+1].data[i] = newRi->data[i];
        localRAM[ramIndex+2].data[i] = newRi->data[i];
    }
    ramIndex = ramIndex + 2;

    SchedulerItem sItem;
    sItem.issued = 0;
    sItem.readID = newRi->readID;
    sItem.segmentID = newQi->segID >> 1;
    sItem.UB = newW->upperBound;
    sItem.HCU_Total_NUM = 1;
    sItem.addr = static_cast<sc_int<WIDTH>>(ramIndex);


    SchedulerTime tl;
    tl.start_duration = sc_time(0, SC_NS);
    tl.hcuID = -1;
    tl.type = 1;
    tl.SBase = 0;
    tl.LBase = newW->upperBound;
    // fill at real allocationTime
    sItem.startTime = sc_time(0, SC_NS);
    sItem.endTime = sc_time(0, SC_NS);
    sItem.TimeList.push_back(tl);
    {
        std::lock_guard<std::mutex> lock(mtx);
        st.push_back(sItem);
    }
    //std::cout << st.size() << std::endl;
    return 1;
}

int countIdle(){
    int zeroCount = 5;
    return zeroCount;
}
void Scheduler::scheduler_hcu_fillTable(){
    while (true){
        wait();
        if (start.read()){
            // 加一些延迟，不然一次性都生成到调度表中了
            if(countIdle() <= IdleThreshLow){
                // little idle reduction -> allocate shortPort
                if(segNumShort > 0){
                    fillTableOfShortSegments(schedulerTable->mtx, localRAM, ramIndex, schedulerTable->schedulerItemList, riSegQueueShort, qiSegQueueShort, wSegQueueShort, segNumShort);
                }else if(segNumLong > 0){
                    fillTableOfLongSegments(schedulerTable->mtx, localRAM, ramIndex, schedulerTable->schedulerItemList, riSegQueueLong, qiSegQueueLong, wSegQueueLong, segNumLong);
                }
            }else{
                // enough idle reduction -> allocate longPort
                if(segNumLong > 0){
                    fillTableOfLongSegments(schedulerTable->mtx, localRAM, ramIndex, schedulerTable->schedulerItemList, riSegQueueLong, qiSegQueueLong, wSegQueueLong, segNumLong);
                }else if(segNumShort > 0){
                    fillTableOfShortSegments(schedulerTable->mtx, localRAM, ramIndex, schedulerTable->schedulerItemList, riSegQueueShort, qiSegQueueShort, wSegQueueShort, segNumShort);
                }
            }
        }
    }
}

int allocateHCU(MCU *mcuPool[HCU_NUM], ECU *ecuPool[HCU_NUM], mcuIODispatcher *mcuIODisPatcherPool[HCU_NUM], ecuIODispatcher *ecuIODisPatcherPool[HCU_NUM], const sc_int<WIDTH> &readID, const sc_int<WIDTH> &segID, const sc_time &startTime, const sc_time &endTime, const sc_int<WIDTH> &addr, SchedulerTime &item) {
    int allo = -1;
    bool isAlloc = false;
    for (int i = 0; i < HCU_NUM; i++) {
        if (mcuPool[i]->currentReadID.read() == -1 && ecuPool[i]->currentReadID.read() == -1) {
            // mcu
            mcuPool[i]->currentReadID.write(readID);
            mcuPool[i]->currentSegID.write(segID);
            mcuPool[i]->LowerBound.write(item.SBase);
            mcuPool[i]->UpperBound.write(item.LBase);
            mcuPool[i]->type.write(item.type);
            mcuPool[i]->executeTime.write(startTime);
            mcuPool[i]->freeTime.write(endTime);
            mcuPool[i]->addr.write(addr);
            mcuIODisPatcherPool[i]->LowerBound.write(mcuPool[i]->LowerBound);
            mcuIODisPatcherPool[i]->UpperBound.write(mcuPool[i]->UpperBound);
            // ecu
            ecuPool[i]->currentReadID.write(readID);
            ecuPool[i]->currentSegID.write(segID);
            ecuPool[i]->LowerBound.write(item.SBase);
            ecuPool[i]->UpperBound.write(item.LBase);
            ecuPool[i]->type.write(item.type);
            ecuPool[i]->executeTime.write(startTime);
            ecuPool[i]->freeTime.write(endTime);
            ecuPool[i]->addr.write(addr);
            ecuIODisPatcherPool[i]->LowerBound.write(ecuPool[i]->LowerBound);
            ecuIODisPatcherPool[i]->UpperBound.write(ecuPool[i]->UpperBound);

            item.hcuID = i;
            allo = i;
            isAlloc = true;
            break;
        }
    }
    // Core Algorightm: Replacement Policy
    if (!isAlloc){
        if (item.type){ // if mcu allocation failed, wait
            allo = -2;
        }else{ // if ecu allocation failed, replacing
            for (int i = 0; i < HCU_NUM; i++) {
                // if()  是否有刚启动不久的hcu（UB<=65）的，直接重新弄分配
                if (!mcuPool[i]->type.read() && !ecuPool[i]->type.read()){
                    // mcu
                    mcuPool[i]->currentReadID.write(readID);
                    mcuPool[i]->currentSegID.write(segID);
                    mcuPool[i]->LowerBound.write(item.SBase);
                    mcuPool[i]->UpperBound.write(item.LBase);
                    mcuPool[i]->type.write(item.type);
                    mcuPool[i]->executeTime.write(startTime);
                    mcuPool[i]->freeTime.write(endTime);
                    mcuPool[i]->addr.write(addr);
                    // ecu
                    ecuPool[i]->currentReadID.write(readID);
                    ecuPool[i]->currentSegID.write(segID);
                    ecuPool[i]->LowerBound.write(item.SBase);
                    ecuPool[i]->UpperBound.write(item.LBase);
                    ecuPool[i]->type.write(item.type);
                    ecuPool[i]->executeTime.write(startTime);
                    ecuPool[i]->freeTime.write(endTime);
                    ecuPool[i]->addr.write(addr);

                    item.hcuID = i;
                    allo = i;
                    break;
                }
            }
        }
    }
    return allo;
}
int freeHCU(MCU *mcuPool[HCU_NUM], ECU *ecuPool[HCU_NUM], mcuIODispatcher *mcuIODisPatcherPool[HCU_NUM], ecuIODispatcher *ecuIODisPatcherPool[HCU_NUM], SchedulerTime &item) {
    int i = item.hcuID;
    assert(i>=0 && "Error: hcuID must be positive!");
    if(mcuPool[i]->currentReadID.read() != -1 
        && ecuPool[i]->currentReadID.read() != -1
        && mcuPool[i]->freeTime.read() == sc_time_stamp() 
        && ecuPool[i]->freeTime.read() == sc_time_stamp()
        && mcuIODisPatcherPool[i]->en.read()
        && ecuIODisPatcherPool[i]->en.read()){ 
            mcuPool[i]->currentReadID.write(static_cast<sc_int<WIDTH>>(-1));
            mcuPool[i]->currentSegID.write(static_cast<sc_int<WIDTH>>(-1));
            mcuPool[i]->freeTime.write(sc_time(0, SC_NS));
            mcuPool[i]->executeTime.write(sc_time(0, SC_NS));
            mcuPool[i]->type.write(static_cast<bool>(0));
            mcuPool[i]->LowerBound.write(static_cast<sc_int<WIDTH>>(-1));
            mcuPool[i]->UpperBound.write(static_cast<sc_int<WIDTH>>(-1));
            mcuPool[i]->en.write(static_cast<bool>(0));
            mcuIODisPatcherPool[i]->LowerBound.write(static_cast<bool>(0));
            mcuIODisPatcherPool[i]->UpperBound.write(static_cast<bool>(0));
            mcuIODisPatcherPool[i]->en.write(static_cast<bool>(0));

            ecuPool[i]->currentReadID.write(static_cast<sc_int<WIDTH>>(-1));
            ecuPool[i]->currentSegID.write(static_cast<sc_int<WIDTH>>(-1));
            ecuPool[i]->freeTime.write(sc_time(0, SC_NS));
            ecuPool[i]->executeTime.write(sc_time(0, SC_NS));
            ecuPool[i]->type.write(static_cast<bool>(0));
            ecuPool[i]->LowerBound.write(static_cast<sc_int<WIDTH>>(-1));
            ecuPool[i]->UpperBound.write(static_cast<sc_int<WIDTH>>(-1));
            ecuPool[i]->en.write(static_cast<bool>(0));
            ecuIODisPatcherPool[i]->LowerBound.write(static_cast<bool>(0));
            ecuIODisPatcherPool[i]->UpperBound.write(static_cast<bool>(0));
            ecuIODisPatcherPool[i]->en.write(static_cast<bool>(0));
            item.hcuID = -1;
            return 1;
    }else if(mcuPool[i]->currentReadID.read() != -1
    && ecuPool[i]->currentReadID.read() != -1
    && mcuPool[i]->executeTime.read() - sc_time(10, SC_NS) == sc_time_stamp()     // en should be one cycle ahead of computing and IODispatcher 
    && ecuPool[i]->executeTime.read()  - sc_time(10, SC_NS) == sc_time_stamp()
    && !mcuIODisPatcherPool[i]->en.read()
    && !ecuIODisPatcherPool[i]->en.read()){    //enable all computing unit
        mcuPool[i]->en.write(static_cast<bool>(1));
        ecuPool[i]->en.write(static_cast<bool>(1));
        mcuIODisPatcherPool[i]->en.write(static_cast<bool>(1));
        ecuIODisPatcherPool[i]->en.write(static_cast<bool>(1));
        return 0;
    }else {
        return 0;
    }
}
void Scheduler::scheduler_hcu_allocate() {
    while (true) {
        wait();
        if(start.read()){
            for (auto it = schedulerTable->schedulerItemList.begin(); it != schedulerTable->schedulerItemList.end(); ){
                auto timeIt = it->TimeList.begin();
                if(it->issued){
                    // ecu allocation
                    sc_time st = sc_time_stamp();
                    int freeNum = 0;
                    bool isforward = true;
                    for (; timeIt != it->TimeList.end(); ++timeIt) {
                        // allocation cycle: C(startTime + 65)  C(startTime + 130) ...
                        if (timeIt->hcuID == -1 && st - it->startTime == timeIt->start_duration){
                            if (!(timeIt->hcuID = allocateHCU(mcuPool, ecuPool, mcuIODisPatcherPool, ecuIODisPatcherPool, it->readID, it->segmentID, it->startTime + sc_time(10, SC_NS), it->endTime, it->addr, *timeIt))){
                                assert(timeIt->hcuID == -1 && it->TimeList.size() > 1 && "Error: Cannot stop ecu allocation util its over!");
                            }
                        }else if(timeIt->hcuID >= 0){
                            int id = timeIt->hcuID;
                            if(freeHCU(mcuPool, ecuPool, mcuIODisPatcherPool, ecuIODisPatcherPool, *timeIt)) {
                                if(++freeNum == it->HCU_Total_NUM) {
                                    std::cout << "successfully Free mcu: " << id << " for Seg: " << it->segmentID << " at: " << sc_time_stamp() << std::endl;
                                    {
                                        std::lock_guard<std::mutex> lock(schedulerTable->mtx);
                                        it = schedulerTable->schedulerItemList.erase(it);
                                    }
                                    isforward = false;
                                    break;
                                }
                            }
                        }
                    }
                    if(isforward) {
                        ++it;
                    }
                }else{
                    // mcu allocation
                    it->startTime = sc_time_stamp();
                    it->endTime = sc_time_stamp() + sc_time((it->UB + 2) * 10, SC_NS); // endTime = startTime + (newW.upperbound + 1)
                    assert((timeIt->hcuID==-1 || timeIt->hcuID == -2) && "Error: mcu already allocated!");
                    assert(timeIt->type && " Error: mcu allocator cannot operates ecu!");
                    if ((timeIt->hcuID = allocateHCU(mcuPool, ecuPool, mcuIODisPatcherPool, ecuIODisPatcherPool, it->readID, it->segmentID, it->startTime + sc_time(20, SC_NS), it->endTime, it->addr, *timeIt)) == -2){
                        std::cout << "mcu allocation failed, wait for some time..." << std::endl;
                        wait(40, SC_NS);
                        continue;
                    }
                    assert(timeIt->hcuID>=0  && "Error: mcu allocator return the wrong value!");
                    std::cout << "successfully Allocate mcu: " << timeIt->hcuID <<" for Segment: " << it->segmentID << " -----UB: " << it->UB << " startTime: " << it->startTime+sc_time(20, SC_NS) << " endTime: "  << it->endTime << std::endl;
                    it->issued = 1;
                    if (it->HCU_Total_NUM > 1){
                        it--; // finish mcu, then continue allocate ecu of one segment.
                    }
                    it++;
                }
            }
        }
    }
}

void Scheduler::scheduler_hcu_execute(){
    while (true){
        wait();
        if (start.read()){
            for (int i = 0; i < HCU_NUM; i++){

                MCU *mt = mcuPool[i];
                ECU *et = ecuPool[i];
                //std::cout << sc_time_stamp() << " vs " << mt->executeTime.read() << mt->currentReadID.read() << " " << mt->currentSegID.read() << std::endl;
                if (mt->currentReadID.read()!=-1 // one cycle after hcu allocation
                    && mt->currentSegID.read()!=-1 
                    && sc_time_stamp() == mt->executeTime.read() - sc_time(10, SC_NS)){
                    //std::cout << "IO start at" << sc_time_stamp() << std::endl;
                    if (mt->type.read()){ // mcu

                        assert(mt->LowerBound.read()==0 && "mcu's LowerBound must be 0!");
                        for(int j = 0; j < mt->UpperBound.read(); j ++) {
                            mcuIODisPatcherPool[i]->ri[j].write(localRAM[mt->addr.read()].data[j]);
                            mcuIODisPatcherPool[i]->ri[j].write(localRAM[mt->addr.read() + 1].data[j]);
                            mcuIODisPatcherPool[i]->ri[j].write(localRAM[mt->addr.read() + 2].data[j]);
                        }

                    }else{ // ecu

                        assert(et->LowerBound.read()!=0 && "ecu's LowerBound must not be 0!");
                        for(int j = 0; j < et->UpperBound.read(); j ++) {
                            ecuIODisPatcherPool[i]->ri[j].write(localRAM[et->addr.read()].data[j]);
                            ecuIODisPatcherPool[i]->qi[j].write(localRAM[et->addr.read() + 1].data[j]);
                            ecuIODisPatcherPool[i]->w[j].write(localRAM[et->addr.read() + 2].data[j]);
                        }

                    }
                }
            }
        }
    }
}

/* 查表，如果有满足条件的直接分配RT
void Scheduler::scheduler_rt_checkTable(){
}
*/
