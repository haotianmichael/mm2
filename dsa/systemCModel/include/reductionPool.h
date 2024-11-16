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
    sc_in<sc_int<WIDTH>> vecNotify;  // notify signal
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
                wait(vecNotify.default_event());
                done.write(false);
                std::vector<sc_int<WIDTH>> tmpVec;
                for(auto ele : vecFromController) {
                    tmpVec.push_back(ele.read());
                }
                auto ret = std::max_element(tmpVec.begin(), tmpVec.end()-1);
                if(ret != tmpVec.end()) {
                    wait(static_cast<int>(log2(tmpVec.back())), SC_NS);
                    result.write(*ret);
                    done.write(true);
                }else {
                    std::cerr << "Error in reductionPool comparator!" << std::endl;
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
    sc_port<sc_fifo_in_if<reductionInput>> *reductionInputArrayPorts[Reduction_USAGE];
    sc_port<sc_fifo_in_if<sc_int<WIDTH>>> *notifyArrayPorts[Reduction_USAGE];
    std::vector<sc_signal<sc_int<WIDTH>>> notifyOutArray;
    std::vector<sc_in<bool>> reduction_done;
    std::vector<sc_signal<sc_int<WIDTH>>> ROCC;
    std::vector<std::vector<sc_signal<sc_int<WIDTH>>*>> reductionOutArrayToTree;
    int counter[Reduction_KIND];

    
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
        notifyOutArray(Reduction_USAGE),
        reductionOutArrayToTree(Reduction_USAGE, std::vector<sc_signal<sc_int<WIDTH>>*>(Reduction_NUM)){

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
        std::stringstream r_name, n_name;
        for(int i = 0; i < Reduction_USAGE; i++) {
            r_name << "reductionInputArrayPorts(" << i << ")";
            n_name << "notifyArrayPorts(" << i << ")";
            reductionInputArrayPorts[i] = new sc_port<sc_fifo_in_if<reductionInput>>(r_name.str().c_str());
            notifyArrayPorts[i] = new sc_port<sc_fifo_in_if<sc_int<WIDTH>>>(n_name.str().c_str());
            for(int j = 0; j < Reduction_NUM; j ++) {
                reductionOutArrayToTree[i][j] = new sc_signal<sc_int<WIDTH>>();
            }
            r_name.str("");
            n_name.str("");
        }
    }

};


#endif