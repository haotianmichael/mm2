#include "readFromDDR.h"

// only takeReads at reset signal=1, not every cycle
void ReadFromDDR::takeReadsByOneBatch() {
    while(true) {
        wait();
        if(!rst.read()) {
            // take one read (actually an anchors' array[r, q, span] of one read)
            std::string filename("data/in4.txt");
            std::ifstream infile(filename);
            std::string line;
            int i = 0, readIdx = 0; 
            // different anchorSegs's range > 5000
            while (std::getline(infile, line) && readIdx < ReadNumProcessedOneTime) {
                if(line == "EOR") {
                    anchorNum[readIdx++]->write(i);
                    i = 0;
                }else {
                    std::istringstream iss(line);
                    uint val0;
                    int  val1, val2, val3;
                    if (iss >> val0 >> val1 >> val2 >> val3) {
                        anchorSegNum[readIdx][i]->write(val0);
                        anchorRi[readIdx][i]->write(val1);
                        anchorIdx[readIdx][i]->write(i);
                        anchorW[readIdx]->write(val2);
                        anchorQi[readIdx][i]->write(val3);
                        i++;
                    }
                }
            }
            readDone.write(true);
        }else {
            readDone.write(false);
            anchorNum[0]->write(0);
        }
    }

}


