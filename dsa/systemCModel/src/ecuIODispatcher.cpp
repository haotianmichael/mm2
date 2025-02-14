#include "ioDispatcher.h"

void ecuIODispatcher::shift_data() {
    while(true) {
        wait();
        if(!rst.read()) {
            if(en.read()){
                if(base_count <= UpperBound.read() - LowerBound.read()) {
                    ecu_ri_out.write(ri[base_count]);
                    ecu_qi_out.write(qi[base_count]);
                    ecu_w_out.write(w[base_count]);
                    ecu_idx_out.write(idx[base_count]);
                    ecu_score_out.write(final_score[base_count]);
                    base_count++;
                }
                if(cycle_count <= UpperBound.read()-65) {
                     for(int i = 0; i < ECUInputLaneWIDTH; i ++) {
                         ri_out[i].write(ri[i + cycle_count]);
                         qi_out[i].write(qi[i + cycle_count]);
                         w_out[i].write(w[i + cycle_count]);
                         idx_out[i].write(idx[i + cycle_count]);
                     }
                     cycle_count ++;
                }else if(cycle_count > UpperBound.read()-65 && cycle_count <= UpperBound.read()) {
                     for(int i = 0; i <= UpperBound.read()-cycle_count; i ++) {
                         ri_out[i].write(ri[i + cycle_count]);
                         qi_out[i].write(qi[i + cycle_count]);
                         w_out[i].write(w[i + cycle_count]);
                         idx_out[i].write(idx[i + cycle_count]);
                     }
                     for(int i = ECUInputLaneWIDTH-1; i > (UpperBound.read()-cycle_count); i --) {
                        ri_out[i].write(static_cast<sc_int<32> >(-1));
                        qi_out[i].write(static_cast<sc_int<32> >(-1));
                        w_out[i].write(static_cast<sc_int<32> >(-1));
                        idx_out[i].write(static_cast<sc_int<32> >(-1));
                     }
                     cycle_count ++;
                }else {
                     for(int i = 0; i < ECUInputLaneWIDTH; i++) {
                          ri_out[i].write(static_cast<sc_int<32>>(-1));
                          qi_out[i].write(static_cast<sc_int<32>>(-1));
                          w_out[i].write(static_cast<sc_int<32>>(-1));
                          idx_out[i].write(static_cast<sc_int<32> >(-1));
                     } 
                }
            }else {
                base_count = 0;
                cycle_count = LowerBound.read();
                for(int i = 0; i < ECUInputLaneWIDTH + 1; i++) {
                    ri_out[i].write(static_cast<sc_int<32>>(-1));
                    qi_out[i].write(static_cast<sc_int<32>>(-1));
                    w_out[i].write(static_cast<sc_int<32>>(-1));
                    idx_out[i].write(static_cast<sc_int<32> >(-1));
                }
                ecu_qi_out.write(static_cast<sc_int<WIDTH> >(-1));
                ecu_ri_out.write(static_cast<sc_int<WIDTH> >(-1));
                ecu_w_out.write(static_cast<sc_int<WIDTH> >(-1));
                ecu_idx_out.write(static_cast<sc_int<WIDTH>>(-1));
                ecu_score_out.write(static_cast<sc_int<WIDTH>>(-1));
            }
        }else {
            base_count = 0;
            cycle_count = LowerBound.read();
            for(int i = 0; i < ECUInputLaneWIDTH + 1; i++) {
                ri_out[i].write(static_cast<sc_int<32>>(-1));
                qi_out[i].write(static_cast<sc_int<32>>(-1));
                w_out[i].write(static_cast<sc_int<32>>(-1));
                idx_out[i].write(static_cast<sc_int<32> >(-1));
            }
            ecu_qi_out.write(static_cast<sc_int<WIDTH> >(-1));
            ecu_ri_out.write(static_cast<sc_int<WIDTH> >(-1));
            ecu_w_out.write(static_cast<sc_int<WIDTH> >(-1));
            ecu_idx_out.write(static_cast<sc_int<WIDTH>>(-1));
            ecu_score_out.write(static_cast<sc_int<WIDTH>>(-1));
        }
    }
}