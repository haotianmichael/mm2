#ifndef RC_UNIT_H
#define RC_UNIT_H

#include <systemc.h>
#include <vector>
#include "hcu.h"

SC_MODULE(RangeCountUnit) {

    sc_in<bool> rst;
    /* 
        @max read length: 30000
        @max segments length: 5000
    */
    sc_out<bool> readDone;
    std::vector<sc_out<sc_int<WIDTH>>*> anchorNum;
    std::vector<std::vector<sc_out<sc_int<WIDTH>>*>> anchorRi;
    std::vector<std::vector<sc_out<sc_int<WIDTH>>*>> anchorQi;
    std::vector<std::vector<sc_out<sc_int<WIDTH>>*>> anchorIdx;
    std::vector<sc_out<sc_int<WIDTH>> *> anchorW;
    std::vector<std::vector<sc_out<sc_uint<WIDTH>>*>> anchorSegNum;


    void takeReadsByOneBatch();
    SC_CTOR(RangeCountUnit) : 
        anchorNum(ReadNumProcessedOneTime),
        anchorW(ReadNumProcessedOneTime),
        anchorRi(ReadNumProcessedOneTime, std::vector<sc_out<sc_int<WIDTH>>*>(MAX_READ_LENGTH, nullptr)),
        anchorQi(ReadNumProcessedOneTime, std::vector<sc_out<sc_int<WIDTH>>*>(MAX_READ_LENGTH, nullptr)),
        anchorIdx(ReadNumProcessedOneTime, std::vector<sc_out<sc_int<WIDTH>>*>(MAX_READ_LENGTH, nullptr)),
        anchorSegNum(ReadNumProcessedOneTime, std::vector<sc_out<sc_uint<WIDTH>>*>(MAX_READ_LENGTH, nullptr)) {

        for(int i = 0; i < ReadNumProcessedOneTime; i ++) {
            anchorNum[i] = new sc_out<sc_int<WIDTH>>();
            anchorW[i] = new sc_out<sc_int<WIDTH>>();
            for(int j = 0; j < MAX_READ_LENGTH; j ++) {
                anchorRi[i][j] = new sc_out<sc_int<WIDTH>>();
                anchorQi[i][j] = new sc_out<sc_int<WIDTH>>();
                anchorIdx[i][j] = new sc_out<sc_int<WIDTH>>();
                anchorSegNum[i][j] = new sc_out<sc_uint<WIDTH>>();
            }
        }

        SC_THREAD(takeReadsByOneBatch);
        sensitive << rst;
    }

};

#endif