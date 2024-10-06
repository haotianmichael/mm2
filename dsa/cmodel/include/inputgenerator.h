#include <systemc.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#define WIDTH  65

SC_MODULE(InputGenerator) {
    sc_in<bool> clk;      
    sc_in<bool> rst;
    sc_signal<sc_uint<32> > ri[WIDTH];
    sc_signal<sc_uint<32> > qi[WIDTH];
    sc_signal<sc_uint<32> > w[WIDTH];

    sc_out<sc_uint<32>> ri_out[WIDTH];
    sc_out<sc_uint<32>> qi_out[WIDTH];
    sc_out<sc_uint<32>> w_out[WIDTH];

    int cycle_count;

    void shift_elements() {
        if(!rst.read()) {
        while (true) {
            wait(); 
            for (int i = WIDTH-1; i >= cycle_count; i--) {
                ri_out[i].write(ri[i - cycle_count]);
                qi_out[i].write(qi[i - cycle_count]);
                w_out[i].write(w[i - cycle_count]);
            }
            for(int i = 0; i < cycle_count; i++) {
                ri_out[i].write(static_cast<sc_uint<32>>(-1));
                qi_out[i].write(static_cast<sc_uint<32>>(-1));
                w_out[i].write(static_cast<sc_uint<32>>(-1));
            }
            cycle_count++;
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
        while (std::getline(infile, line) && i < WIDTH) {
            std::istringstream iss(line);
            int val1, val2, val3;
            if (iss >> val1 >> val2 >> val3) {
                ri[i].write(val1);
                qi[i].write(val2);
                w[i].write(val3);
                i++;
            }
        }
        infile.close();
    }
};
