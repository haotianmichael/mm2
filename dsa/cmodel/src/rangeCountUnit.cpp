#include "rangeCountUnit.h"

void RangeCountUnit::takeOneReadAndCut() {

    if(!rst.read()) {
        // take one read
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
        read_length.write(i);
 
        // cut
        // 算每一个anchor的range，如果=0就切
        //每一个segment的长度是不是就是它的UpperBound

        cutDone.write(true);
    }else {
        cutDone.write(false);
        read_length.write(0);
    }
       

}