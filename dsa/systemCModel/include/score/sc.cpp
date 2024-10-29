#include "sc.h"

void ScCompute::rXY() {
    while(true) {
        wait();
        if(rst.read()) {
            tmpRXY.write(0);
        }else {
            tmpRXY.write(riX.read() - riY.read());
        }
    }
}


void ScCompute::rYX() {
    while(true) {
        wait();
        if(rst.read()) {
            tmpRYX.write(0);
        }else {
            tmpRYX.write(riY.read() - riX.read());
        }
    }
}

void ScCompute::gapR() {
    while(true) {
       wait();
       if(rst.read()) {
            diffR.write(0);
       }else {
            diffR.write((riX.read() > riY.read()) ? tmpRXY.read() : tmpRYX.read());
       } 
    }    
}

void ScCompute::qXY() {
    while(true) {
        wait();
        if(rst.read()) {
            tmpQXY.write(0); 
        }else {
            tmpQXY.write(qiX.read() - qiY.read()); 
        }
    }
}

void ScCompute::qYX() {
    while(true) {
        wait();
        if(rst.read()) {
            tmpQYX.write(0);
        }else {
            tmpQYX.write(qiY.read() - qiY.read());
        }
    }
}

void ScCompute::gapQ() {
    while(true) {
        wait();
        if(rst.read()) {
            diffQ.write(0);
        }else {
            diffQ.write((qiX.read() > qiY.read()) ? tmpQXY.read() : tmpQYX.read());
        }
    }
}


void ScCompute::tQR() {
    while(true) {
        wait();
        if(rst.read()) {
            tmpQR.write(0);
        }else {
            tmpQR.write(diffQ.read() - diffR.read());
        }
    }

}


void ScCompute::tRQ() {
    while(true) {
        wait();
        if(rst.read()) {
            tmpRQ.write(0);
        }else {
            tmpRQ.write(diffR.read() - diffQ.read());
        }
    } 

}

void ScCompute::abs() {
    while(true) {
        wait();
        if(rst.read()) {
            absDiff.write(0);
            min.write(0);
        }else {
            if(diffR.read() > diffQ.read()) {
                absDiff.write(tmpRQ);
                min.write(diffQ);
            }else {
                absDiff.write(tmpQR);
                min.write(diffR);
            }
        }
    }
}


void ScCompute::get_mult() {
    while(true) {
        wait();
        if(rst.read()) {
            mult.write(0);
        }else {
            if(absDiff.read() < 5000) {
                mult.write(flut->FLUT[absDiff.read()]);
            }else {
               mult.write(0);
            }
        }
    }
}

void ScCompute::get_log() {
    while(true) {
        wait();
        if(rst.read()) {
            log_res.write(0);
        }else {
            log_res.write(log2_val.read());
        }
    }
}

void ScCompute::get_partialSum() {
    while(true) {
        wait();
        if(rst.read()) {
            partialSum.write(0);
        }else {
            partialSum.write(mult.read() + log_res.read());
        }
    }
}


void ScCompute::getA() {
    while(true) {
        wait();
        if(rst.read()) {
            A.write(0); 
        }else {
            A.write((min.read() < W.read()) ? min.read() : W.read());
        }
    }
}


void ScCompute::getB() {
    while(true) {
        wait();
        if(rst.read()) {
            B.write(0);
        }else {
            B.write((absDiff.read() == 0) ? sc_dt::sc_uint<32>(0) : partialSum);
        }
    }
}


void ScCompute::compute() {
    while(true) {
        wait();
        if(rst.read()) {
            result.write(0);
        }else {
            result.write(A.read() - B.read());
        }
    }
}
