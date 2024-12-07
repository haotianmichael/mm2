#include <cassert>
#include "chain.h"
#define HCU_OUTPUT 1

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
bool isRamFree(std::vector<bool>& freeListForLong, std::vector<bool>& freeListForShort) {
    bool shortfull = false;
    bool longfull = false;
    for(int i = 0; i < RAM_SIZE; i ++) {
        if(freeListForLong[i]) {
            longfull = true;
            break;
        }
    }
    for(int i = 0; i < RAM_SIZE; i ++) {
        if(freeListForShort[i]) {
            shortfull = true;
            break;
        }
    }
    return (shortfull && longfull);
}
void Chain::chain_ram_check() {
    while(true) {
        wait();
        bool is = isRamFree(freeListForLong, freeListForShort); 
        if(is) {
            ram_not_full.notify(SC_ZERO_TIME);
        }
    }
}

void Chain::chain_hcu_cut() {
    while (true) {
        wait();
        if (start.read()) {
            /*
            Cut one batch of read (one read for a batch for simulator)
            FIXME: bottleneck lies in batch size 
            */
            int rangeHeuristic[8] = {0, 16, 512, 1024, 2048, 3072, 4096, 5000};
            for(; readIdx < ReadNumProcessedOneTime; readIdx++){
                // compute dynamic range and cut
                // cut algorighm is strictly according to the software version.
                for(int j = 0; j < anchorNum[readIdx]->read(); j ++) {
                    if(anchorSegNum[readIdx][j+1]->read() != anchorSegNum[readIdx][j]->read()) {
                        anchorSuccessiveRange[readIdx][j] = 1;
                        continue;
                    }
                    int end = j + 5000;
                    for(int delta = 0; delta < 8; delta++) {
                        if((j + rangeHeuristic[delta] < anchorNum[readIdx]->read())) {
                            if(anchorSegNum[readIdx][j+rangeHeuristic[delta]]->read() == anchorSegNum[readIdx][j]->read()) {
                                    if(anchorRi[readIdx][j+rangeHeuristic[delta]]->read() - anchorRi[readIdx][j]->read() > 5000) {
                                            end = j + rangeHeuristic[delta];
                                            break;
                                    }
                            }else {
                                end = j + rangeHeuristic[delta-1];
                                break;
                            }
                        }
                        // FIXME: the last 16 anchor will omit segID
                        if(anchorNum[readIdx]->read() - j <= 16) {
                             end = anchorNum[readIdx]->read() - 1; 
                        }
                    }
                    while(end > j && anchorRi[readIdx][end]->read() - anchorRi[readIdx][j]->read() > 5000) {
                        end--;
                    } 
                    /*
                        1. range of anchor[j]'s successor is [j + 1, end]
                        2. cut when rangeLength = 1
                        3. max range of a segment is its UpperBound
                    */
                    anchorSuccessiveRange[readIdx][j] = end-j+1;
                    //std::cout << end - j + 1  << " " << j<< std::endl;
                }
                // store segments into SRAM. 
                int segStart = 0, tmpSegLongNum = 0, tmpSegShortNum = 0, tmpSegID = 0;
                sc_int<WIDTH> *RiData = new sc_int<WIDTH>[MAX_SEGLENGTH];
                sc_int<WIDTH> *QiData = new sc_int<WIDTH>[MAX_SEGLENGTH];
                sc_int<WIDTH> *Idx = new sc_int<WIDTH>[MAX_SEGLENGTH];
                sc_int<WIDTH> WData;
                assert(readIdx < ReadNumProcessedOneTime && "Error: cutting operation exceeds the maximum");
                // FIXME: there are still a few UpperBound situations which is not covered here.
                for(int i = 0; i < anchorNum[readIdx]->read(); i ++) {
                    if(!isRamFree(freeListForLong, freeListForShort)) {
                        wait(ram_not_full);
                    }
                    assert(anchorSuccessiveRange[readIdx][i] != 0 && "Error: successiveRange cannot be -1.");
                    if(anchorSuccessiveRange[readIdx][i] != 1 && anchorSuccessiveRange[readIdx][i] != -1) {
                        RiData[segStart] = anchorRi[readIdx][i]->read();
                        QiData[segStart] = anchorQi[readIdx][i]->read();
                        Idx[segStart] = anchorIdx[readIdx][i]->read();
                        segStart++;
                    }else if(anchorSuccessiveRange[readIdx][i] == 1 || i == anchorNum[readIdx]->read()-1) {
                        // range = 1 means segments ends with this anchor
                        // ending anchor still added
                        RiData[segStart] = anchorRi[readIdx][i]->read();
                        QiData[segStart] = anchorQi[readIdx][i]->read();
                        Idx[segStart] = anchorIdx[readIdx][i]->read();
                        WData = anchorW[readIdx]->read();
                        segStart++;
                        if(segStart <= MCUInputLaneWIDTH) {
                            ramIndexForShort = allocateBlock(freeListForShort);
                            localRAMForShort[ramIndexForShort].readID = readIdx;
                            localRAMForShort[ramIndexForShort].segID = tmpSegID << 1 | 0; // shortSegments ends with 0
                            localRAMForShort[ramIndexForShort].UpperBound = segStart;
                            tmpSegShortNum++;
                            for(int i = 0; i < segStart; i ++) {
                                localRAMForShort[ramIndexForShort].Rdata[i] = RiData[i];
                                localRAMForShort[ramIndexForShort].Qdata[i] = QiData[i];
                                localRAMForShort[ramIndexForShort].Idx[i] = Idx[i];
                            }
                            localRAMForShort[ramIndexForShort].Wdata = WData;
                            localRAMForShort[ramIndexForShort].used= true;
                            localRAMForShort[ramIndexForShort].allocated = false;
                        }else {
                            ramIndexForLong = allocateBlock(freeListForLong);
                            localRAMForLong[ramIndexForLong].readID = readIdx;
                            localRAMForLong[ramIndexForLong].segID = tmpSegID << 1 | 1;
                            localRAMForLong[ramIndexForLong].UpperBound = segStart;
                            tmpSegLongNum++;
                            for(int i = 0; i < segStart; i ++) {
                                localRAMForLong[ramIndexForLong].Rdata[i] = RiData[i];
                                localRAMForLong[ramIndexForLong].Qdata[i] = QiData[i];
                                localRAMForLong[ramIndexForLong].Idx[i] = Idx[i];
                            }
                            localRAMForLong[ramIndexForLong].Wdata = WData;
                            localRAMForLong[ramIndexForLong].used = true;
                            localRAMForLong[ramIndexForLong].allocated = false;
                        }
                        tmpSegID++;
                        segStart = 0;
                    }
                }
                std::cout << "this read's segNum: " << tmpSegLongNum + tmpSegShortNum << std::endl;
            }
        }
    }
}
int fillTableOfLongSegments(std::mutex& mtx, std::vector<ram_dataForLong> &localRAMForLong, std::vector<bool> &freeListForLong, std::list<SchedulerItem> &st) {
    // find data in ram ready to be allocated
    int ramIndex= -1;
    bool allo = false;
    for(int i = 0; i < RAM_SIZE; i ++) {
        if(!freeListForLong[i] && localRAMForLong[i].used && !localRAMForLong[i].allocated) {
           ramIndex = i;
           localRAMForLong[i].allocated = true;
           allo = true;
           break; 
        }
    }
    if(!allo) {
        return -1;
    }
    SchedulerItem sItem;
    sItem.issued = 0;
    sItem.readID = localRAMForLong[ramIndex].readID;
    sItem.segmentID = localRAMForLong[ramIndex].segID >> 1;
    sItem.UB = localRAMForLong[ramIndex].UpperBound;
    sItem.addr = static_cast<sc_int<WIDTH>>(ramIndex);

    assert(localRAMForLong[ramIndex].UpperBound >= 66 && "Error: LongQueue's upperbound cannot be less than 66");
    sItem.HCU_Total_NUM = 2 + (localRAMForLong[ramIndex].UpperBound - 66) / 65;
    for (int i = 0; i < sItem.HCU_Total_NUM; i++){
        SchedulerTime tl;
        tl.type = i == 0 ? 1 : 0; // mcu & ecu
        tl.SBase = i * 65;
        tl.LBase = localRAMForLong[ramIndex].UpperBound;
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
    sItem.Reduction_FIFO_Idx.range(31, 0) = -1;
    sItem.Reduction_FIFO_Idx.range(63, 32) = 0;
    {
        std::lock_guard<std::mutex> lock(mtx);
        st.push_back(sItem);
    }
    return 1;
}

int fillTableOfShortSegments(std::mutex& mtx, std::vector<ram_dataForShort> &localRAMForShort, std::vector<bool> &freeListForShort, std::list<SchedulerItem> &st) {
    // find data in ram ready to be allocated
    int ramIndex= -1;
    bool allo = false;
    for(int i = 0; i < RAM_SIZE; i ++) {
        if(!freeListForShort[i] && localRAMForShort[i].used && !localRAMForShort[i].allocated) {
           ramIndex = i;
           localRAMForShort[i].allocated = true;
           allo = true;
           break; 
        }
    }
    if(!allo) {
        return -1;
    }
    SchedulerItem sItem;
    sItem.issued = 0;
    sItem.readID = localRAMForShort[ramIndex].readID;
    sItem.segmentID = localRAMForShort[ramIndex].segID >> 1;
    /*if(newW->upperBound == 11) {
        std::cout << "ri->data[0] from fifo is " <<  newRi->data[0] << " ri->data[last] is " << newRi->data[newW->upperBound - 1] << " segID: "<< sItem.segmentID << std::endl;
        std::cout << "ri->data[0] from RAM is " << localRAM[ramIndex].data[0] << std::endl;
    }*/
    sItem.UB = localRAMForShort[ramIndex].UpperBound;
    sItem.HCU_Total_NUM = 1;
    sItem.addr = static_cast<sc_int<WIDTH>>(ramIndex);

    SchedulerTime tl;
    tl.start_duration = sc_time(0, SC_NS);
    tl.hcuID = -1;
    tl.type = 1;
    tl.SBase = 0;
    tl.LBase = localRAMForShort[ramIndex].UpperBound;
    // fill at real allocationTime
    sItem.startTime = sc_time(0, SC_NS);
    sItem.endTime = sc_time(0, SC_NS);
    sItem.TimeList.push_back(tl);
    sItem.Reduction_FIFO_Idx.range(31, 0) = -1;
    sItem.Reduction_FIFO_Idx.range(63, 32) = 0;
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
void Chain::chain_hcu_fillTable(){
    while (true){
        wait();
        if (start.read()){
            bool enoughRed = true;
            countIdle(ROCC, enoughRed);
            /*allocate ShortSeg as soon as one kind of reductionTree is running out.*/
            if(!enoughRed){
                // little idle reduction -> allocate shortPort
                if(fillTableOfShortSegments(schedulerTable->mtx, localRAMForShort, freeListForShort, schedulerTable->schedulerItemList) == -1) {
                    fillTableOfLongSegments(schedulerTable->mtx, localRAMForLong, freeListForLong, schedulerTable->schedulerItemList);
                }
           }else{
                // enough idle reduction -> allocate longPort
                if(fillTableOfLongSegments(schedulerTable->mtx, localRAMForLong, freeListForLong, schedulerTable->schedulerItemList) == -1) {
                    fillTableOfShortSegments(schedulerTable->mtx, localRAMForShort, freeListForShort, schedulerTable->schedulerItemList);
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
void Chain::chain_hcu_allocate() {
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
                                if(HCU_OUTPUT == 1) {
                                    std::cout << "successfully Allocate ecu: " << timeIt->hcuID <<" for Segment: " << it->segmentID << " -----UB: " << it->UB << " startTime: " << st + sc_time(20, SC_NS) << " endTime: "  << it->endTime << std::endl;
                                }
                            }else if(alloParam == -3) {
                                it->issued = 0;
                                for(auto xyz = it->TimeList.begin(); xyz != it->TimeList.end(); ++xyz) {
                                    xyz->hcuID = -1;
                                }
                                if(HCU_OUTPUT) {
                                    std::cout << "failed Allocate ecu: freeing all related hcu and wait for reallocating..." << std::endl;
                                }
                                break;
                            }
                        }else if(timeIt->hcuID >= 0){
                            int id = timeIt->hcuID;
                            freeHCU(mcuPool, ecuPool, mcuIODisPatcherPool, ecuIODisPatcherPool, *timeIt, freeParam);
                            if(freeParam == 1) {
                                if(++freeNum == it->HCU_Total_NUM) {
                                    if(HCU_OUTPUT) {
                                        std::cout << "successfully Free mcu: " << id << " for Seg: " << it->segmentID << " at: " << it->endTime << std::endl;
                                    }
                                    if(it->UB <= MCUInputLaneWIDTH) {
                                        releaseBlock(it->addr, freeListForShort);
                                        std::lock_guard<std::mutex> lock(schedulerTable->mtx);
                                        it = schedulerTable->schedulerItemList.erase(it);
                                    }else {
                                        it->UB = -1;
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
                    it->endTime = sc_time_stamp() + sc_time((it->UB + 2 + 2 + 1) * 10, SC_NS); // endTime = startTime + (newW.upperbound + 1)
                    assert((timeIt->hcuID==-1 || timeIt->hcuID == -2) && "Error: mcu already allocated!");
                    assert(timeIt->type && " Error: mcu allocator cannot operates ecu!");
                    if(hasScheduled) {
                        break;
                    }else {
                        allocateHCU(mcuPool, ecuPool, mcuIODisPatcherPool, ecuIODisPatcherPool, it->readID, it->segmentID, it->startTime + sc_time(20, SC_NS), it->endTime, it->addr, *timeIt, alloParam);
                        hasScheduled = true;
                    }
                    if (alloParam == -2){
                        if(HCU_OUTPUT) {
                            std::cout << "failed allocate mcu for " << it->segmentID << ", wait for some time..." << std::endl;
                        }
                        wait(10, SC_NS);
                        continue;
                    }
                    assert(timeIt->hcuID>=0  && "Error: mcu allocator return the wrong value!");
                    if(HCU_OUTPUT) {
                        std::cout << "successfully Allocate mcu: " << timeIt->hcuID <<" for Segment: " << it->segmentID << " -----UB: " << it->UB << " startTime: " << it->startTime+sc_time(20, SC_NS) << " endTime: "  << it->endTime << std::endl;
                    }
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

void Chain::chain_ram_write_score() {
    while(true) {
        wait();
        if(start.read()) {

            // the first 65 final-score (FinalScoreIdx up to 65)
            for(auto it = schedulerTable->schedulerItemList.begin(); it != schedulerTable->schedulerItemList.end(); it ++) {
                if(it->issued) {  
                    auto timeIt = it->TimeList.begin();
                    for(; timeIt != it->TimeList.end(); timeIt++) {
                        if(timeIt->hcuID != -1 && timeIt->type) {   
                            sc_int<WIDTH> id = timeIt->hcuID;
                            sc_int<WIDTH> &idx = localRAMForLong[it->addr].FinalScoreIdx;
                            if(mcuPool[id]->en && idx <= 65 && mcuPool[id]->regBiggerScore[0].read() != -1) {
                                localRAMForLong[it->addr].FinalScore[idx++] = mcuPool[id]->regBiggerScore[0].read();
                            } 
                        }
                    }
                }
            }     

            // the rest of final-score (FinalScoreIdx start from 66)
           for(int i = 0; i < Reduction_USAGE; i ++) {
                int address = resultAddrArray[i].read();
                if(address != -1) {
                    sc_int<WIDTH> &idx = localRAMForLong[address].FinalScoreIdx;
                    localRAMForLong[address].FinalScore[idx++] = resultArray[i].read();
                } 
            } 
        }
    }
}

void Chain::chain_ram_read_score() {
    while(true) {
        wait();
        if(start.read()) {
            for(auto it = schedulerTable->schedulerItemList.begin(); it != schedulerTable->schedulerItemList.end(); it ++) {
                if(it->issued && it->UB >= 66) {
                    auto timeIt = it->TimeList.begin();
                    sc_int<WIDTH>* finalScore = localRAMForLong[it->addr].FinalScore;
                    for(; timeIt != it->TimeList.end(); timeIt++) {
                        sc_int<WIDTH> id = timeIt->hcuID;
                        if(id != -1) {
                            for(int i = 0; i < MAX_SEGLENGTH; i ++) {
                                if(finalScore[i] != -1 && ecuIODisPatcherPool[id]->final_score[i].read() != -1) {
                                     ecuIODisPatcherPool[id]->final_score[i].write(finalScore[i]);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void Chain::chain_hcu_execute(){
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
                        if(mt->UpperBound.read() <= MCUInputLaneWIDTH) {
                            for(int j = 0; j < mt->UpperBound.read(); j ++) {
                                mcuIODisPatcherPool[i]->ri[j].write(localRAMForShort[mt->addr.read()].Rdata[j]);
                                mcuIODisPatcherPool[i]->qi[j].write(localRAMForShort[mt->addr.read()].Qdata[j]);
                                mcuIODisPatcherPool[i]->idx[j].write(localRAMForShort[mt->addr.read()].Idx[j]);
                                mcuIODisPatcherPool[i]->w[j].write(localRAMForShort[mt->addr.read()].Wdata);
                            }
                        }else {
                            for(int j = 0; j < mt->UpperBound.read(); j ++) {
                                mcuIODisPatcherPool[i]->ri[j].write(localRAMForLong[mt->addr.read()].Rdata[j]);
                                mcuIODisPatcherPool[i]->qi[j].write(localRAMForLong[mt->addr.read()].Qdata[j]);
                                mcuIODisPatcherPool[i]->idx[j].write(localRAMForLong[mt->addr.read()].Idx[j]);
                                mcuIODisPatcherPool[i]->w[j].write(localRAMForLong[mt->addr.read()].Wdata);
                            }
                        }
                    }else{ // ecu

                        assert(et->LowerBound.read()!=0 && "ecu's LowerBound must not be 0!");
                        if(et->UpperBound.read() <= MCUInputLaneWIDTH) {
                            for(int j = 0; j < et->UpperBound.read(); j ++) {
                                ecuIODisPatcherPool[i]->ri[j].write(localRAMForShort[et->addr.read()].Rdata[j]);
                                ecuIODisPatcherPool[i]->qi[j].write(localRAMForShort[et->addr.read()].Qdata[j]);
                                ecuIODisPatcherPool[i]->idx[j].write(localRAMForShort[et->addr.read()].Idx[j]);
                                ecuIODisPatcherPool[i]->w[j].write(localRAMForShort[et->addr.read()].Wdata);
                            }
                        }else {
                            for(int j = 0; j < et->UpperBound.read(); j ++) {
                                ecuIODisPatcherPool[i]->ri[j].write(localRAMForLong[et->addr.read()].Rdata[j]);
                                ecuIODisPatcherPool[i]->qi[j].write(localRAMForLong[et->addr.read()].Qdata[j]);
                                ecuIODisPatcherPool[i]->idx[j].write(localRAMForLong[et->addr.read()].Idx[j]);
                                ecuIODisPatcherPool[i]->w[j].write(localRAMForLong[et->addr.read()].Wdata);
                            }
                        }
                    }
                }
            }
        }
    }
}

void fillFIFOPorts(sc_fifo<reductionInput> *reductionInputEle, sc_int<WIDTH> &path, sc_int<WIDTH> *output, sc_int<WIDTH> *predecessor, sc_int<WIDTH> address, int outputNum) {
    reductionInput rt;
    int j = 0;
    assert(outputNum < 128 && "Error: exceeding reductionTree boundry!");
    for(; j < outputNum; j++) {
        rt.data[j] = output[j];
        rt.predecessor[j] = predecessor[j];
    }
    rt.data[127] = path;
    rt.address = address;
    reductionInputEle->write(rt);
}
void Chain::chain_rt_allocate(){
    while(true) {
        wait();
        if(rst.read()) {
            for(int i = 0; i < Reduction_FIFO_NUM; i ++) {
                notifyArray[i]->write(-1);
            }
        }else {
            if(start.read()) {
                for(int i = 0; i < Reduction_FIFO_NUM; i ++) {
                    if(notifyArray[i]->read() == -2) {
                        notifyArray[i]->write(-1);
                    }
                }
                for(auto it = schedulerTable->schedulerItemList.begin(); it != schedulerTable->schedulerItemList.end(); it++) {
                   schedulerTable->high_32 = it->Reduction_FIFO_Idx.range(63, 32);
                   if(it->issued && it->UB >= 66) {
                        schedulerTable->address = it->addr;
                        schedulerTable->enable = false;
                        schedulerTable->index = 0;
                        assert((std::next(it->TimeList.begin()))->type == 0 && "Error: Wrong ECU type!");
                        if((std::next(it->TimeList.begin()))->type == 0 && (std::next(it->TimeList.begin()))->hcuID != -1) {
                            for(auto timeIt = it->TimeList.begin(); timeIt != it->TimeList.end(); timeIt++) {
                                  if(timeIt->hcuID != -1) {
                                      schedulerTable->id = timeIt->hcuID;
                                      if(timeIt->type) {
                                          if(mcuPool[schedulerTable->id]->en) {
                                              assert(schedulerTable->index < 128 && "Error: index out of bounds");
                                              schedulerTable->output[schedulerTable->index++] = mcuPool[schedulerTable->id]->regBiggerScore[0].read(); 
                                              schedulerTable->outputPre[schedulerTable->index++] = mcuPool[schedulerTable->id]->predecessor[0].read();
                                              schedulerTable->enable = true;
                                          }
                                      }else {
                                          if(ecuPool[schedulerTable->id]->en) {
                                              assert(schedulerTable->index < 128 && "Error: index out of bounds");
                                              schedulerTable->output[schedulerTable->index++] = ecuPool[schedulerTable->id]->regBiggerScore[0].read();
                                              schedulerTable->outputPre[schedulerTable->index++] = mcuPool[schedulerTable->id]->predecessor[0].read();
                                              schedulerTable->enable = true;
                                          }
                                      }
                                      if(mcuPool[schedulerTable->id]->currentReadID.read() != -1 
                                          && ecuPool[schedulerTable->id]->currentReadID.read() != -1
                                          && mcuPool[schedulerTable->id]->freeTime.read() - sc_time(10, SC_NS)== sc_time_stamp() 
                                          && ecuPool[schedulerTable->id]->freeTime.read() - sc_time(10, SC_NS)== sc_time_stamp()
                                          && (mcuIODisPatcherPool[schedulerTable->id]->en.read()
                                          || ecuIODisPatcherPool[schedulerTable->id]->en.read())){ 
                                              it->Reduction_FIFO_Idx.range(63, 32) = -1;
                                      }
                                  }
                            } 
                            assert(schedulerTable->index>=1 && "Error: wrong time to reducting!");
                        }
                        if(schedulerTable->enable) {
                            schedulerTable->low_32 = it->Reduction_FIFO_Idx.range(31, 0); 
                            if (schedulerTable->low_32 >= 0) {
                                notifyArray[schedulerTable->low_32]->write(1); 
                                fillFIFOPorts(reductionInputArray[schedulerTable->low_32], it->HCU_Total_NUM, schedulerTable->output, schedulerTable->outputPre, schedulerTable->address, schedulerTable->index);
                            }else if(schedulerTable->low_32 == -1){
                                bool fillS = false;
                                for (int i = 0; i < Reduction_FIFO_NUM; i++){
                                    if (notifyArray[i]->read() == -1){
                                        it->Reduction_FIFO_Idx.range(31, 0) = i;
                                        notifyArray[i]->write(1); // successfully porting, ready to dispathing
                                        fillFIFOPorts(reductionInputArray[i], it->HCU_Total_NUM, schedulerTable->output, schedulerTable->outputPre, schedulerTable->address, schedulerTable->index);
                                        fillS = true;
                                        break;
                                    }
                                }
                                assert(fillS && "Error: Exceeding FIFO Capacity!");
                            }else {
                                std::cerr << "Error: Wong FIFO idx!" << std::endl;
                            }
                        }
                    }else if(schedulerTable->high_32 == -1) {
                        schedulerTable->low_32 = it->Reduction_FIFO_Idx.range(31, 0);
                        if(notifyArray[schedulerTable->low_32]->read() == 1) {
                            notifyArray[schedulerTable->low_32]->write(-2);
                        }
                        {
                           releaseBlock(it->addr, freeListForLong);
                           std::lock_guard<std::mutex> lock(schedulerTable->mtx);
                           it = schedulerTable->schedulerItemList.erase(it);
                        }
                    }
                }
            } 
        }
    }
}