#ifndef SCHEDULER_TABLE_H
#define SCHEDULER_TABLE_H

#include <helper.h>
#include <list>
#include <mutex>

struct ram_dataForLong{
   sc_int<WIDTH> Rdata[MAX_SEGLENGTH]; 
   sc_int<WIDTH> Qdata[MAX_SEGLENGTH]; 
   sc_int<WIDTH> Idx[MAX_SEGLENGTH]; 
   sc_int<WIDTH> FinalScore[MAX_SEGLENGTH];
   sc_int<WIDTH> FinalScoreIdx = 0;
   sc_int<WIDTH> Wdata;
   sc_int<WIDTH> readID;
   sc_int<WIDTH> segID;
   sc_int<WIDTH> UpperBound;
   bool used;
   bool allocated;
};

struct ram_dataForShort{
   sc_int<WIDTH> Rdata[MIN_SEGLENGTH]; 
   sc_int<WIDTH> Qdata[MIN_SEGLENGTH]; 
   sc_int<WIDTH> Idx[MIN_SEGLENGTH]; 
   sc_int<WIDTH> FinalScore[MIN_SEGLENGTH];
   sc_int<WIDTH> Wdata;
   sc_int<WIDTH> readID;
   sc_int<WIDTH> segID;
   sc_int<WIDTH> UpperBound;
   bool used;
   bool allocated;
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

    std::list<SchedulerItem> schedulerItemList;
    std::mutex mtx;

    // for chain_rt_allocate
    sc_int<WIDTH> output[HCU_NUM];
    sc_int<WIDTH> outputPre[HCU_NUM];
    sc_int<WIDTH>  address;
    sc_int<WIDTH> id;
    int high_32;
    int low_32;
    int index;
    bool enable;

    void addItem(const SchedulerItem& item) {
        schedulerItemList.push_back(item);
    }

    void removeItem(std::function<bool(const SchedulerItem&)> condition) {
        schedulerItemList.remove_if(condition);
    }


};



#endif