#ifndef PARTIAL_SCORE_QUEUE_H
#define PARTIAL_SCORE_QUEUE_H

#include "helper.h"
#include "reductionPool.h"

struct PSTable {

    sc_int<WIDTH> segID;
    sc_int<WIDTH> NumUB;
    //sc_int<TableWIDTH> REQ;
    sc_int<WIDTH> Priority;
};


SC_MODULE(PartialScoreQueue) {


    // FIFO and fill the PSQTable
    sc_in<bool> clk, rst;
    std::vector<sc_in<sc_int<WIDTH>>> hcuPoolOut;
    sc_port<sc_fifo_out_if<reductionInput>> *ToReductionInputPorts[Reduction_USAGE];
    sc_port<sc_fifo_out_if<sc_int<WIDTH>>> *ToNotifyPorts[Reduction_USAGE];


    PSTable psTable[MAX_SEG_NUM];   
    SC_CTOR(PartialScoreQueue) :
        hcuPoolOut(HCU_NUM*2){
            std::stringstream r_name, n_name;
            for(int i = 0; i < Reduction_USAGE; i ++) {
                r_name << "ToReductionInputPorts(" << i << ")";
                n_name << "ToNotifyPorts(" << i << ")";
                ToReductionInputPorts[i] = new sc_port<sc_fifo_out_if<reductionInput>>(r_name.str().c_str());
                ToNotifyPorts[i] = new sc_port<sc_fifo_out_if<sc_int<WIDTH>>>(n_name.str().c_str());
                r_name.str("");
                n_name.str("");
            }
    }
};

#endif