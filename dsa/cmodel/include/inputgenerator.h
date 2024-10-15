#include <systemc.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#define InputLaneWIDTH  65
#define MAX_SEGLENGTH   5000

SC_MODULE(InputGenerator) {
    sc_in<bool> clk;      
    sc_in<bool> rst;
    sc_in<sc_int<32> > UpperBound; 
    sc_signal<sc_int<32> > ri[MAX_SEGLENGTH];
    sc_signal<sc_int<32> > qi[MAX_SEGLENGTH];
    sc_signal<sc_int<32> > w[MAX_SEGLENGTH];

    sc_out<sc_int<32>> ri_out[InputLaneWIDTH];
    sc_out<sc_int<32>> qi_out[InputLaneWIDTH];
    sc_out<sc_int<32>> w_out[InputLaneWIDTH];

    int cycle_count;

    void initialize_data() {
        wait();
        std::string filename("data/in2.txt");
        std::ifstream infile(filename);
        std::string line;
        int i = 0; 
        if(UpperBound.read() <= InputLaneWIDTH){
            while (std::getline(infile, line) && i < InputLaneWIDTH) {
                 std::istringstream iss(line);
                 int val1, val2, val3;
                 if (iss >> val1 >> val2 >> val3) {
                    ri[i].write(val1);
                    qi[i].write(val3);
                    w[i].write(val2);
                    i++;
                 }
            }
        }else {
            while(std::getline(infile, line) && i < UpperBound.read()) {
                std::istringstream iss(line);
                int val1, val2, val3;
                if(iss >> val1 >> val2 >> val3) {
                    ri[i].write(val1); 
                    qi[i].write(val3); 
                    w[i].write(val2); 
                    i++;
                }
            }
        }
    }

    void shift_elements() {
        while(true) {
            wait();
            if(!rst.read()) {
                if(UpperBound.read() <= InputLaneWIDTH) {
                      if(cycle_count <= InputLaneWIDTH) {
                            for(int i = 0; i < (InputLaneWIDTH - cycle_count); i ++) {
                                 ri_out[i].write(ri[i + cycle_count]);
                                 qi_out[i].write(qi[i + cycle_count]);
                                 w_out[i].write(w[i + cycle_count]);
                            }
                            for(int i = InputLaneWIDTH-1; i >= (InputLaneWIDTH-cycle_count); i --) {
                                 ri_out[i].write(static_cast<sc_int<32> >(-1));
                                 qi_out[i].write(static_cast<sc_int<32> >(-1));
                                 w_out[i].write(static_cast<sc_int<32> >(-1));
                            }
                            cycle_count++;
                        }else {
                            for(int i = 0; i < InputLaneWIDTH; i++) {
                                 ri_out[i].write(static_cast<sc_int<32>>(-1));
                                 qi_out[i].write(static_cast<sc_int<32>>(-1));
                                 w_out[i].write(static_cast<sc_int<32>>(-1));
                            }
                        }
                }else {
                    if(cycle_count <= UpperBound.read()-65) {
                         for(int i = 0; i < InputLaneWIDTH; i ++) {
                             ri_out[i].write(ri[i + cycle_count]);
                             qi_out[i].write(qi[i + cycle_count]);
                             w_out[i].write(w[i + cycle_count]);
                         }
                         cycle_count ++;
                    }else if(cycle_count > UpperBound.read()-65 && cycle_count < UpperBound.read()) {
                         for(int i = 0; i < UpperBound.read()-cycle_count; i ++) {
                             ri_out[i].write(ri[i + cycle_count]);
                             qi_out[i].write(qi[i + cycle_count]);
                             w_out[i].write(w[i + cycle_count]);
                         }
                         for(int i = InputLaneWIDTH-1; i > (UpperBound.read()-cycle_count); i --) {
                            ri_out[i].write(static_cast<sc_int<32> >(-1));
                            qi_out[i].write(static_cast<sc_int<32> >(-1));
                            w_out[i].write(static_cast<sc_int<32> >(-1));
                         }
                         cycle_count ++;
                    }else {
                         for(int i = 0; i < InputLaneWIDTH; i++) {
                              ri_out[i].write(static_cast<sc_int<32>>(-1));
                              qi_out[i].write(static_cast<sc_int<32>>(-1));
                              w_out[i].write(static_cast<sc_int<32>>(-1));
                         } 
                    }
                }
            }else {
                cycle_count = 0;
                for(int i = 0; i < InputLaneWIDTH; i++) {
                    ri_out[i].write(static_cast<sc_int<32>>(-1));
                    qi_out[i].write(static_cast<sc_int<32>>(-1));
                    w_out[i].write(static_cast<sc_int<32>>(-1));
                }
            }
        }
    }

    SC_CTOR(InputGenerator) : cycle_count(0) {

        SC_THREAD(shift_elements);
        sensitive << clk.pos();

        SC_THREAD(initialize_data);
        sensitive << rst.pos();
     
    }
};
 