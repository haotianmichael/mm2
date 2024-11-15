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
        }else {
            for(int i = 0; i < Reduction_USAGE; i ++) {
                if(reduction_done[i].read()) {
                    reductionInput *ri = new reductionInput;
                    sc_int<WIDTH> riNum;
                    int reductionInputArrayIndex, inputNumArrayIndex;
                    if(i>=0 && i <=127) {
                        for(int j = 0; j <= 127; j++) {
                            if(reductionInputArray[j]->num_available()) {
                                reductionInputArrayIndex = j;
                                inputNumArrayIndex = j;
                                break;
                            }
                        }
                    }
                    *ri = reductionInputArray[reductionInputArrayIndex]->read();
                    riNum = numArray[inputNumArrayIndex]->read();
                    reductionOutArray[i].write(*ri);
                    numOutArray[i].write(riNum);
                }                    
            }
        }
    }
}

void ReductionController::arbitratorFour() {
    while(true) {
        wait();
        if(rst.read()) {
        }else {
            for(int i = 0; i < Reduction_USAGE; i ++) {
                if(reduction_done[i].read()) {
                    reductionInput *ri = new reductionInput;
                    sc_int<WIDTH> riNum;
                    int reductionInputArrayIndex, inputNumArrayIndex;
                    if(i>=128 && i <=191) {
                        for(int j = 128; j <= 191; j++) {
                            if(reductionInputArray[j]->num_available()) {
                                reductionInputArrayIndex = j;
                                inputNumArrayIndex = j;
                                break;
                            }
                        }
                    }
                    *ri = reductionInputArray[reductionInputArrayIndex]->read();
                    riNum = numArray[inputNumArrayIndex]->read();
                    reductionOutArray[i].write(*ri);
                    numOutArray[i].write(riNum);
                }                    
            }
        }
    }
}

void ReductionController::arbitratorEight() {
    while(true) {
        wait();
        if(rst.read()) {
        }else {
            for(int i = 0; i < Reduction_USAGE; i ++) {
                if(reduction_done[i].read()) {
                    reductionInput *ri = new reductionInput;
                    sc_int<WIDTH> riNum;
                    int reductionInputArrayIndex, inputNumArrayIndex;
                    if(i>=192 && i <=223) {
                        for(int j = 192; j <= 223; j++) {
                            if(reductionInputArray[j]->num_available()) {
                                reductionInputArrayIndex = j;
                                inputNumArrayIndex = j;
                                break;
                            }
                        }
                    }
                    *ri = reductionInputArray[reductionInputArrayIndex]->read();
                    riNum = numArray[inputNumArrayIndex]->read();
                    reductionOutArray[i].write(*ri);
                    numOutArray[i].write(riNum);
                }                    
            }
        }
    }
}

void ReductionController::arbitratorSixteen() {
    while(true) {
        wait();
        if(rst.read()) {
        }else {
            for(int i = 0; i < Reduction_USAGE; i ++) {
                if(reduction_done[i].read()) {
                    reductionInput *ri = new reductionInput;
                    sc_int<WIDTH> riNum;
                    int reductionInputArrayIndex, inputNumArrayIndex;
                    if(i>=224 && i <=239) {
                        for(int j = 224; j <= 239; j++) {
                            if(reductionInputArray[j]->num_available()) {
                                reductionInputArrayIndex = j;
                                inputNumArrayIndex = j;
                                break;
                            }
                        }
                    }
                    *ri = reductionInputArray[reductionInputArrayIndex]->read();
                    riNum = numArray[inputNumArrayIndex]->read();
                    reductionOutArray[i].write(*ri);
                    numOutArray[i].write(riNum);
                }                    
            }
        }
    }
}

void ReductionController::arbitratorThirtyTwo() {
    while(true) {
        wait();
        if(rst.read()) {
        }else {
            for(int i = 0; i < Reduction_USAGE; i ++) {
                if(reduction_done[i].read()) {
                    reductionInput *ri = new reductionInput;
                    sc_int<WIDTH> riNum;
                    int reductionInputArrayIndex, inputNumArrayIndex;
                    if(i>=240 && i <=247) {
                        for(int j = 240; j <= 247; j++) {
                            if(reductionInputArray[j]->num_available()) {
                                reductionInputArrayIndex = j;
                                inputNumArrayIndex = j;
                                break;
                            }
                        }
                    }
                    *ri = reductionInputArray[reductionInputArrayIndex]->read();
                    riNum = numArray[inputNumArrayIndex]->read();
                    reductionOutArray[i].write(*ri);
                    numOutArray[i].write(riNum);
                }                    
            }
        }
    }
}

void ReductionController::arbitratorSixtyFour() {
    while(true) {
        wait();
        if(rst.read()) {
        }else {
            for(int i = 0; i < Reduction_USAGE; i ++) {
                if(reduction_done[i].read()) {
                    reductionInput *ri = new reductionInput;
                    sc_int<WIDTH> riNum;
                    int reductionInputArrayIndex, inputNumArrayIndex;
                    if(i>=248 && i <=251) {
                        for(int j = 248; j <= 251; j++) {
                            if(reductionInputArray[j]->num_available()) {
                                reductionInputArrayIndex = j;
                                inputNumArrayIndex = j;
                                break;
                            }
                        }
                    }
                    *ri = reductionInputArray[reductionInputArrayIndex]->read();
                    riNum = numArray[inputNumArrayIndex]->read();
                    reductionOutArray[i].write(*ri);
                    numOutArray[i].write(riNum);
                }                    
            }
        }
    }
}

void ReductionController::arbitratorOneHundred() {
    while(true) {
        wait();
        if(rst.read()) {
        }else {
            for(int i = 0; i < Reduction_USAGE; i ++) {
                if(reduction_done[i].read()) {
                    reductionInput *ri = new reductionInput;
                    sc_int<WIDTH> riNum;
                    int reductionInputArrayIndex, inputNumArrayIndex;
                    if(i>=252 && i <=253) {
                        for(int j = 251; j <= 253; j++) {
                            if(reductionInputArray[j]->num_available()) {
                                reductionInputArrayIndex = j;
                                inputNumArrayIndex = j;
                                break;
                            }
                        }
                    }
                    *ri = reductionInputArray[reductionInputArrayIndex]->read();
                    riNum = numArray[inputNumArrayIndex]->read();
                    reductionOutArray[i].write(*ri);
                    numOutArray[i].write(riNum);
                }                    
            }
        }
    }
}