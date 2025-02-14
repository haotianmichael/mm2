#include "ilog2.h"


void ilog2::initialize() {
    while(true) {
        wait();
        if(rst.read()) {
             stage1_v.write(0);
             valid.write(0);
        }else {
             stage1_v.write(in.read());
            valid.write(in.read() != 0);
        }
    }
}

void ilog2::stage1() {
    while(true) {
        wait();
        if(rst.read()) {
            stage2_v.write(0); 
            stage2_log2.write(0);
        }else if(valid){
             if(stage1_v.read().range(31, 16) != 0) {
                 stage2_v.write(stage1_v.read().range(31, 16));
                 stage2_log2.write(16);
            }else {
                 stage2_v.write(stage1_v.read().range(15, 0));
                stage2_log2.write(0);
            }
        }
    }
}

void ilog2::stage2() {
    while(true) {
        wait();
        if(rst.read()) {
            stage3_indx.write(0);
            stage3_log2.write(0);
        } else if(valid){
            if(stage2_v.read().range(15, 8) != 0) {
                stage3_indx.write(stage2_v.read().range(15, 8));
                stage3_log2.write(stage2_log2.read() + 8);
            }else {
                stage3_indx.write(stage2_v.read().range(7, 0));
                stage3_log2.write(stage2_log2.read());
            }
        }
    }
}


void ilog2::stage3() {
    while(true) {
        wait();
        if(rst.read()) {
             stage4_logtable_value.write(0);
        } else if(valid){
            stage4_logtable_value.write(LogTable256[stage3_indx.read()]);
        }
    }
}

void ilog2::stage4() {
    while(true) {
        wait();
        if(rst.read()) {
             stage5_log2.write(0);
        }else if(valid){
             stage5_log2.write(stage3_log2.read() + stage4_logtable_value.read());
        }
    }
}

void ilog2::end() {
    while(true) {
        wait();
        if(rst.read()) {
             outlog2.write(0);
        }else if(valid){
             outlog2.write(stage5_log2.read() / 2);
        }
    }
}