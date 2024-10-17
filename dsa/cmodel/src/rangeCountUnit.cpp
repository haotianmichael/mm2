#include "rangeCountUnit.h"

void RangeCountUnit::takeOneReadAndCut() {

    if(!rst.read()) {
        // take one read (actually an anchors' array[r, q, span] of one read)
        std::string filename("data/in2.txt");
        std::ifstream infile(filename);
        std::string line;
        int i = 0; 
        while (std::getline(infile, line)) {
             std::istringstream iss(line);
             int val1, val2, val3;
             if (iss >> val1 >> val2 >> val3) {
                anchorRi[i].write(val1);
                anchorW[i].write(val2);
                anchorQi[i].write(val3);
                i++;
             }
        }
        anchorNum.write(i);
 
        // compute dynamic range and cut
        int rangeHeuristic[8] = {0, 16, 512, 1024, 2048, 3072, 4096, 5000};
        for(int j = 0; j < anchorNum.read(); j ++) {
            int end = j + 5000;
            for(int delta = 0; delta < 8; delta++) {
                if(anchorRi[j+rangeHeuristic[delta]].read() - anchorRi[j].read() > 5000) {
                    end = j + rangeHeuristic[delta];
                    break;                    
                }
            }
            while(end > j) {
                if(anchorRi[end].read() - anchorRi[j].read() > 5000) {
                    end--;
                } 
            }
            /*
                1. range of anchor[j]'s successor is [j + 1, end]
                2. cut when rangeLength = 0
                3. max range of a segment is its UpperBound
            */
            anchorSuccessiveRange[j] = end - j + 1;
        }
        cutDone.write(true);
    }else {
        cutDone.write(false);
        anchorNum.write(0);
    }
       

}