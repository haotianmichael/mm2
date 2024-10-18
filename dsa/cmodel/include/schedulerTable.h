#ifndef SCHEDULER_TABLE_H
#define SCHEDULER_TABLE_H

#include <helper.h>


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