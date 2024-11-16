#include "reductionPool.h"


void ReductionController::compute_ROCC() {
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

void ReductionController::arbitratorTwo() {
    while(true) {
        wait();
        if(rst.read()) {
            counter[0] = 0;
        }else {
            for(int i = 0; i < Reduction_USAGE; i ++) {
                if(reduction_done[i].read()) {
                    reductionInput *ri = new reductionInput;
                    sc_int<WIDTH> riNum;
                    int reductionInputArrayIndex, inputNumArrayIndex;
                    bool isIdle = false;
                    if(i>=0 && i <=127) {
                        for(int j = 0; j <= 127; j++) {
                            if(reductionInputArray[j]->num_available()) {
                                reductionInputArrayIndex = j;
                                inputNumArrayIndex = j;
                                isIdle = true;
                                break;
                            }
                        }
                    }
                    if(isIdle) {
                        *ri = reductionInputArray[reductionInputArrayIndex]->read();
                        riNum = notifyArray[inputNumArrayIndex]->read();
                        assert(riNum > 0 && riNum < 128 && "Error: exceeding the reductionTree Boundary!");
                        int j = 0;
                        for(; j < riNum; j ++) {
                            reductionOutArrayToTree[i][j]->write(ri->data[j]);
                        }
                        reductionOutArrayToTree[i][j]->write(riNum);
                        counter[0] = (counter[0] + 1) % 256 - 128; 
                        notifyOutArray[i].write(counter[0]);
                    }
                }                    
            }
        }
    }
}

void ReductionController::arbitratorFour() {
    while(true) {
        wait();
        if(rst.read()) {
            counter[1] = 0;
        }else {
            for(int i = 0; i < Reduction_USAGE; i ++) {
                if(reduction_done[i].read()) {
                    reductionInput *ri = new reductionInput;
                    sc_int<WIDTH> riNum;
                    int reductionInputArrayIndex, inputNumArrayIndex;
                    bool isIdle = false;
                    if(i>=128 && i <=191) {
                        for(int j = 128; j <= 191; j++) {
                            if(reductionInputArray[j]->num_available()) {
                                reductionInputArrayIndex = j;
                                inputNumArrayIndex = j;
                                isIdle = true;
                                break;
                            }
                        }
                    }
                    if(isIdle){
                        *ri = reductionInputArray[reductionInputArrayIndex]->read();
                        riNum = notifyArray[inputNumArrayIndex]->read();
                        assert(riNum > 0 && riNum < 128 && "Error: exceeding the reductionTree Boundary!");
                        int j = 0;
                        for(; j < riNum; j ++) {
                            reductionOutArrayToTree[i][j]->write(ri->data[j]);
                        }
                        reductionOutArrayToTree[i][j]->write(riNum);
                        counter[1] = (counter[1] + 1) % 256 - 128; 
                        notifyOutArray[i].write(counter[1]);
                    }
                }                    
            }
        }
    }
}

void ReductionController::arbitratorEight() {
    while(true) {
        wait();
        if(rst.read()) {
            counter[2] = 0;
        }else {
            for(int i = 0; i < Reduction_USAGE; i ++) {
                if(reduction_done[i].read()) {
                    reductionInput *ri = new reductionInput;
                    sc_int<WIDTH> riNum;
                    int reductionInputArrayIndex, inputNumArrayIndex;
                    bool isIdle = false;
                    if(i>=192 && i <=223) {
                        for(int j = 192; j <= 223; j++) {
                            if(reductionInputArray[j]->num_available()) {
                                reductionInputArrayIndex = j;
                                inputNumArrayIndex = j;
                                isIdle = true;
                                break;
                            }
                        }
                    }
                    if(isIdle){
                        *ri = reductionInputArray[reductionInputArrayIndex]->read();
                        riNum = notifyArray[inputNumArrayIndex]->read();
                        assert(riNum > 0 && riNum < 128 && "Error: exceeding the reductionTree Boundary!");
                        int j = 0;
                        for(; j < riNum; j ++) {
                            reductionOutArrayToTree[i][j]->write(ri->data[j]);
                        }
                        reductionOutArrayToTree[i][j]->write(riNum);
                        counter[2] = (counter[2] + 1) % 256 - 128; 
                        notifyOutArray[i].write(riNum);
                    }
                }                    
            }
        }
    }
}

void ReductionController::arbitratorSixteen() {
    while(true) {
        wait();
        if(rst.read()) {
            counter[3] = 0;
        }else {
            for(int i = 0; i < Reduction_USAGE; i ++) {
                if(reduction_done[i].read()) {
                    reductionInput *ri = new reductionInput;
                    sc_int<WIDTH> riNum;
                    int reductionInputArrayIndex, inputNumArrayIndex;
                    bool isIdle = false;
                    if(i>=224 && i <=239) {
                        for(int j = 224; j <= 239; j++) {
                            if(reductionInputArray[j]->num_available()) {
                                reductionInputArrayIndex = j;
                                inputNumArrayIndex = j;
                                isIdle = true;
                                break;
                            }
                        }
                    }
                    if(isIdle){
                        *ri = reductionInputArray[reductionInputArrayIndex]->read();
                        riNum = notifyArray[inputNumArrayIndex]->read();
                        assert(riNum > 0 && riNum < 128 && "Error: exceeding the reductionTree Boundary!");
                        int j = 0;
                        for(; j < riNum; j ++) {
                            reductionOutArrayToTree[i][j]->write(ri->data[j]);
                        }
                        reductionOutArrayToTree[i][j]->write(riNum);
                        counter[3] = (counter[3] + 1) % 256 - 128; 
                        notifyOutArray[i].write(counter[3]);
                    }
                }                    
            }
        }
    }
}

void ReductionController::arbitratorThirtyTwo() {
    while(true) {
        wait();
        if(rst.read()) {
            counter[4] = 0;
        }else {
            for(int i = 0; i < Reduction_USAGE; i ++) {
                if(reduction_done[i].read()) {
                    reductionInput *ri = new reductionInput;
                    sc_int<WIDTH> riNum;
                    int reductionInputArrayIndex, inputNumArrayIndex;
                    bool isIdle = false;
                    if(i>=240 && i <=247) {
                        for(int j = 240; j <= 247; j++) {
                            if(reductionInputArray[j]->num_available()) {
                                reductionInputArrayIndex = j;
                                inputNumArrayIndex = j;
                                isIdle = true;
                                break;
                            }
                        }
                    }
                    if(isIdle) {
                        *ri = reductionInputArray[reductionInputArrayIndex]->read();
                        riNum = notifyArray[inputNumArrayIndex]->read();
                        assert(riNum > 0 && riNum < 128 && "Error: exceeding the reductionTree Boundary!");
                        int j = 0;
                        for(; j < riNum; j ++) {
                            reductionOutArrayToTree[i][j]->write(ri->data[j]);
                        }
                        reductionOutArrayToTree[i][j]->write(riNum);
                        counter[4] = (counter[4] + 1) % 256 - 128; 
                        notifyOutArray[i].write(counter[4]);
                    }
                }                    
            }
        }
    }
}

void ReductionController::arbitratorSixtyFour() {
    while(true) {
        wait();
        if(rst.read()) {
            counter[5] = 0;
        }else {
            for(int i = 0; i < Reduction_USAGE; i ++) {
                if(reduction_done[i].read()) {
                    reductionInput *ri = new reductionInput;
                    sc_int<WIDTH> riNum;
                    int reductionInputArrayIndex, inputNumArrayIndex;
                    bool isIdle = false;
                    if(i>=248 && i <=251) {
                        for(int j = 248; j <= 251; j++) {
                            if(reductionInputArray[j]->num_available()) {
                                reductionInputArrayIndex = j;
                                inputNumArrayIndex = j;
                                isIdle = true;
                                break;
                            }
                        }
                    }
                    if(isIdle) {
                        *ri = reductionInputArray[reductionInputArrayIndex]->read();
                        riNum = notifyArray[inputNumArrayIndex]->read();
                        assert(riNum > 0 && riNum < 128 && "Error: exceeding the reductionTree Boundary!");
                        int j = 0;
                        for(; j < riNum; j ++) {
                            reductionOutArrayToTree[i][j]->write(ri->data[j]);
                        }
                        reductionOutArrayToTree[i][j]->write(riNum);
                        counter[5] = (counter[5] + 1) % 256 - 128; 
                        notifyOutArray[i].write(counter[5]);
                    }
                }                    
            }
        }
    }
}

void ReductionController::arbitratorOneHundred() {
    while(true) {
        wait();
        if(rst.read()) {
            counter[6] = 0;
        }else {
            for(int i = 0; i < Reduction_USAGE; i ++) {
                if(reduction_done[i].read()) {
                    reductionInput *ri = new reductionInput;
                    sc_int<WIDTH> riNum;
                    int reductionInputArrayIndex, inputNumArrayIndex;
                    bool isIdle = false;
                    if(i>=252 && i <=253) {
                        for(int j = 251; j <= 253; j++) {
                            if(reductionInputArray[j]->num_available()) {
                                reductionInputArrayIndex = j;
                                inputNumArrayIndex = j;
                                isIdle = true;
                                break;
                            }
                        }
                    }
                    if(isIdle) {
                        *ri = reductionInputArray[reductionInputArrayIndex]->read();
                        riNum = notifyArray[inputNumArrayIndex]->read();
                        assert(riNum > 0 && riNum < 128 && "Error: exceeding the reductionTree Boundary!");
                        int j = 0;
                        for(; j < riNum; j ++) {
                            reductionOutArrayToTree[i][j]->write(ri->data[j]);
                        }
                        reductionOutArrayToTree[i][j]->write(riNum);
                        counter[6] = (counter[6] + 1) % 256 - 128; 
                        notifyOutArray[i].write(counter[6]);
                    }
                }                    
            }
        }
    }
}