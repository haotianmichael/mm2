#include "rangeCountUnit.h"

// only takeReads at reset signal=1, not every cycle
void RangeCountUnit::takeReadsAndCut() {
    while(true) {
        wait();
        if(!rst.read()) {
            // take one read (actually an anchors' array[r, q, span] of one read)
            std::string filename("data/in4.txt");
            std::ifstream infile(filename);
            std::string line;
            int i = 0, readIdx = 0; 
            /*tmpSignal*/
            int tmpNum[ReadNumProcessedOneTime];
            // different anchorSegs's range > 5000
            while (std::getline(infile, line) && readIdx < ReadNumProcessedOneTime) {
                if(line == "EOR") {
                    anchorNum[readIdx]->write(i);
                    tmpNum[readIdx++] = i;
                    i = 0;
                }else {
                    std::istringstream iss(line);
                    uint val0;
                    int  val1, val2, val3;
                    if (iss >> val0 >> val1 >> val2 >> val3) {
                        tmpAnchorSegNum[readIdx][i] = val0;
                        anchorRi[readIdx][i]->write(val1);
                        tmpAnchorRi[readIdx][i] = val1;
                        anchorW[readIdx][i]->write(val2);
                        anchorQi[readIdx][i]->write(val3);
                        i++;
                    }
                }
            }
            // compute dynamic range and cut
            // cut algorighm is strictly according to the software version.
            int rangeHeuristic[8] = {0, 16, 512, 1024, 2048, 3072, 4096, 5000};
            for(readIdx = 0; readIdx < ReadNumProcessedOneTime; readIdx++){
                for(int j = 0; j < tmpNum[readIdx]; j ++) {
                    if(tmpAnchorSegNum[readIdx][j+1] != tmpAnchorSegNum[readIdx][j]) {
                        //std::cout << tmpAnchorSegNum[readIdx][j] << std::endl;
                        anchorSuccessiveRange[readIdx][j]->write(1);
                        continue;
                    }
                    int end = j + 5000;
                    for(int delta = 0; delta < 8; delta++) {
                        if((j + rangeHeuristic[delta] < tmpNum[readIdx])) {
                            if(tmpAnchorSegNum[readIdx][j+rangeHeuristic[delta]] == tmpAnchorSegNum[readIdx][j]) {
                                    if(tmpAnchorRi[readIdx][j+rangeHeuristic[delta]] - tmpAnchorRi[readIdx][j] > 5000) {
                                            end = j + rangeHeuristic[delta];
                                            break;
                                    }
                            }else {
                                end = j + rangeHeuristic[delta-1];
                                break;
                            }
                        }
                        // FIXME: the last 16 anchor will omit segID
                        if(tmpNum[readIdx] - j <= 16) {
                             end = tmpNum[readIdx] - 1; 
                        }
                    }
                    while(end > j && tmpAnchorRi[readIdx][end] - tmpAnchorRi[readIdx][j] > 5000) {
                        end--;
                    } 
                    /*
                        1. range of anchor[j]'s successor is [j + 1, end]
                        2. cut when rangeLength = 1
                        3. max range of a segment is its UpperBound
                    */
                    anchorSuccessiveRange[readIdx][j]->write(end-j+1);
                    //std::cout << end - j + 1  << " " << j<< std::endl;
                }
            }
            cutDone.write(true);
        }else {
            cutDone.write(false);
            anchorNum[0]->write(0);
            for(int readIdx = 0; readIdx < ReadNumProcessedOneTime; readIdx++) {
                for(int i = 0; i < MAX_READ_LENGTH; i ++) {
                    anchorSuccessiveRange[readIdx][i]->write(-1);
                }
            }
        }
    }

}


