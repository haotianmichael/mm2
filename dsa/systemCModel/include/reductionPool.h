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
    sc_in<sc_int<WIDTH>> num;  // num of data to be sorted
    sc_signal<reductionInput> vec;
    sc_out<sc_int<WIDTH>> result;
    sc_signal<bool> done;

    void process() {
        while(true) {
            wait();
            if(rst.read()){
                result.write(static_cast<sc_int<WIDTH>>(-1));
                done.write(true);
            }else {
                wait(num.default_event());
                done.write(false);
                std::vector<sc_int<WIDTH>> tmpVec = vec.read().data;
                auto ret = std::max_element(tmpVec.begin(), tmpVec.end());
                if(ret != tmpVec.end()) {
                    wait(static_cast<int>(log2(num.read())), SC_NS);
                    result.write(*ret);
                    done.write(true);
                }else {
                    std::cerr << "Error in reductionPool comparator!" << std::endl;
                }
            }
        }
    }

    SC_CTOR(ReductionTree) {
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
    sc_fifo<reductionInput> *reductionInputArray[Reduction_USAGE];
    sc_fifo<sc_int<WIDTH>> *numArray[Reduction_USAGE];
    std::vector<sc_signal<reductionInput>> reductionOutArray;
    std::vector<sc_signal<sc_int<WIDTH>>> numOutArray;
    std::vector<sc_in<bool>> reduction_done;
    std::vector<sc_out<sc_int<WIDTH>>> ROCC;


    
    void compute_ROCC();
    void arbitratorTwo();
    void arbitratorFour();
    void arbitratorEight();
    void arbitratorSixteen();
    void arbitratorThirtyTwo();
    void arbitratorSixtyFour();
    void arbitratorOneHundred();

    SC_CTOR(ReductionController):
        ROCC(Reduction_KIND),
        reduction_done(Reduction_USAGE),
        reductionOutArray(Reduction_USAGE),
        numOutArray(Reduction_USAGE){

        SC_THREAD(arbitratorTwo);
        sensitive << clk.pos();

        SC_THREAD(arbitratorFour);
        sensitive << clk.pos();

        SC_THREAD(arbitratorEight);
        sensitive << clk.pos();

        SC_THREAD(arbitratorSixteen);
        sensitive << clk.pos();

        SC_THREAD(arbitratorThirtyTwo);
        sensitive << clk.pos();

        SC_THREAD(arbitratorSixtyFour);
        sensitive << clk.pos();

        SC_THREAD(arbitratorOneHundred);
        sensitive << clk.pos();

        SC_THREAD(compute_ROCC);
        sensitive << clk.pos();

        for(int i = 0; i < Reduction_USAGE; i++) {
            reductionInputArray[i] = new sc_fifo<reductionInput>(1);
            numArray[i] = new sc_fifo<sc_int<WIDTH>>(1);
        }
    }

};


#endif