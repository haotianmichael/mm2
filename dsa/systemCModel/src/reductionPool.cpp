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

void ReductionController::arbitrator() {
    while(true) {
        wait();
        if(rst.read()) {
            for(int i = 0; i < Reduction_USAGE; i ++) {
                fifoIdxArray[i].write(static_cast<sc_int<WIDTH>>(-1));
                notifyOutArray[i].write(false);
                for(int j = 0; j < Reduction_NUM; j ++) {
                    reductionOutArrayToTree[i][j]->write(static_cast<sc_int<WIDTH>>(-1));
                }
            }
            /* -2 deotes: not porting 
               -1 denotes: not dispatching
              >=0 denotes: reducting
            */
            for(int i = 0; i < Reduction_FIFO_NUM; i ++) {
                notifyArrayPorts[i]->write(static_cast<sc_int<WIDTH>>(-2));
            }
        }else {
            for(int i = 0; i < Reduction_FIFO_NUM; i ++) {
                reductionInput *ri = new reductionInput;
                sc_int<WIDTH> riNum;
                if(notifyArrayPorts[i]->read() >= 0) {
                    int j = notifyArrayPorts[i]->read().to_int();
                    if(reduction_done[j].read()) {
                        assert(reductionInputArrayPorts[i]->num_available()==0 && "Error: FIFO's data not processed!");
                        notifyArrayPorts[i]->write(static_cast<sc_int<WIDTH>>(-2));
                        notifyOutArray[i].write(false);
                        fifoIdxArray[i].write(static_cast<sc_int<WIDTH>>(-1));
                    }else {
                        *ri = reductionInputArrayPorts[i]->read();
                        riNum = ri->data.back();
                        assert(fifoIdxArray[j].read()==i && "Error: unknow error about FIFOIx and ReductionIdx!"); 
                        for(int t = 0; t < riNum - 1; t++) {
                            reductionOutArrayToTree[j][t]->write(ri->data[t]);
                        }
                        notifyOutArray[j].write(true);
                    }
                }else if(notifyArrayPorts[i]->read() == -1) {
                    *ri = reductionInputArrayPorts[i]->read();
                    riNum = ri->data.back();
                    if(riNum >= 1 && riNum <= 2) {
                        bool dispatchS = false;
                        int j = 0;
                        for(; j <= 127; j ++) {
                            if(reduction_done[j].read()) {
                                fifoIdxArray[j].write(static_cast<sc_int<WIDTH>>(i));
                                for(int t = 0; t < riNum - 1; t++) {
                                    reductionOutArrayToTree[j][t]->write(ri->data[t]);
                                }
                                notifyOutArray[j].write(true);
                            } 
                            break;
                            dispatchS = true;
                        }
                        if(dispatchS) {
                            notifyArrayPorts[i]->write(static_cast<sc_int<WIDTH>>(j));
                        }
                    }else if(riNum >= 3 && riNum <= 4) {
                        bool dispatchS = false;
                        int j = 128;
                        for(; j <= 191; j ++) {
                            if(reduction_done[j].read()) {
                                fifoIdxArray[j].write(static_cast<sc_int<WIDTH>>(i));
                                for(int t = 0; t < riNum - 1; t++) {
                                    reductionOutArrayToTree[j][t]->write(ri->data[t]);
                                }
                                notifyOutArray[j].write(true);
                            } 
                            break;
                            dispatchS = true;
                        }
                        if(dispatchS) {
                            notifyArrayPorts[i]->write(static_cast<sc_int<WIDTH>>(j));
                        }
                    }else if(riNum >= 5 && riNum <= 8) {
                        bool dispatchS = false;
                        int j = 192;
                        for(; j <= 223; j ++) {
                            if(reduction_done[j].read()) {
                                fifoIdxArray[j].write(static_cast<sc_int<WIDTH>>(i));
                                for(int t = 0; t < riNum - 1; t++) {
                                    reductionOutArrayToTree[j][t]->write(ri->data[t]);
                                }
                                notifyOutArray[j].write(true);
                            } 
                            break;
                            dispatchS = true;
                        }
                        if(dispatchS) {
                            notifyArrayPorts[i]->write(static_cast<sc_int<WIDTH>>(j));
                        }
                    }else if(riNum >= 9 && riNum <= 16) {
                        bool dispatchS  = false;
                        int j = 224;
                        for(; j <= 239; j ++) {
                            if(reduction_done[j].read()) {
                                fifoIdxArray[j].write(static_cast<sc_int<WIDTH>>(i));
                                for(int t = 0; t < riNum - 1; t++) {
                                    reductionOutArrayToTree[j][t]->write(ri->data[t]);
                                }
                                notifyOutArray[j].write(true);
                            } 
                            break;
                            dispatchS = true;
                        }
                        if(dispatchS) {
                            notifyArrayPorts[i]->write(static_cast<sc_int<WIDTH>>(j));
                        }
                    }else if(riNum >= 17 && riNum <= 32) {
                        bool dispatchS = false;
                        int j = 240;
                        for(; j <= 247; j ++) {
                            if(reduction_done[j].read()) {
                                fifoIdxArray[j].write(static_cast<sc_int<WIDTH>>(i));
                                for(int t = 0; t < riNum - 1; t++) {
                                    reductionOutArrayToTree[j][t]->write(ri->data[t]);
                                }
                                notifyOutArray[j].write(true);
                            } 
                            break;
                            dispatchS = true;
                        }
                        if(dispatchS) {
                            notifyArrayPorts[i]->write(static_cast<sc_int<WIDTH>>(j));
                        }
                    }else if(riNum >= 33 && riNum <= 64) {
                        bool dispatchS = false;
                        int j = 248;
                        for(; j <= 251; j ++) {
                            if(reduction_done[j].read()) {
                                fifoIdxArray[j].write(static_cast<sc_int<WIDTH>>(i));
                                for(int t = 0; t < riNum - 1; t++) {
                                    reductionOutArrayToTree[j][t]->write(ri->data[t]);
                                }
                                notifyOutArray[j].write(true);
                            } 
                            break;
                            dispatchS = true;
                        }
                        if(dispatchS) {
                            notifyArrayPorts[i]->write(static_cast<sc_int<WIDTH>>(j));
                        }
                    }else if(riNum >= 65 && riNum <= 128) {
                        bool dispatchS = false;
                        int j = 252;
                        for(; j <= 253; j ++) {
                            if(reduction_done[j].read()) {
                                fifoIdxArray[j].write(static_cast<sc_int<WIDTH>>(i));
                                for(int t = 0; t < riNum - 1; t++) {
                                    reductionOutArrayToTree[j][t]->write(ri->data[t]);
                                }
                                notifyOutArray[j].write(true);
                            } 
                            break;
                            dispatchS = true;
                        }
                        if(dispatchS) {
                            notifyArrayPorts[i]->write(static_cast<sc_int<WIDTH>>(j));
                        }
                    }else {    // riNum == 0

                    }
                }
            }
        }
    }
}