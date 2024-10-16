#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <systemc.h>
#include "hcu.h"
#include "rangeCountUnit.h"

/*Segment Scheduler*/
SC_MODULE(Scheduler) {

    sc_in<bool> clk;
    sc_in<bool> rst;
    sc_in<sc_int<WIDTH> >  UpperBound;

    void scheduler_top();
	SC_CTOR(Scheduler) {
        SC_THREAD(scheduler_top);
        sensitive << clk.pos(); 
	}



};

#endif