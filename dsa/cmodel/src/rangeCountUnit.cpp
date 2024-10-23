#include "rangeCountUnit.h"

// only takeReads at reset signal=1, not every cycle
void RangeCountUnit::takeReadsAndCut() {

    if(!rst.read()) {
        // take one read (actually an anchors' array[r, q, span] of one read)
        std::string filename("data/in2.txt");
        std::ifstream infile(filename);
        std::string line;
        int i = 0, readIdx = 0; 
        while (std::getline(infile, line) && readIdx < ReadNumProcessedOneTime) {
            if(line == "EOR") {
                anchorNum[readIdx++].write(i);
                i = 0;
            }else {
                std::istringstream iss(line);
                int val1, val2, val3;
                if (iss >> val1 >> val2 >> val3) {
                    anchorRi[readIdx][i].write(val1);
                    anchorW[readIdx][i].write(val2);
                    anchorQi[readIdx][i].write(val3);
                    i++;
                }
            }
        }
 
        // compute dynamic range and cut
        // cut algorighm is strictly according to the software version.
        int rangeHeuristic[8] = {0, 16, 512, 1024, 2048, 3072, 4096, 5000};
        for(readIdx = 0; readIdx < ReadNumProcessedOneTime; readIdx++){
            for(int j = 0; j < anchorNum[readIdx].read(); j ++) {
                int end = j + 5000;
                for(int delta = 0; delta < 8; delta++) {
                    if((j + rangeHeuristic[delta] >= anchorNum[readIdx].read()) 
                        || (anchorRi[readIdx][j+rangeHeuristic[delta]].read() - anchorRi[readIdx][j].read() > 5000)) {
                        end = j + rangeHeuristic[delta];
                        break;                    
                    }
                }
                while(end > j) {
                    if(end >= anchorNum[readIdx].read() || anchorRi[readIdx][end].read() - anchorRi[readIdx][j].read() > 5000) {
                        end--;
                    } 
                }
                /*
                    1. range of anchor[j]'s successor is [j + 1, end]
                    2. cut when rangeLength = 1
                    3. max range of a segment is its UpperBound
                */
                anchorSuccessiveRange[readIdx][j].write(end-j+1);
            }
        }
        cutDone.write(true);
    }else {
        cutDone.write(false);
        anchorNum[0].write(0);
        for(int readIdx = 0; readIdx < ReadNumProcessedOneTime; readIdx++) {
            for(int i = 0; i < MAX_READ_LENGTH; i ++) {
                anchorSuccessiveRange[readIdx][i].write(-1);
            }
        }
    }
       

}