#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "hcu.h"
#include "rangeCountUnit.h"
#include "schedulerTable.h"
#include "reductionPool.h"
#include "ioDispatcher.h"
#include "partialScorePool.h"


SC_MODULE(Scheduler) {

    sc_in<bool> clk;
    sc_in<bool> rst;
    sc_signal<bool> start;     // initialization done signal

    sc_signal<sc_int<WIDTH> > anchorNum;  // number of anchors 
    sc_signal<sc_int<WIDTH> > anchorRi[MAX_READ_LENGTH];
    sc_signal<sc_int<WIDTH> > anchorQi[MAX_READ_LENGTH];
    sc_signal<sc_int<WIDTH> > anchorW[MAX_READ_LENGTH];
    sc_signal<sc_int<WIDTH>> anchorSuccessiveRange[MAX_READ_LENGTH];  // successive range of every anchor 
    
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

    // @PartialScoreQueue
    sc_fifo<sc_int<WIDTH> > partialScoreQueue;


    // @SchedulerTable
    SchedulerTable schedulerTable;

    // @HCU Pool
    HCU *hcuPool[HCU_NUM];

    // @ReductionPool
    ReductionPool *reductionPool;

    void scheduler_top();
    void scheduler_pre();
    void scheduler_execute();
    void scheduler_allocate();

	SC_CTOR(Scheduler) : 
         riSegsLong(MAX_SEG_NUM), 
         qiSegsLong(MAX_SEG_NUM), 
         wSegsLong(MAX_SEG_NUM),
         riSegsShort(MAX_SEG_NUM),
         qiSegsShort(MAX_SEG_NUM),
         wSegsShort(MAX_SEG_NUM) {

        SC_THREAD(scheduler_top);
        sensitive << clk.pos();

        // prepare the segmentQueue
        SC_THREAD(scheduler_pre);
        sensitive << clk.pos();

        // allocate HCU for every Segment
        SC_THREAD(scheduler_allocate);
        sensitive << clk.pos();

        SC_THREAD(scheduler_execute);
        sensitive << clk.pos();

        rc = new RangeCountUnit("RangeCountUnit");
        rc->rst(rst);
        rc->cutDone(start);
        rc->anchorNum(anchorNum);
        for(int i = 0; i < MAX_READ_LENGTH; i ++) {
            rc->anchorRi[i](anchorRi[i]);
            rc->anchorQi[i](anchorQi[i]);
            rc->anchorW[i](anchorW[i]);
            rc->anchorSuccessiveRange[i](anchorSuccessiveRange[i]);
        }

        std::ostringstream pe_name;
        for(int i = 0; i < HCU_NUM; i ++) {
            pe_name << "hcuPool(" << i << ")";
            hcuPool[i] = new HCU(pe_name.str().c_str());
            hcuPool[i]->clk(clk);
            hcuPool[i]->rst(rst);
            //for(int j = 0; j < InputLaneWIDTH; j ++) {
             //   hcuPool[i]->riArray[j](static_cast<sc_int<WIDTH> >(-1));
              //  hcuPool[i]->qiArray[j](static_cast<sc_int<WIDTH> >(-1));
               // hcuPool[i]->W[j](static_cast<sc_int<WIDTH> >(-1));
            //}
            sc_bigint<TableWIDTH> mask = ~(1 << i); 
            schedulerTable.HOCC &= mask;  // HOCC transfer
            pe_name.str("");
        }

        reductionPool = new ReductionPool("ReductionPool");
        reductionPool->clk(clk);
        reductionPool->rst(rst);
        schedulerTable(reductionPool->ROCC.read());  // () override for ROCC transfer

	}

};

#endif