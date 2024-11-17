#include <cassert>
#include "scheduler.h"

void Scheduler::scheduler_hcu_pre() {
    while (true) {
        wait();
        if (start.read()) {
            /*
            FIXME: 
            std::cout << sc_time_stamp() << std::endl;
            */
            if (readIdx < ReadNumProcessedOneTime) {
                int segStart = 0, tmpSegLongNum = 0, tmpSegShortNum = 0, tmpSegID = 0;
                riSegment *newRi = new riSegment;
                qiSegment *newQi = new qiSegment;
                wSegment *newW = new wSegment;
                assert(readIdx < ReadNumProcessedOneTime && "Error: cutting operation exceeds the maximum");
                // FIXME: there are still a few UpperBound situations which is not covered here.
                for(int i = 0; i < anchorNum[readIdx]->read(); i ++) {
                    assert(anchorSuccessiveRange[readIdx][i]->read() != 0 && "Error: successiveRange cannot be -1.");
                    if(anchorSuccessiveRange[readIdx][i]->read() != 1 && anchorSuccessiveRange[readIdx][i]->read() != -1) {
                        newRi->data[segStart] = anchorRi[readIdx][i]->read();
                        newQi->data[segStart] = anchorQi[readIdx][i]->read();
                        newW->data[segStart] = anchorW[readIdx][i]->read();
                        segStart++;
                    }else if(anchorSuccessiveRange[readIdx][i]->read() == 1 || i == anchorNum[readIdx]->read()-1) {
                        // range = 1 means segments ends with this anchor
                        // ending anchor still added
                        newRi->data[segStart] = anchorRi[readIdx][i]->read();
                        newQi->data[segStart] = anchorQi[readIdx][i]->read();
                        newW->data[segStart] = anchorW[readIdx][i]->read();
                        segStart++;

                        newRi->readID = readIdx;
                        if (segStart <= MCUInputLaneWIDTH){
                            newQi->segID = tmpSegID << 1 | 0; // shortSegments ends with 0
                        }else {
                            newQi->segID = tmpSegID << 1 | 1;
                        }
                        newW->upperBound = segStart;
                        if (newW->upperBound <= MCUInputLaneWIDTH){
                            while(riSegQueueShort.num_free() == 0) {
                                wait(space_available);
                            }
                            riSegQueueShort.write(*newRi);
                            qiSegQueueShort.write(*newQi);
                            wSegQueueShort.write(*newW);
                            tmpSegShortNum++;
                            data_available.notify();
                        }else {
                            while(riSegQueueLong.num_free() == 0) {
                                wait(space_available);
                            }
                            riSegQueueLong.write(*newRi);
                            qiSegQueueLong.write(*newQi);
                            wSegQueueLong.write(*newW);
                            tmpSegLongNum++;
                            data_available.notify();
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

                readIdx++;
            }
        }
    }
}


int allocateBlock(std::vector<bool> &freeList) {
    for(size_t i = 0; i < freeList.size(); i ++) {
        if(freeList[i]) {
            freeList[i] = false;
            return i;
        }
    }
    std::cerr << "Error: No free memory blocks available" << std::endl;
    return -1;
}
void releaseBlock(int index, std::vector<bool> &freeList) {
    assert(index >= 0 && index < freeList.size() && "Error: invalid Index");
    if(freeList[index]){
        std::cerr << "Error: Block already free!" << std::endl;
    }else {
        freeList[index] = true;
    }
}
int fillTableOfLongSegments(std::mutex& mtx, std::vector<ram_data> &localRAM, int &ramIndex, std::vector<bool> &freeList, std::list<SchedulerItem> &st, sc_fifo<riSegment> &riSegQueueLong, sc_fifo<qiSegment> &qiSegQueueLong, sc_fifo<wSegment> &wSegQueueLong, sc_event& space_available, sc_event& data_available) {

    riSegment *newRi = new riSegment;
    qiSegment *newQi = new qiSegment;
    wSegment *newW = new wSegment;
    while(!riSegQueueLong.num_available()) {
        wait(data_available);
    }
    *newRi = riSegQueueLong.read();
    *newQi = qiSegQueueLong.read();
    *newW = wSegQueueLong.read();
    space_available.notify();
    // store data in ram
    ramIndex = allocateBlock(freeList);
    for (int i = 0; i < newW->upperBound; i++) {
        localRAM[ramIndex].Rdata[i] = newRi->data[i];
        localRAM[ramIndex].Qdata[i] = newQi->data[i];
        localRAM[ramIndex].Wdata[i] = newW->data[i];
    }

    SchedulerItem sItem;
    sItem.issued = 0;
    sItem.readID = newRi->readID;
    sItem.segmentID = newQi->segID >> 1;
    sItem.UB = newW->upperBound;
    sItem.addr = static_cast<sc_int<WIDTH>>(ramIndex);

    assert(newW->upperBound >= 66 && "Error: LongQueue's upperbound cannot be less than 66");
    sItem.HCU_Total_NUM = 2 + (newW->upperBound - 66) / 65;
    for (int i = 0; i < sItem.HCU_Total_NUM; i++){
        SchedulerTime tl;
        tl.type = i == 0 ? 1 : 0; // mcu & ecu
        tl.SBase = i * 65;
        tl.LBase = newW->upperBound;
        tl.hcuID = -1;
        // allocation cycle:
        //  C(startTime)
        //  C(startTime + 65)
        //  C(startTime + 130) ....
        tl.start_duration = sc_time(i * 650, SC_NS);
        sItem.TimeList.push_back(tl);
    }
    // fill at real allocationTime
    sItem.startTime = sc_time(0, SC_NS);
    sItem.endTime = sc_time(0, SC_NS);
    {
        std::lock_guard<std::mutex> lock(mtx);
        st.push_back(sItem);
    }
    return 1;
}

int fillTableOfShortSegments(std::mutex& mtx, std::vector<ram_data> &localRAM, int &ramIndex, std::vector<bool> &freeList, std::list<SchedulerItem> &st, sc_fifo<riSegment> &riSegQueueShort, sc_fifo<qiSegment> &qiSegQueueShort, sc_fifo<wSegment> &wSegQueueShort, sc_event& space_available, sc_event& data_available) {
    riSegment *newRi = new riSegment;
    qiSegment *newQi = new qiSegment;
    wSegment *newW = new wSegment;

    while(!riSegQueueShort.num_available()) {
        wait(data_available);
    }
    *newRi = riSegQueueShort.read();
    *newQi = qiSegQueueShort.read();
    *newW = wSegQueueShort.read();
    space_available.notify();
    //std::cout << "fifo valid at " << sc_time_stamp() << std::endl;
    // store data in ram
    ramIndex = allocateBlock(freeList);
    for (int i = 0; i < newW->upperBound; i++){
        localRAM[ramIndex].Rdata[i] = newRi->data[i];
        localRAM[ramIndex].Qdata[i] = newQi->data[i];
        localRAM[ramIndex].Wdata[i] = newW->data[i];
    }

    SchedulerItem sItem;
    sItem.issued = 0;
    sItem.readID = newRi->readID;
    sItem.segmentID = newQi->segID >> 1;
    /*if(newW->upperBound == 11) {
        std::cout << "ri->data[0] from fifo is " <<  newRi->data[0] << " ri->data[last] is " << newRi->data[newW->upperBound - 1] << " segID: "<< sItem.segmentID << std::endl;
        std::cout << "ri->data[0] from RAM is " << localRAM[ramIndex].data[0] << std::endl;
    }*/
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

void countIdle(sc_in<sc_int<WIDTH>> ROCC[Reduction_KIND], bool& eR){
    for(int i = 0; i < Reduction_KIND; i++) {
        if(ROCC[i].read() == 0) {
            eR = false;
            return;
        }
    }
    eR = true;
}
void Scheduler::scheduler_hcu_fillTable(){
    while (true){
        wait();
        if (start.read()){
            bool enoughRed = true;
            countIdle(ROCC, enoughRed);
            /*allocate ShortSeg as soon as one kind of reductionTree is running out.*/
            if(!enoughRed){
                // little idle reduction -> allocate shortPort
                if(riSegQueueShort.num_available() > 0){
                    fillTableOfShortSegments(schedulerTable->mtx, localRAM, ramIndex, freeList, schedulerTable->schedulerItemList, riSegQueueShort, qiSegQueueShort, wSegQueueShort, space_available, data_available);
                }else if(riSegQueueLong.num_available() > 0){
                    fillTableOfLongSegments(schedulerTable->mtx, localRAM, ramIndex, freeList, schedulerTable->schedulerItemList, riSegQueueLong, qiSegQueueLong, wSegQueueLong, space_available, data_available);
                }
            }else{
                // enough idle reduction -> allocate longPort
                if(riSegQueueLong.num_available() > 0){
                    fillTableOfLongSegments(schedulerTable->mtx, localRAM, ramIndex, freeList, schedulerTable->schedulerItemList, riSegQueueLong, qiSegQueueLong, wSegQueueLong, space_available, data_available);
                }else if(riSegQueueShort.num_available() > 0){
                    fillTableOfShortSegments(schedulerTable->mtx, localRAM, ramIndex, freeList, schedulerTable->schedulerItemList, riSegQueueShort, qiSegQueueShort, wSegQueueShort, space_available, data_available);
                }
            }
        }
    }
}

void allocateHCU(MCU *mcuPool[HCU_NUM], ECU *ecuPool[HCU_NUM], mcuIODispatcher *mcuIODisPatcherPool[HCU_NUM], ecuIODispatcher *ecuIODisPatcherPool[HCU_NUM], const sc_int<WIDTH> &readID, const sc_int<WIDTH> &segID, const sc_time &startTime, const sc_time &endTime, const sc_int<WIDTH> &addr, SchedulerTime &item, sc_int<WIDTH> &alloParam) {
    bool isAlloc = false;
    for (int i = 0; i < HCU_NUM; i++) {
        if (mcuPool[i]->currentReadID.read() == -1 
        && ecuPool[i]->currentReadID.read() == -1
        && mcuPool[i]->UpperBound.read() == -1
        && ecuPool[i]->UpperBound.read() == -1) {
            // mcu
            mcuPool[i]->currentReadID.write(readID);
            mcuPool[i]->currentSegID.write(segID);
            mcuPool[i]->LowerBound.write(item.SBase);
            mcuPool[i]->UpperBound.write(item.LBase);
            mcuPool[i]->type.write(item.type);
            mcuPool[i]->executeTime.write(startTime);
            mcuPool[i]->freeTime.write(endTime);
            mcuPool[i]->addr.write(addr);
            mcuIODisPatcherPool[i]->LowerBound.write(item.SBase);
            mcuIODisPatcherPool[i]->UpperBound.write(item.LBase);
            // ecu
            ecuPool[i]->currentReadID.write(readID);
            ecuPool[i]->currentSegID.write(segID);
            ecuPool[i]->LowerBound.write(item.SBase);
            ecuPool[i]->UpperBound.write(item.LBase);
            ecuPool[i]->type.write(item.type);
            ecuPool[i]->executeTime.write(startTime);
            ecuPool[i]->freeTime.write(endTime);
            ecuPool[i]->addr.write(addr);
            ecuIODisPatcherPool[i]->LowerBound.write(item.SBase);
            ecuIODisPatcherPool[i]->UpperBound.write(item.LBase);

            item.hcuID = i;
            alloParam = i;
            isAlloc = true;
            break;
        }
    }
    // Core Algorightm: Replacement Policy
    if (!isAlloc){
        if (item.type){ // if mcu allocation failed, wait
            alloParam = -2;
        }else{ 
            /*  if ecu allocation failed, replacing or freeing
               @iterating for shortSeg(<=65), replacing directly
            */ 
            for (int i = 0; i < HCU_NUM; i++) {
                if (mcuPool[i]->type.read() 
                    && ecuPool[i]->type.read()
                    && (mcuPool[i]->LowerBound.read() == 0 && ecuPool[i]->LowerBound.read() == 0)
                    && (mcuPool[i]->UpperBound.read() <= MCUInputLaneWIDTH && ecuPool[i]->UpperBound.read() <= MCUInputLaneWIDTH)){
                    // mcu
                    mcuPool[i]->currentReadID.write(readID);
                    mcuPool[i]->currentSegID.write(segID);
                    mcuPool[i]->LowerBound.write(item.SBase);
                    mcuPool[i]->UpperBound.write(item.LBase);
                    mcuPool[i]->type.write(item.type);
                    mcuPool[i]->executeTime.write(startTime);
                    mcuPool[i]->freeTime.write(endTime);
                    mcuPool[i]->addr.write(addr);
                    mcuIODisPatcherPool[i]->LowerBound.write(item.SBase);
                    mcuIODisPatcherPool[i]->UpperBound.write(item.LBase);
                    // ecu
                    ecuPool[i]->currentReadID.write(readID);
                    ecuPool[i]->currentSegID.write(segID);
                    ecuPool[i]->LowerBound.write(item.SBase);
                    ecuPool[i]->UpperBound.write(item.LBase);
                    ecuPool[i]->type.write(item.type);
                    ecuPool[i]->executeTime.write(startTime);
                    ecuPool[i]->freeTime.write(endTime);
                    ecuPool[i]->addr.write(addr);
                    ecuIODisPatcherPool[i]->LowerBound.write(item.SBase);
                    ecuIODisPatcherPool[i]->UpperBound.write(item.LBase);

                    item.hcuID = i;
                    alloParam = i;
                    isAlloc = true;
                    std::cout << "Successfully Replacing hcu:" << i << " of seg:" << mcuPool[i]->currentSegID.read() << " for seg:"<< segID << std::endl;
                    break;
                }
            }
            /*
              @free all hcu of currentReadID+currentSegID
            */
            if(!isAlloc) {
                bool isFree = false;
                for(int i = 0; i < HCU_NUM; i ++) {
                    if((mcuPool[i]->currentReadID == readID && mcuPool[i]->currentSegID == segID)
                        && (ecuPool[i]->currentReadID == readID && ecuPool[i]->currentSegID == segID)) {

                        mcuPool[i]->currentReadID.write(static_cast<sc_int<WIDTH>>(-1));
                        mcuPool[i]->currentSegID.write(static_cast<sc_int<WIDTH>>(-1));
                        mcuPool[i]->freeTime.write(sc_time(0, SC_NS));
                        mcuPool[i]->executeTime.write(sc_time(0, SC_NS));
                        mcuPool[i]->type.write(static_cast<bool>(0));
                        mcuPool[i]->LowerBound.write(static_cast<sc_int<WIDTH>>(-1));
                        mcuPool[i]->UpperBound.write(static_cast<sc_int<WIDTH>>(-1));
                        mcuPool[i]->en.write(static_cast<bool>(0));
                        mcuIODisPatcherPool[i]->LowerBound.write(static_cast<sc_int<WIDTH> >(-1));
                        mcuIODisPatcherPool[i]->UpperBound.write(static_cast<sc_int<WIDTH> >(-1));
                        mcuIODisPatcherPool[i]->en.write(static_cast<bool>(0));

                        ecuPool[i]->currentReadID.write(static_cast<sc_int<WIDTH>>(-1));
                        ecuPool[i]->currentSegID.write(static_cast<sc_int<WIDTH>>(-1));
                        ecuPool[i]->freeTime.write(sc_time(0, SC_NS));
                        ecuPool[i]->executeTime.write(sc_time(0, SC_NS));
                        ecuPool[i]->type.write(static_cast<bool>(0));
                        ecuPool[i]->LowerBound.write(static_cast<sc_int<WIDTH>>(-1));
                        ecuPool[i]->UpperBound.write(static_cast<sc_int<WIDTH>>(-1));
                        ecuPool[i]->en.write(static_cast<bool>(0));
                        ecuIODisPatcherPool[i]->LowerBound.write(static_cast<sc_int<WIDTH> >(-1));
                        ecuIODisPatcherPool[i]->UpperBound.write(static_cast<sc_int<WIDTH> >(-1));
                        ecuIODisPatcherPool[i]->en.write(static_cast<bool>(0));
                        item.hcuID = -1;
                        isFree = true;
                    }
                }
                if(isFree) {
                    alloParam = -3;
                }else {
                    alloParam = -1;
                }
            }
        }
    }
    return;
}
void freeHCU(MCU *mcuPool[HCU_NUM], ECU *ecuPool[HCU_NUM], mcuIODispatcher *mcuIODisPatcherPool[HCU_NUM], ecuIODispatcher *ecuIODisPatcherPool[HCU_NUM], SchedulerTime &item, sc_int<WIDTH>& freeParam) {
    int i = item.hcuID;
    assert(i>=0 && "Error: hcuID must be positive!");
    if(mcuPool[i]->currentReadID.read() != -1 
        && ecuPool[i]->currentReadID.read() != -1
        && mcuPool[i]->freeTime.read() - sc_time(10, SC_NS)== sc_time_stamp() 
        && ecuPool[i]->freeTime.read() - sc_time(10, SC_NS)== sc_time_stamp()
        && (mcuIODisPatcherPool[i]->en.read()
        || ecuIODisPatcherPool[i]->en.read())){ 
            mcuPool[i]->currentReadID.write(static_cast<sc_int<WIDTH>>(-1));
            mcuPool[i]->currentSegID.write(static_cast<sc_int<WIDTH>>(-1));
            mcuPool[i]->freeTime.write(sc_time(0, SC_NS));
            mcuPool[i]->executeTime.write(sc_time(0, SC_NS));
            mcuPool[i]->type.write(static_cast<bool>(0));
            mcuPool[i]->LowerBound.write(static_cast<sc_int<WIDTH>>(-1));
            mcuPool[i]->UpperBound.write(static_cast<sc_int<WIDTH>>(-1));
            mcuPool[i]->en.write(static_cast<bool>(0));
            mcuIODisPatcherPool[i]->LowerBound.write(static_cast<sc_int<WIDTH> >(-1));
            mcuIODisPatcherPool[i]->UpperBound.write(static_cast<sc_int<WIDTH> >(-1));
            mcuIODisPatcherPool[i]->en.write(static_cast<bool>(0));

            ecuPool[i]->currentReadID.write(static_cast<sc_int<WIDTH>>(-1));
            ecuPool[i]->currentSegID.write(static_cast<sc_int<WIDTH>>(-1));
            ecuPool[i]->freeTime.write(sc_time(0, SC_NS));
            ecuPool[i]->executeTime.write(sc_time(0, SC_NS));
            ecuPool[i]->type.write(static_cast<bool>(0));
            ecuPool[i]->LowerBound.write(static_cast<sc_int<WIDTH>>(-1));
            ecuPool[i]->UpperBound.write(static_cast<sc_int<WIDTH>>(-1));
            ecuPool[i]->en.write(static_cast<bool>(0));
            ecuIODisPatcherPool[i]->LowerBound.write(static_cast<sc_int<WIDTH> >(-1));
            ecuIODisPatcherPool[i]->UpperBound.write(static_cast<sc_int<WIDTH> >(-1));
            ecuIODisPatcherPool[i]->en.write(static_cast<bool>(0));
            item.hcuID = -1;
            freeParam = 1;
    }else if(mcuPool[i]->currentReadID.read() != -1
    && ecuPool[i]->currentReadID.read() != -1
    && mcuPool[i]->executeTime.read() - sc_time(10, SC_NS) == sc_time_stamp()     // en should be one cycle ahead of computing and IODispatcher 
    && ecuPool[i]->executeTime.read()  - sc_time(10, SC_NS) == sc_time_stamp()
    && !mcuIODisPatcherPool[i]->en.read()
    && !ecuIODisPatcherPool[i]->en.read()){    //enable all computing unit
        if(item.type == 1) {
            mcuPool[i]->en.write(static_cast<bool>(1));
            mcuIODisPatcherPool[i]->en.write(static_cast<bool>(1));
        }else {
            ecuPool[i]->en.write(static_cast<bool>(1));
            ecuIODisPatcherPool[i]->en.write(static_cast<bool>(1));
        }
        freeParam = 0;
    }else {
        freeParam = -2;
    }
}
void Scheduler::scheduler_hcu_allocate() {
    while (true) {
        wait();
        if(start.read()){
            bool hasScheduled = false;
            for (auto it = schedulerTable->schedulerItemList.begin(); it != schedulerTable->schedulerItemList.end(); ){
                auto timeIt = it->TimeList.begin();
                if(it->issued){
                    // ecu allocation
                    sc_time st = sc_time_stamp();
                    int freeNum = 0;
                    bool isforward = true;
                    for (; timeIt != it->TimeList.end(); ++timeIt) {
                        // allocation cycle: C(startTime + 65)  C(startTime + 130) ...
                        if (timeIt->hcuID == -1 && st - it->startTime == timeIt->start_duration && timeIt->start_duration != sc_time(0, SC_NS)){
                            if(hasScheduled) {
                                break;
                            }else {
                                allocateHCU(mcuPool, ecuPool, mcuIODisPatcherPool, ecuIODisPatcherPool, it->readID, it->segmentID, st + sc_time(20, SC_NS), it->endTime, it->addr, *timeIt, alloParam);
                                hasScheduled = true;
                            }
                            assert(alloParam != -1 && it->TimeList.size() > 1 && "Error: Cannot stop ecu allocation util its over!");
                            assert(alloParam != -2 && it->TimeList.size() > 1 && "Error: ecu allocator cannot operate mcu!");
                            if(alloParam > -1) {
                                std::cout << "successfully Allocate ecu: " << timeIt->hcuID <<" for Segment: " << it->segmentID << " -----UB: " << it->UB << " startTime: " << st + sc_time(20, SC_NS) << " endTime: "  << it->endTime << std::endl;
                            }else if(alloParam == -3) {
                                it->issued = 0;
                                for(auto xyz = it->TimeList.begin(); xyz != it->TimeList.end(); ++xyz) {
                                    xyz->hcuID = -1;
                                }
                                std::cout << "failed Allocate ecu: freeing all related hcu and wait for reallocating..." << std::endl;
                                break;
                            }
                        }else if(timeIt->hcuID >= 0){
                            int id = timeIt->hcuID;
                            freeHCU(mcuPool, ecuPool, mcuIODisPatcherPool, ecuIODisPatcherPool, *timeIt, freeParam);
                            if(freeParam == 1) {
                                if(++freeNum == it->HCU_Total_NUM) {
                                    std::cout << "successfully Free mcu: " << id << " for Seg: " << it->segmentID << " at: " << sc_time_stamp() + sc_time(10, SC_NS) << std::endl;
                                    {
                                        releaseBlock(it->addr, freeList);
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
                    assert(it->HCU_Total_NUM <= HCU_NUM && "Error: exceeding hcu allocation bounds!");
                    it->startTime = sc_time_stamp();
                    it->endTime = sc_time_stamp() + sc_time((it->UB + 2) * 10, SC_NS); // endTime = startTime + (newW.upperbound + 1)
                    assert((timeIt->hcuID==-1 || timeIt->hcuID == -2) && "Error: mcu already allocated!");
                    assert(timeIt->type && " Error: mcu allocator cannot operates ecu!");
                    if(hasScheduled) {
                        break;
                    }else {
                        allocateHCU(mcuPool, ecuPool, mcuIODisPatcherPool, ecuIODisPatcherPool, it->readID, it->segmentID, it->startTime + sc_time(20, SC_NS), it->endTime, it->addr, *timeIt, alloParam);
                        hasScheduled = true;
                    }
                    if (alloParam == -2){
                        std::cout << "failed allocate mcu for " << it->segmentID << ", wait for some time..." << std::endl;
                        wait(10, SC_NS);
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
                /*if (mt->currentReadID.read()!=-1 
                    && mt->currentSegID.read()!=-1) {
                        if(mt->currentSegID.read() == 0) {
                            std::cout << "mcuID:" << i << " "<< sc_time_stamp() << std::endl;
                            for(int j = 0; j < mt->UpperBound.read(); j ++) {
                                std::cout << mcuIODisPatcherPool[i]->ri[j].read() << std::endl;
                                std::cout << localRAM[mt->addr.read()].data[j] << std::endl;
                            }
                        }
                }*/
                if (mt->currentReadID.read()!=-1 // one cycle after hcu allocation
                    && mt->currentSegID.read()!=-1 
                    && sc_time_stamp() == mt->executeTime.read() - sc_time(10, SC_NS) ){
                    //std::cout << "IO start at" << sc_time_stamp() << std::endl;
                    if (mt->type.read()){ // mcu

                        assert(mt->LowerBound.read()==0 && "mcu's LowerBound must be 0!");
                        
                        for(int j = 0; j < mt->UpperBound.read(); j ++) {
                            mcuIODisPatcherPool[i]->ri[j].write(localRAM[mt->addr.read()].Rdata[j]);
                            mcuIODisPatcherPool[i]->qi[j].write(localRAM[mt->addr.read()].Qdata[j]);
                            mcuIODisPatcherPool[i]->w[j].write(localRAM[mt->addr.read()].Wdata[j]);
                        }
                    }else{ // ecu

                        assert(et->LowerBound.read()!=0 && "ecu's LowerBound must not be 0!");
                        for(int j = 0; j < et->UpperBound.read(); j ++) {
                            ecuIODisPatcherPool[i]->ri[j].write(localRAM[et->addr.read()].Rdata[j]);
                            ecuIODisPatcherPool[i]->qi[j].write(localRAM[et->addr.read()].Qdata[j]);
                            ecuIODisPatcherPool[i]->w[j].write(localRAM[et->addr.read()].Wdata[j]);
                        }

                    }
                }
            }
        }
    }
}

void Scheduler::scheduler_rt_checkTable(){
    while(true) {
        wait();
        if(start.read()) {
            for(auto it = schedulerTable->schedulerItemList.begin(); it != schedulerTable->schedulerItemList.end(); it++) {
               if(it->issued && it->UB >= 66) {
                  sc_int<WIDTH> output[it->HCU_Total_NUM.to_int()];
                  bool enable = false;
                  int index = 0;
                  for( auto timeIt = it->TimeList.begin(); timeIt != it->TimeList.end(); timeIt++) {
                        if(timeIt->hcuID != -1) {
                            sc_int<WIDTH>  id = timeIt->hcuID;
                            if(timeIt->type) {
                                if(mcuPool[id]->en) {
                                    output[index++] = mcuPool[id]->regBiggerScore[0].read(); 
                                    enable = true;
                                }
                            }else {
                                if(ecuPool[id]->en) {
                                    output[index++] = ecuPool[id]->regBiggerScore[0].read();
                                    enable = true;
                                }
                            }
                        }
                  } 
                  if(enable) {
                        int path = it->HCU_Total_NUM.to_int();
                        bool success = false;
                        if(path > 0 && path <= 2) {
                            for(int i = 0; i < 128; i ++) {
                                if(reductionInputArray[i]->num_free()) {
                                    reductionInput rt;
                                    for(int j = 0; j < index; j ++) {
                                        rt.data[j] = output[j];
                                     }
                                     reductionInputArray[i]->write(rt);
                                     notifyArray[i]->write(static_cast<sc_int<WIDTH>>(path));
                                     success = true;
                                     break;
                                }
                            }                            
                        }else if(path >=3 && path <= 4) {
                            for(int i = 128; i < 192; i ++) {
                                if(reductionInputArray[i]->num_free()) {
                                    reductionInput rt;
                                    for(int j = 0; j < index; j ++) {
                                        rt.data[j] = output[j];
                                     }
                                     reductionInputArray[i]->write(rt);
                                     notifyArray[i]->write(static_cast<sc_int<WIDTH>>(path));
                                     success = true;
                                     break;
                                }
                            }
                        }else if(path >= 5 && path <= 8) {
                            for(int i = 192; i < 224; i ++) {
                                if(reductionInputArray[i]->num_free()) {
                                    reductionInput rt;
                                    for(int j = 0; j < index; j ++) {
                                        rt.data[j] = output[j];
                                     }
                                     reductionInputArray[i]->write(rt);
                                     notifyArray[i]->write(static_cast<sc_int<WIDTH>>(path));
                                     success = true;
                                     break;
                                }
                            }
                        }else if(path >= 9 && path <= 16) {
                            for(int i = 224; i < 240; i ++) {
                                if(reductionInputArray[i]->num_free()) {
                                    reductionInput rt;
                                    for(int j = 0; j < index; j ++) {
                                        rt.data[j] = output[j];
                                     }
                                     reductionInputArray[i]->write(rt);
                                     notifyArray[i]->write(static_cast<sc_int<WIDTH>>(path));
                                     success = true;
                                     break;
                                }
                            }
                        }else if(path >= 17 && path <= 32) {
                            for(int i = 240; i < 248; i ++) {
                                if(reductionInputArray[i]->num_free()) {
                                    reductionInput rt;
                                    for(int j = 0; j < index; j ++) {
                                        rt.data[j] = output[j];
                                     }
                                     reductionInputArray[i]->write(rt);
                                     notifyArray[i]->write(static_cast<sc_int<WIDTH>>(path));
                                     success = true;
                                     break;
                                }
                            }
                        }else if(path >= 33 && path <= 64) {
                            for(int i = 248; i < 252; i ++) {
                                if(reductionInputArray[i]->num_free()) {
                                    reductionInput rt;
                                    for(int j = 0; j < index; j ++) {
                                        rt.data[j] = output[j];
                                     }
                                     reductionInputArray[i]->write(rt);
                                     notifyArray[i]->write(static_cast<sc_int<WIDTH>>(path));
                                     success = true;
                                     break;
                                }
                            }
                        }else if(path >= 65 && path <= 128) {
                            for(int i = 252; i < 254; i ++) {
                                if(reductionInputArray[i]->num_free()) {
                                    reductionInput rt;
                                    for(int j = 0; j < index; j ++) {
                                        rt.data[j] = output[j];
                                     }
                                     reductionInputArray[i]->write(rt);
                                     notifyArray[i]->write(static_cast<sc_int<WIDTH>>(path));
                                     success = true;
                                     break;
                                }
                            }
                        }
                        assert(success && "Error: Not enough reductionTree for scheduling!");
                   }
               } 
            }
        } 
    }
}

