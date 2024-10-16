#include <systemc.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#define InputLaneWIDTH  65
#define MAX_SEGLENGTH   5000

SC_MODULE(InputDispatcher) {
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
    SC_CTOR(InputDispatcher) : cycle_count(0) {
    }
};
 
struct mcuInputDispatcher : InputDispatcher {

    void initialize_data();
    void shift_data();
    SC_CTOR(mcuInputDispatcher) :
    InputDispatcher("InputDispatcher") {
        SC_THREAD(shift_data);
        sensitive << clk.pos();

        SC_THREAD(initialize_data);
        sensitive << rst.pos();
    }
};
 

struct ecuInputDispatcher : InputDispatcher {

    sc_in<sc_int<32> > SBase;
    sc_in<sc_int<32> > LBase; 

    void initialize_data();
    void shift_data();
    SC_CTOR(ecuInputDispatcher) :
     InputDispatcher("InputDispatcher"){
        SC_THREAD(shift_data);
        sensitive << clk.pos();

        SC_THREAD(initialize_data);
        sensitive << rst.pos();
    }
};