#include "ioDispatcher.h"

void mcuIODispatcher::shift_data() {
    while(true) {
        wait();
        if(!rst.read()) {
            if(en.read()){
                //std::cout << "IO enable start at: " << sc_time_stamp() << std::endl;
                if(UpperBound.read() <= MCUInputLaneWIDTH) {
                       if(cycle_count <= MCUInputLaneWIDTH) {
                            for(int i = 0; i < (MCUInputLaneWIDTH - cycle_count); i ++) {
                                 ri_out[i].write(ri[i + cycle_count]);
                                 qi_out[i].write(qi[i + cycle_count]);
                                 w_out[i].write(w[i + cycle_count]);
                                 idx_out[i].write(idx[i + cycle_count]);
                            }
                            for(int i = MCUInputLaneWIDTH-1; i >= (MCUInputLaneWIDTH-cycle_count); i --) {
                                 ri_out[i].write(static_cast<sc_int<32> >(-1));
                                 qi_out[i].write(static_cast<sc_int<32> >(-1));
                                 w_out[i].write(static_cast<sc_int<32> >(-1));
                                 idx_out[i].write(static_cast<sc_int<32> >(-1));
                            }
                            cycle_count++;
                        }else {
                            for(int i = 0; i < MCUInputLaneWIDTH; i++) {
                                 ri_out[i].write(static_cast<sc_int<32>>(-1));
                                 qi_out[i].write(static_cast<sc_int<32>>(-1));
                                 w_out[i].write(static_cast<sc_int<32>>(-1));
                                 idx_out[i].write(static_cast<sc_int<32> >(-1));
                            }
                        }
                }else {
                    if(cycle_count <= UpperBound.read()-MCUInputLaneWIDTH) {
                         for(int i = 0; i < MCUInputLaneWIDTH; i ++) {
                             ri_out[i].write(ri[i + cycle_count]);
                             qi_out[i].write(qi[i + cycle_count]);
                             w_out[i].write(w[i + cycle_count]);
                             idx_out[i].write(idx[i + cycle_count]);
                         }
                         cycle_count ++;
                    }else if(cycle_count > UpperBound.read()-MCUInputLaneWIDTH && cycle_count <= UpperBound.read()) {
                         for(int i = 0; i <= UpperBound.read()-cycle_count; i ++) {
                             ri_out[i].write(ri[i + cycle_count]);
                             qi_out[i].write(qi[i + cycle_count]);
                             w_out[i].write(w[i + cycle_count]);
                             idx_out[i].write(idx[i + cycle_count]);
                         }
                         for(int i = MCUInputLaneWIDTH-1; i > (UpperBound.read()-cycle_count); i --) {
                            ri_out[i].write(static_cast<sc_int<32> >(-1));
                            qi_out[i].write(static_cast<sc_int<32> >(-1));
                            w_out[i].write(static_cast<sc_int<32> >(-1));
                            idx_out[i].write(static_cast<sc_int<32> >(-1));
                         }
                         cycle_count ++;
                    }else {
                         for(int i = 0; i < MCUInputLaneWIDTH + 1; i++) {
                              ri_out[i].write(static_cast<sc_int<32>>(-1));
                              qi_out[i].write(static_cast<sc_int<32>>(-1));
                              w_out[i].write(static_cast<sc_int<32>>(-1));
                              idx_out[i].write(static_cast<sc_int<32> >(-1));
                         } 
                    }
                }
            }else {
                cycle_count = 0;
                for(int i = 0; i < MCUInputLaneWIDTH + 1; i++) {
                    ri_out[i].write(static_cast<sc_int<32>>(-1));
                    qi_out[i].write(static_cast<sc_int<32>>(-1));
                    w_out[i].write(static_cast<sc_int<32>>(-1));
                    idx_out[i].write(static_cast<sc_int<32> >(-1));
                } 
            }
        }else {
            cycle_count = 0;
            for(int i = 0; i < MCUInputLaneWIDTH + 1; i++) {
                ri_out[i].write(static_cast<sc_int<32>>(-1));
                qi_out[i].write(static_cast<sc_int<32>>(-1));
                w_out[i].write(static_cast<sc_int<32>>(-1));
                idx_out[i].write(static_cast<sc_int<32> >(-1));
            }
        }
    }
}