#ifndef REDUCTION_POOL_H
#define REDUCTION_POOL_H

#include "helper.h"


SC_MODULE(ReductionTree) {

    sc_in<bool> clk, rst;
    sc_in<sc_int<WIDTH>> inputNum;  // num of data to be sorted
    std::vector<sc_in<sc_int<WIDTH>>> vec;
    sc_out<sc_int<WIDTH>> result;
    sc_signal<bool> done;

    void process() {
        while(true) {
            wait();
            if(rst.read()){
                result.write(static_cast<sc_int<WIDTH>>(-1));
                done.write(false);
            }else {
                wait(inputNum.default_event());
                done.write(false);
                auto ret = std::max_element(vec.begin(), vec.end());
                if(ret != vec.end()) {
                    wait(static_cast<int>(log2(inputNum.read())), SC_NS);
                    result.write((*ret).read());
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
    std::vector<std::vector<sc_signal<sc_int<WIDTH>>*>> reductionInputArray;
    std::vector<sc_in<sc_int<WIDTH>>> inputNumArray;
    std::vector<sc_in<bool>> reduction_done;
    std::vector<sc_out<sc_int<WIDTH>>> ROCC;


    int width = 2;
    int index = 0;

    void compute_ROCC() {
        while(true) {
            wait();
            if(rst.read()) {
                /*number of each kind of reductionTree*/
                ROCC[0].write(128);
                ROCC[1].write(64);
                ROCC[2].write(32);
                ROCC[3].write(16);
                ROCC[4].write(8);
                ROCC[5].write(4);
                ROCC[6].write(2);
            }else {
                int twoP = 0, fourP = 0, eightP = 0, sixteP = 0, thirtyP = 0, sixtyP = 0, hundredsP = 0;
                for(int i = 0; i < 128; i ++) {
                    if(reduction_done[i].read() == true) {
                        twoP++;  // two-path
                    }
                }
                for(int i = 128; i < 192; i ++) {
                    if(reduction_done[i].read() == true) {
                        fourP++;
                    }
                }
                for(int i = 192; i < 224; i ++) {
                    if(reduction_done[i].read() == true) {
                        eightP++;
                    }
                }
                for(int i = 224; i < 240; i ++) {
                    if(reduction_done[i].read() == true) {
                        sixteP++;
                    }
                }
                for(int i = 240; i < 248; i ++) {
                    if(reduction_done[i].read() == true) {
                        thirtyP++;
                    }
                }
                for(int i = 248; i < 252; i ++) {
                    if(reduction_done[i].read() == true) {
                        sixtyP++;
                    }
                }
                for(int i = 252; i < 254; i ++) {
                    if(reduction_done[i].read() == true) {
                        hundredsP++;
                    }
                }
                ROCC[0].write(twoP);
                ROCC[1].write(fourP);
                ROCC[2].write(eightP);
                ROCC[3].write(sixteP);
                ROCC[4].write(thirtyP);
                ROCC[5].write(sixtyP);
                ROCC[6].write(hundredsP);
            }
        }
    }

    /*base on the reductionInputArray, fill the reductionNum*/
    void arbitrator() {
        while(true) {
            wait();
            if(rst.read()) {
                width = 2;
                index = 0;
            }else {

               // while(width < inputNum.read()) {
                //    width *= 2;
                 //   index ++;
               // }

            
            }
        }
    }

    SC_CTOR(ReductionController):
        ROCC(Reduction_KIND),
        reduction_done(Reduction_USAGE),
        inputNumArray(Reduction_USAGE),
        reductionInputArray(Reduction_USAGE, std::vector<sc_signal<sc_int<WIDTH>>*>(Reduction_NUM)) {

        SC_THREAD(arbitrator);
        sensitive << clk.pos();

        SC_THREAD(compute_ROCC);
        sensitive << clk.pos();

        for(int i = 0; i < Reduction_USAGE; i++) {
            for(int j = 0; j < Reduction_NUM; j ++) {
                try{
                    reductionInputArray[i][j] = new sc_signal<sc_int<WIDTH>>();
                }catch(const std::exception& e){
                    std::cerr << e.what() << "Memeory Error: reductionController!" << std::endl;
                }
            }
        }
    }

};


#endif