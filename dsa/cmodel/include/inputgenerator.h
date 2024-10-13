#include <systemc.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#define InputLaneWIDTH  65

SC_MODULE(InputGenerator) {
    sc_in<bool> clk;      
    sc_in<bool> rst;
    sc_signal<sc_int<32> > ri[InputLaneWIDTH];
    sc_signal<sc_int<32> > qi[InputLaneWIDTH];
    sc_signal<sc_int<32> > w[InputLaneWIDTH];

    sc_out<sc_int<32>> ri_out[InputLaneWIDTH];
    sc_out<sc_int<32>> qi_out[InputLaneWIDTH];
    sc_out<sc_int<32>> w_out[InputLaneWIDTH];

    int cycle_count;

    void shift_elements() {
        while (true) {
            wait(); 
            if(!rst.read()) {
                if(cycle_count < InputLaneWIDTH) {
                    for (int i = InputLaneWIDTH-1; i >= cycle_count; i--) {
                        ri_out[i].write(ri[i - cycle_count]);
                        qi_out[i].write(qi[i - cycle_count]);
                        w_out[i].write(w[i - cycle_count]);
                    }
                    for(int i = 0; i < cycle_count; i++) {
                        ri_out[i].write(static_cast<sc_int<32>>(-1));
                        qi_out[i].write(static_cast<sc_int<32>>(-1));
                        w_out[i].write(static_cast<sc_int<32>>(-1));
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

        // read file
        std::string filename("data/in.txt");
        std::ifstream infile(filename);
        std::string line;
        int i = 0; 
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
    }
};
