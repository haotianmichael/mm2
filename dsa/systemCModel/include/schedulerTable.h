#ifndef SCHEDULER_TABLE_H
#define SCHEDULER_TABLE_H

#include <helper.h>
#include <list>
#include <mutex>

struct ram_data{
   sc_int<WIDTH> Rdata[MAX_SEGLENGTH]; 
   sc_int<WIDTH> Qdata[MAX_SEGLENGTH]; 
   sc_int<WIDTH> Wdata[MAX_SEGLENGTH]; 
};

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
    bool type;  // 1-mcu, 0-ecu
    sc_int<WIDTH> SBase;
    sc_int<WIDTH> LBase;
    sc_int<WIDTH> hcuID;
    sc_time start_duration;  // when ecu start allocate 
};

struct SchedulerItem{

    bool issued;
    sc_int<WIDTH> readID;
    sc_int<WIDTH> segmentID;
    sc_int<WIDTH> addr;
    sc_int<WIDTH> UB;
    sc_int<WIDTH> HCU_Total_NUM;
    // allocate hcu, start scheduler according to TimeList
    sc_time startTime;  // mcu start scheduler and allocate 
    sc_time endTime;   // all hcu finished, freeTime
    sc_int<64> Reduction_FIFO_Idx;  // high_32: denotes whether to free;    low_32: fifo_idx
    std::list<SchedulerTime> TimeList;
};

struct SchedulerTable {

    //sc_bigint<TableWIDTH> HOCC;    // HCU's Occurence
    //sc_bigint<TableWIDTH> ROCC;    // ReductionTree's Occurence
    std::list<SchedulerItem> schedulerItemList;
    std::mutex mtx;

    void addItem(const SchedulerItem& item) {
        schedulerItemList.push_back(item);
    }

    void removeItem(std::function<bool(const SchedulerItem&)> condition) {
        schedulerItemList.remove_if(condition);
    }


};



#endif