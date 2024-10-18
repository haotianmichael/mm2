#ifndef SCHEDULER_TABLE_H
#define SCHEDULER_TABLE_H

#include <helper.h>

struct SchedulerItem{

    sc_bit segID;
    sc_bit startTime;
    sc_bit UB;
    sc_bigint<TableWIDTH> HREQ;
    sc_bigint<TableWIDTH> RREQ;
    sc_int<WIDTH> SBase;
    sc_int<WIDTH> LBase;
};


struct SchedulerTable {

    sc_bigint<TableWIDTH> HOCC;    // HCU's Occurence
    sc_bigint<TableWIDTH> ROCC;    // ReductionTree's Occurence
    SchedulerItem schedulerItem[MAX_SEG_NUM];

    void operator()(const sc_bigint<TableWIDTH>& ROCC_sig) {
        ROCC = ROCC_sig;
    }

};



#endif