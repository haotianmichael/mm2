#include <systemc.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#define InputLaneWIDTH  65
#define MAX_SEGLENGTH   5000
#define UpperBound 1659
#define SHIFT_ALL 1

SC_MODULE(InputGenerator) {
    sc_in<bool> clk;      
    sc_in<bool> rst;
    sc_signal<sc_int<32> > ri[MAX_SEGLENGTH];
    sc_signal<sc_int<32> > qi[MAX_SEGLENGTH];
    sc_signal<sc_int<32> > w[MAX_SEGLENGTH];

    sc_out<sc_int<32>> ri_out[InputLaneWIDTH];
    sc_out<sc_int<32>> qi_out[InputLaneWIDTH];
    sc_out<sc_int<32>> w_out[InputLaneWIDTH];

    int cycle_count;

    void shift_elements_lt64() {
        while (true && !SHIFT_ALL) {
            wait(); 
            if(!rst.read()) {
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
                cycle_count = 0;
                for(int i = 0; i < InputLaneWIDTH; i++) {
                    ri_out[i].write(static_cast<sc_int<32>>(-1));
                    qi_out[i].write(static_cast<sc_int<32>>(-1));
                    w_out[i].write(static_cast<sc_int<32>>(-1));
                }
            }
        }
    }

    void shift_elements_all() {
        while(true && SHIFT_ALL) {
            wait();
            if(!rst.read()) {
                if(cycle_count <= UpperBound-65) {
                    for(int i = 0; i < InputLaneWIDTH; i ++) {
                        ri_out[i].write(ri[i + cycle_count]);
                        qi_out[i].write(qi[i + cycle_count]);
                        w_out[i].write(w[i + cycle_count]);
                    }
                    cycle_count ++;
                }else if(cycle_count > UpperBound-65 && cycle_count < UpperBound) {
                    for(int i = 0; i < UpperBound-cycle_count; i ++) {
                        ri_out[i].write(ri[i + cycle_count]);
                        qi_out[i].write(qi[i + cycle_count]);
                        w_out[i].write(w[i + cycle_count]);
                    }
                    for(int i = InputLaneWIDTH-1; i > (UpperBound-cycle_count); i --) {
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

        SC_THREAD(shift_elements_lt64);
        sensitive << clk.pos();

        SC_THREAD(shift_elements_all);
        sensitive << clk.pos();

        // read file
        std::string filename("data/in2.txt");
        std::ifstream infile(filename);
        std::string line;
        int i = 0; 
        if(UpperBound <= InputLaneWIDTH){
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
            while(std::getline(infile, line) && i < UpperBound) {
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
};
 