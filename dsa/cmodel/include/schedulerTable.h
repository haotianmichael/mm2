#ifndef SCHEDULER_TABLE_H
#define SCHEDULER_TABLE_H

#include <helper.h>
#include <list>


struct riSegment{
    sc_int<WIDTH>  data[MAX_SEGLENGTH]; 
    sc_int<WIDTH> readID;   // readID which this segment belongs to 
    friend std::ostream& operator<<(std::ostream& os, const riSegment& segment){
        os << "riSegment: ";
        for(int i =0; i < MAX_SEGLENGTH; i ++) {
            os << segment.data[i] << " "; 
        }
        return os;
    }
};

struct qiSegment{
    sc_int<WIDTH>  data[MAX_SEGLENGTH]; 
    sc_int<WIDTH> segID;   // segmentID
    friend std::ostream& operator<<(std::ostream& os, const qiSegment& segment) {
        os << "qiSegment: ";
        for(int i =0; i < MAX_SEGLENGTH; i ++) {
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

struct SchedulerTime{
    sc_bit type;  // 1-mcu, 0-ecu
    sc_int<WIDTH> SBase;
    sc_int<WIDTH> LBase;
    sc_int<WIDTH> hcuID;
    sc_time cycle;  // when ecu start allocate 
};

struct SchedulerItem{

    sc_bit issued;
    sc_int<WIDTH> readID;
    sc_int<WIDTH> segmentID;
    sc_int<WIDTH> UB;
    sc_int<WIDTH> HCU_Total_NUM;
    // allocate hcu, start scheduler according to TimeList
    sc_time startTime;  // mcu start scheduler and allocate 
    std::list<SchedulerTime> TimeList;
};

struct SchedulerTable {

    //sc_bigint<TableWIDTH> HOCC;    // HCU's Occurence
    //sc_bigint<TableWIDTH> ROCC;    // ReductionTree's Occurence
    std::list<SchedulerItem> schedulerItemList;

    void addItem(const SchedulerItem& item) {
        schedulerItemList.push_back(item);
    }

    void removeItem(std::function<bool(const SchedulerItem&)> condition) {
        schedulerItemList.remove_if(condition);
    }


};



#endif