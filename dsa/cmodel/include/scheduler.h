#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <systemc.h>
#include "hcu.h"
#include "rangeCountUnit.h"

SC_MODULE(Scheduler) {

    sc_in<bool> clk;
    sc_in<bool> rst;
    //sc_in<sc_int<WIDTH> >  UpperBound;

    // segments Queue
    sc_signal<sc_int<WIDTH> > riSegs[MAX_SEG_NUM][MAX_SEGLENGTH];
    sc_signal<sc_int<WIDTH> > qiSegs[MAX_SEG_NUM][MAX_SEGLENGTH];
    sc_signal<sc_int<WIDTH> > wSegs[MAX_SEG_NUM][MAX_SEGLENGTH];

    sc_signal<bool> start;   // initialization done signal.
    sc_signal<sc_int<WIDTH> > read_length;  // the length of read
    sc_signal<sc_int<WIDTH>> segNum; // number of segments 
    sc_signal<sc_int<WIDTH> > anchorRi[MAX_READ_LENGTH];
    sc_signal<sc_int<WIDTH> > anchorQi[MAX_READ_LENGTH];
    sc_signal<sc_int<WIDTH> > anchorW[MAX_READ_LENGTH];
    sc_signal<sc_int<WIDTH>> UpperBound[MAX_SEG_NUM];  // the length of every segments

    /*notify signal*/
    sc_event s_top;
    sc_event s_execute;
    sc_event s_allocate;

    RangeCountUnit *rc;

    void scheduler_top();
    void scheduler_execute();
    void scheduler_allocate();

	SC_CTOR(Scheduler) {

        SC_THREAD(scheduler_top);
        sensitive << s_top;

        SC_THREAD(scheduler_execute);
        sensitive << s_execute;

        SC_THREAD(scheduler_allocate);
        sensitive << s_allocate;

        rc = new RangeCountUnit("RangeCountUnit");
        rc->rst(rst);
        rc->cutDone(start);
        rc->read_length(read_length);
        rc->segNum(segNum);
        for(int i = 0; i < MAX_READ_LENGTH; i ++) {
            rc->anchorRi[i](anchorRi[i]);
            rc->anchorQi[i](anchorQi[i]);
            rc->anchorW[i](anchorW[i]);
            rc->segmentLen[i](UpperBound[i]);
        }
	}

};

#endif