#ifndef REDUCTION_POOL_H
#define REDUCTION_POOL_H

#include "helper.h"


struct reductionInput {
    std::vector<sc_int<WIDTH>> data;
    reductionInput () : data(Reduction_NUM) {}
    friend std::ostream& operator<<(std::ostream& os, const reductionInput& segment){
        os << "reductionInput: ";
        for(int i =0; i < MAX_SEGLENGTH; i ++) {
            os << segment.data[i] << " "; 
        }
        return os;
    }

    bool operator==(const reductionInput& other) {
        return data == other.data;
    }
};
SC_MODULE(ReductionTree) {

    sc_in<bool> clk, rst;
    sc_in<sc_int<WIDTH>> fifoIdx;
    sc_in<bool> vecNotify;  // notify signal
    std::vector<sc_in<sc_int<WIDTH>>> vecFromController;  // inputArray - the last ele is the length of inputArray
    sc_signal<sc_int<WIDTH>> result;
    sc_signal<bool> done;

    void process() {
        while(true) {
            wait();
            if(rst.read()){
                result.write(static_cast<sc_int<WIDTH>>(-1));
                done.write(true);
            }else {
                if(vecNotify.read()) {
                    done.write(false);
                    int tmpVec[Reduction_NUM];
                    int num = vecFromController.back().read();
                    int res = -1;
                    for(int i = 0; i < num - 1; i ++) {
                        tmpVec[i] = vecFromController[i].read();
                        if(tmpVec[i] > res) {
                            res = tmpVec[i];
                        }
                    }
                    result.write(res);
                }else {
                    //wait(static_cast<int>(log2(tmpVec.back())), SC_NS);
                    done.write(true); 
                    result.write(static_cast<sc_int<WIDTH>>(-1));
                }
            }
        }
    }

    SC_CTOR(ReductionTree) :
       vecFromController(Reduction_NUM){
       SC_THREAD(process); 
       sensitive << clk.pos();
    }

};

SC_MODULE(ReductionController) {

    /*
        7 kinds of Pipelined-ReductionTree, totally 254 results/cycle

        cycle                           num

        2-path Reduction                128
        4-path Reduction                64
        8-path Reduction                32
        ...
        64-path Reduciton               4
        128-path Reduction              2
    */
    sc_in<bool> clk, rst;
    sc_in<bool> start;
    sc_port<sc_fifo_in_if<reductionInput>> reductionInputArrayPorts[Reduction_FIFO_NUM];
    sc_in<sc_int<WIDTH>> notifyArray[Reduction_FIFO_NUM];
    std::vector<sc_signal<sc_int<WIDTH>>> notifyArrayToScheduler;
    std::vector<sc_signal<bool>> notifyOutArray;
    std::vector<sc_in<bool>> reduction_done;
    std::vector<sc_signal<sc_int<WIDTH>>> fifoIdxArray;
    std::vector<sc_signal<sc_int<WIDTH>>> ROCC;
    std::vector<std::vector<sc_signal<sc_int<WIDTH>>*>> reductionOutArrayToTree;

    
    void compute_ROCC();
    void arbitrator();

    SC_CTOR(ReductionController):
        ROCC(Reduction_KIND),
        reduction_done(Reduction_USAGE),
        fifoIdxArray(Reduction_USAGE),
        notifyOutArray(Reduction_USAGE),
        notifyArrayToScheduler(Reduction_FIFO_NUM),
        reductionOutArrayToTree(Reduction_USAGE, std::vector<sc_signal<sc_int<WIDTH>>*>(Reduction_NUM)){

        SC_THREAD(arbitrator);
        sensitive << clk.pos();

        SC_THREAD(compute_ROCC);
        sensitive << clk.pos();
        for(int i = 0; i < Reduction_USAGE; i++) {
            for(int j = 0; j < Reduction_NUM; j ++) {
                reductionOutArrayToTree[i][j] = new sc_signal<sc_int<WIDTH>>();
            }
        }
    }

};


#endif