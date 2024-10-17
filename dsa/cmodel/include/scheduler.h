#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <systemc.h>
#include "hcu.h"
#include "rangeCountUnit.h"

struct riSegment{
    sc_int<WIDTH>  data[MAX_SEGLENGTH]; 
    sc_int<WIDTH> upperBound;
    friend std::ostream& operator<<(std::ostream& os, const riSegment& segment){
        os << "riSegment: ";
        for(int i =0; i < segment.upperBound; i ++) {
            os << segment.data[i] << " "; 
        }
        return os;
    }
};

struct qiSegment{
    sc_int<WIDTH>  data[MAX_SEGLENGTH]; 
    sc_int<WIDTH> upperBound;
    friend std::ostream& operator<<(std::ostream& os, const qiSegment& segment) {
        os << "qiSegment: ";
        for(int i =0; i < segment.upperBound; i ++) {
            os << segment.data[i] << " ";
        }
        return os;
    }
};

struct wSegment{
    sc_int<WIDTH>  data[MAX_SEGLENGTH]; 
    sc_int<WIDTH> upperBound;
    friend std::ostream& operator<<(std::ostream& os, const wSegment& segment) {
        os << "wSegment: ";
        for(int i = 0; i < segment.upperBound; i ++) {
                os << segment.data[i] << " ";
        }
        return os;
    }
};

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

    // @Segments Queue (Two Ports)
    // UpperBound <= 65 elements  Lane[0, 64]
    sc_signal<sc_int<WIDTH>> segNumLong; // number of segments 
    sc_fifo<riSegment> riSegsLong;  
    sc_fifo<qiSegment> qiSegsLong;
    sc_fifo<wSegment> wSegsLong;
    // UpperBound > 65
    sc_signal<sc_int<WIDTH>> segNumShort;
    sc_fifo<riSegment> riSegsShort; 
    sc_fifo<qiSegment> qiSegsShort;
    sc_fifo<wSegment> wSegsShort;

    // @SchedulerTable

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
	}

};

#endif