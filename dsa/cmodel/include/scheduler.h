#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "hcu.h"
#include "rangeCountUnit.h"
#include "schedulerTable.h"
#include "reductionPool.h"
#include "ioDispatcher.h"
#include "partialScoreQueue.h"


SC_MODULE(Scheduler) {

    sc_in<bool> clk;
    sc_in<bool> rst;
    sc_signal<bool> start;     // initialization done signal

    sc_signal<sc_int<WIDTH> > anchorNum[ReadNumProcessedOneTime];  // number of anchors 
    sc_signal<sc_int<WIDTH> > anchorRi[ReadNumProcessedOneTime][MAX_READ_LENGTH];
    sc_signal<sc_int<WIDTH> > anchorQi[ReadNumProcessedOneTime][MAX_READ_LENGTH];
    sc_signal<sc_int<WIDTH> > anchorW[ReadNumProcessedOneTime][MAX_READ_LENGTH];
    sc_signal<sc_int<WIDTH>> anchorSuccessiveRange[ReadNumProcessedOneTime][MAX_READ_LENGTH];  // successive range of every anchor 
    
    // @RC Unit
    RangeCountUnit *rc;

    // @SegmentsQueue (Two Ports)
    // UpperBound <= 65 elements  Lane[0, 64]
    sc_signal<sc_int<WIDTH>> segNumLong; // number of segments 
    sc_fifo<riSegment> riSegQueueLong;  
    sc_fifo<qiSegment> qiSegQueueLong;
    sc_fifo<wSegment> wSegQueueLong;
    // UpperBound > 65
    sc_signal<sc_int<WIDTH>> segNumShort;
    sc_fifo<riSegment> riSegQueueShort; 
    sc_fifo<qiSegment> qiSegQueueShort;
    sc_fifo<wSegment> wSegQueueShort;
    sc_int<WIDTH> readIdx = 0; //read which currently being cut

    // @PartialScoreQueue
    PartialScoreQueue *partialScoreQueue;

    // @SchedulerTable
    SchedulerTable schedulerTable;

    // @LocalRAM store the Input
    ram_data localRAM[MAX_READ_LENGTH];
    int ramIndex;

    // @HCU Pool
    // simulator can only use either mcuPool[i] or ecuPool[i], cannot use them both at the same time.
    MCU *mcuPool[HCU_NUM];
    ECU *ecuPool[HCU_NUM];

    // @ReductionPool
    ReductionPool *reductionPool;

    void scheduler_hcu_pre();
    void scheduler_hcu_execute();
    void scheduler_hcu_fillTable();
    void scheduler_hcu_allocate();
    void scheduler_rt_checkTable();

	SC_CTOR(Scheduler) : 
         riSegQueueLong(MAX_SEG_NUM), 
         qiSegQueueLong(MAX_SEG_NUM), 
         wSegQueueLong(MAX_SEG_NUM),
         riSegQueueShort(MAX_SEG_NUM),
         qiSegQueueShort(MAX_SEG_NUM),
         wSegQueueShort(MAX_SEG_NUM),
         ramIndex(0) {

        // prepare the segmentQueue
        // one read per cycle.(延时问题后续重新考虑)
        SC_THREAD(scheduler_hcu_pre);
        sensitive << clk.pos();

        // fill SchedulerTable for every Segment based on SegQueue
        SC_THREAD(scheduler_hcu_fillTable);
        sensitive << clk.pos();

        // allocate HCU for every Segment based on SchedulerTable
        SC_THREAD(scheduler_hcu_allocate);
        sensitive << clk.pos();

        // HCU IO Wiring/Free and fill PSQTable
        SC_THREAD(scheduler_hcu_execute);
        sensitive << clk.pos();

        // check PSQTable and allocate 
        SC_THREAD(scheduler_rt_checkTable);
        sensitive << clk.pos();

        rc = new RangeCountUnit("RangeCountUnit");
        rc->rst(rst);
        rc->cutDone(start);
        for(int readIdx = 0; readIdx < ReadNumProcessedOneTime; readIdx++) {
            rc->anchorNum[readIdx](anchorNum[readIdx]);
            for(int i = 0; i < MAX_READ_LENGTH; i ++) {
                rc->anchorRi[readIdx][i](anchorRi[readIdx][i]);
                rc->anchorQi[readIdx][i](anchorQi[readIdx][i]);
                rc->anchorW[readIdx][i](anchorW[readIdx][i]);
                rc->anchorSuccessiveRange[readIdx][i](anchorSuccessiveRange[readIdx][i]);
            }
        }

        std::ostringstream mcu_name, ecu_name;
        for(int i = 0; i < HCU_NUM; i ++) {
            mcu_name << "mcuPool(" << i << ")";
            ecu_name << "ecuPool(" << i << ")";
            mcuPool[i] = new mCU(mcu_name.str().c_str());
            mcuPool[i]->clk(clk);
            mcuPool[i]->rst(rst);
            ecuPool[i] = new eCU(ecu_name.str().c_str());
            ecuPool[i]->clk(clk);
            ecuPool[i]->rst(rst);
            //for(int j = 0; j < InputLaneWIDTH; j ++) {
             //   hcuPool[i]->riArray[j](static_cast<sc_int<WIDTH> >(-1));
              //  hcuPool[i]->qiArray[j](static_cast<sc_int<WIDTH> >(-1));
               // hcuPool[i]->W[j](static_cast<sc_int<WIDTH> >(-1));
            //}
            //sc_bigint<TableWIDTH> mask = ~(1 << i); 
            //schedulerTable.HOCC &= mask;  // HOCC transfer
            mcu_name.str("");
            ecu_name.str("");
        }

        reductionPool = new ReductionPool("ReductionPool");
        reductionPool->clk(clk);
        reductionPool->rst(rst);
        //schedulerTable(reductionPool->ROCC.read());  // () override for ROCC transfer

        partialScoreQueue = new PartialScoreQueue("PartialScoreQueue");
        partialScoreQueue->clk(clk);
        partialScoreQueue->rst(rst);




	}

};

#endif