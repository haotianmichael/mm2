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

int fillTableOfLongSegments(ram_data *localRAM, int &ramIndex, std::list<SchedulerItem> &st, sc_fifo<riSegment> &riSegQueueLong, sc_fifo<qiSegment> &qiSegQueueLong, sc_fifo<wSegment> &wSegQueueLong, sc_int<WIDTH> &segNumLong)
{

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
    st.push_back(sItem);
    return 1;
}

int fillTableOfShortSegments(ram_data *localRAM, int &ramIndex, std::list<SchedulerItem> &st, sc_fifo<riSegment> &riSegQueueShort, sc_fifo<qiSegment> &qiSegQueueShort, sc_fifo<wSegment> &wSegQueueShort, sc_int<WIDTH> &segNumShort)
{
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
    st.push_back(sItem);
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
                    fillTableOfShortSegments(localRAM, ramIndex, schedulerTable->schedulerItemList, riSegQueueShort, qiSegQueueShort, wSegQueueShort, segNumShort);
                }else if(segNumLong > 0){
                    fillTableOfLongSegments(localRAM, ramIndex, schedulerTable->schedulerItemList, riSegQueueLong, qiSegQueueLong, wSegQueueLong, segNumLong);
                }
            }else{
                // enough idle reduction -> allocate longPort
                if(segNumLong > 0){
                    fillTableOfLongSegments(localRAM, ramIndex, schedulerTable->schedulerItemList, riSegQueueLong, qiSegQueueLong, wSegQueueLong, segNumLong);
                }else if(segNumShort > 0){
                    fillTableOfShortSegments(localRAM, ramIndex, schedulerTable->schedulerItemList, riSegQueueShort, qiSegQueueShort, wSegQueueShort, segNumShort);
                }
            }
        }
    }
}
/*
int allocateHCU(MCU *mcuPool[HCU_NUM], ECU *ecuPool[HCU_NUM], const sc_int<WIDTH> &readID, const sc_int<WIDTH> &segID, const sc_time &startTime, const sc_time &endTime, const sc_int<WIDTH> &addr, SchedulerTime &item)
{

    int allo = -1;
    for (int i = 0; i < HCU_NUM; i++)
    {
        if (mcuPool[i]->currentReadID.read() == -1 && ecuPool[i]->currentReadID.read() == -1)
        {
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
    // Core Algorightm: Replacement Policy
    if (allo == HCU_NUM)
    {
        if (item.type)
        { // if mcu allocation failed, wait
            allo = -2;
        }
        else
        { // if ecu allocation failed, replacing
            for (int i = 0; i < HCU_NUM; i++)
            {
                // if()  是否有刚启动不久的hcu（UB<=65）的，直接重新弄分配
                if (!mcuPool[i]->type.read() && !ecuPool[i]->type.read())
                {
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

void Scheduler::scheduler_hcu_allocate()
{
    while (true)
    {
        wait();
        if (start.read())
        {
            for (auto it = schedulerTable.schedulerItemList.rbegin(); it != schedulerTable.schedulerItemList.rend(); ++it)
            {

                auto timeIt = it->TimeList.rbegin();
                if (it->issued)
                {
                    // ecu allocation
                    sc_time st = sc_time_stamp();
                    for (; timeIt != it->TimeList.rend(); ++timeIt)
                    {
                        // allocation cycle: C(startTime + 65)  C(startTime + 130) ...
                        if (timeIt->hcuID == -1 && st - it->startTime == timeIt->start_duration)
                        {
                            if (!(timeIt->hcuID = allocateHCU(mcuPool, ecuPool, it->readID, it->segmentID, it->startTime + sc_time(10, SC_NS), it->endTime, it->addr, *timeIt)))
                            {
                                assert(timeIt->hcuID == -1 && it->TimeList.size() > 1 && "Error: Cannot stop ecu allocation util its over!");
                            }
                        }
                    }
                }
                else
                {
                    // mcu allocation
                    it->startTime = sc_time_stamp();
                    it->endTime = sc_time_stamp() + sc_time((it->UB + 1) * 10, SC_NS); // endTime = startTime + (newW.upperbound + 1)
                    assert(timeIt->hcuID && "Error: mcu already allocated!");
                    assert(!timeIt->type && " Error: mcu allocator cannot operates ecu!");
                    if ((timeIt->hcuID = allocateHCU(mcuPool, ecuPool, it->readID, it->segmentID, it->startTime + sc_time(10, SC_NS), it->endTime, it->addr, *timeIt)) == -2)
                    {
                        std::cout << "mcu allocation failed, wait for some time..." << std::endl;
                        wait(40, SC_NS);
                        continue;
                    }
                    assert((timeIt->hcuID == -1) && "Error: mcu allocator return the wrong value!");
                    std::cout << "mcu allocated successfully!" << std::endl;
                    it->issued = 1;
                    if (it->HCU_Total_NUM > 1)
                        it--; // finish mcu, then continue allocate ecu of one segment.
                }
            }
        }
    }
}

void Scheduler::scheduler_hcu_execute()
{
    while (true)
    {
        wait();
        if (start.read())
        {
            for (int i = 0; i < HCU_NUM; i++)
            {

                MCU *mt = mcuPool[i];
                ECU *et = ecuPool[i];

                if (mt->currentReadID.read() // one cycle after hcu allocation
                    && mt->currentSegID.read() && sc_time_stamp() == mt->executeTime.read())
                {
                    if (mt->type.read())
                    { // mcu
                        mcuIODispatcher ing("mcuIODispatcher");

                        sc_signal<sc_int<WIDTH>> ri[MCUInputLaneWIDTH];
                        sc_signal<sc_int<WIDTH>> qi[MCUInputLaneWIDTH];
                        sc_signal<sc_int<WIDTH>> w[MCUInputLaneWIDTH];

                        ing.clk(clk);
                        ing.rst(rst);
                        assert(mt->LowerBound.read() && "mcu's LowerBound must be 0!");
                        ing.LowerBound(mt->LowerBound);
                        ing.UpperBound(mt->UpperBound);
                        for(int i = 0; i < ing.UpperBound.read(); i ++) {
                                ing.ri[i].write(localRAM[mt->addr.read()].data[i]);
                                ing.ri[i].write(localRAM[mt->addr.read() + 1].data[i]);
                                ing.ri[i].write(localRAM[mt->addr.read() + 2].data[i]);

                        }

                        for (int i = 0; i < MCUInputLaneWIDTH; i++)
                        {
                            ing.ri_out[i](ri[i]);
                            ing.qi_out[i](qi[i]);
                            ing.w_out[i](w[i]);
                        }
                        mt->clk(clk);
                        mt->rst(rst);
                        for (int j = 0; j < MCUInputLaneWIDTH; j++)
                        {
                            mt->riArray[j](ri[j]);
                            mt->qiArray[j](qi[j]);
                            mt->W[j](w[j]);
                        }
                    }
                    else
                    { // ecu

                        sc_signal<sc_int<WIDTH>> ri[MCUInputLaneWIDTH];
                        sc_signal<sc_int<WIDTH>> qi[MCUInputLaneWIDTH];
                        sc_signal<sc_int<WIDTH>> w[MCUInputLaneWIDTH];
                        sc_signal<sc_int<WIDTH>> ecu_ri;
                        sc_signal<sc_int<WIDTH>> ecu_qi;
                        sc_signal<sc_int<WIDTH>> ecu_w;

                        ecuIODispatcher ing("ecuIODispatcher");
                        ing.clk(clk);
                        ing.rst(rst);
                        assert(et->LowerBound.read() && "ecu's LowerBound must not be 0!");
                        ing.LowerBound(et->LowerBound);
                        ing.UpperBound(et->UpperBound);

                        for(int i = 0; i < ing.UpperBound.read(); i ++) {
                                ing.ri[i].write(localRAM[et->addr.read()].data[i]);
                                ing.qi[i].write(localRAM[et->addr.read() + 1].data[i]);
                                ing.w[i].write(localRAM[et->addr.read() + 2].data[i]);

                        }

                        ing.ecu_ri_out(ecu_ri);
                        ing.ecu_qi_out(ecu_qi);
                        ing.ecu_w_out(ecu_w);
                        for (int i = 0; i < ECUInputLaneWIDTH; i++)
                        {
                            ing.ri_out[i](ri[i]);
                            ing.qi_out[i](qi[i]);
                            ing.w_out[i](w[i]);
                        }

                        et->clk(clk);
                        et->rst(rst);
                        for (int j = 0; j < ECUInputLaneWIDTH; j++)
                        {
                            et->riArray[j](ri[j]);
                            et->qiArray[j](qi[j]);
                            et->W[j](w[j]);
                        }
                        et->ecu_ri(ecu_ri);
                        et->ecu_qi(ecu_qi);
                        et->ecu_W(ecu_w);
                    }
                }
                else if (mt->currentReadID.read() && mt->currentSegID.read() && mt->freeTime.read() == sc_time_stamp())
                { // free hcu

                    mt->currentReadID.write(static_cast<sc_int<WIDTH>>(-1));
                    mt->currentSegID.write(static_cast<sc_int<WIDTH>>(-1));
                    mt->freeTime.write(sc_time(0, SC_NS));
                    mt->executeTime.write(sc_time(0, SC_NS));
                    mt->type.write(static_cast<bool>(0));
                    mt->LowerBound.write(static_cast<sc_int<WIDTH>>(-1));
                    mt->UpperBound.write(static_cast<sc_int<WIDTH>>(-1));

                    et->currentReadID.write(static_cast<sc_int<WIDTH>>(-1));
                    et->currentSegID.write(static_cast<sc_int<WIDTH>>(-1));
                    et->freeTime.write(sc_time(0, SC_NS));
                    et->executeTime.write(sc_time(0, SC_NS));
                    et->type.write(static_cast<bool>(0));
                    et->LowerBound.write(static_cast<sc_int<WIDTH>>(-1));
                    et->UpperBound.write(static_cast<sc_int<WIDTH>>(-1));
                }
            }
        }
    }
}

// 查表，如果有满足条件的直接分配RT
void Scheduler::scheduler_rt_checkTable()
{
}
*/