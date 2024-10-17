#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <systemc.h>
#include "hcu.h"
#include "rangeCountUnit.h"

struct riSegment{
    sc_int<WIDTH>  data[MAX_SEGLENGTH]; 
    sc_int<WIDTH> upperBound;
};

struct qiSegment{
    sc_int<WIDTH>  data[MAX_SEGLENGTH]; 
    sc_int<WIDTH> upperBound;
};

struct wSegment{
    sc_int<WIDTH>  data[MAX_SEGLENGTH]; 
    sc_int<WIDTH> upperBound;
};

SC_MODULE(Scheduler) {

    sc_in<bool> clk;
    sc_in<bool> rst;

    // segments Queue
    sc_signal<sc_int<WIDTH>> segNum; // number of segments 
    sc_fifo<riSegment> riSegs;
    sc_fifo<qiSegment> qiSegs;
    sc_fifo<wSegment> wSegs;

    sc_signal<sc_int<WIDTH> > anchorNum;  // number of anchors 
    sc_signal<sc_int<WIDTH> > anchorRi[MAX_READ_LENGTH];
    sc_signal<sc_int<WIDTH> > anchorQi[MAX_READ_LENGTH];
    sc_signal<sc_int<WIDTH> > anchorW[MAX_READ_LENGTH];
    sc_signal<sc_int<WIDTH>> anchorSuccessiveRange[MAX_READ_LENGTH];  // successive range of every anchor 
    
    // initialization done signal.
    sc_signal<bool> start;   

    RangeCountUnit *rc;

    void scheduler_top();
    void scheduler_pre();
    void scheduler_execute();
    void scheduler_allocate();

	SC_CTOR(Scheduler) : riSegs(MAX_SEG_NUM), qiSegs(MAX_SEG_NUM), wSegs(MAX_SEG_NUM) {

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
	}

};

#endif