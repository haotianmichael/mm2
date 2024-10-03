#include <systemc.h>
#include "ilog2.h"

SC_MODULE(Top) {
    sc_signal<bool> clk;
    sc_signal<bool> reset;
    sc_signal<sc_uint<32>> input_signal;

    ilog2* ilog2_cal;

    SC_CTOR(Top) {
        ilog2_cal = new ilog2("ILOG2");
        ilog2_cal->clk(clk);
        ilog2_cal->rst(reset);
        ilog2_cal->in(input_signal);

        SC_THREAD(clock_generator);
        SC_THREAD(reset_generator);
    }

    void clock_generator() {
        while (true) {
            clk.write(0);
            wait(10, SC_NS);
            clk.write(1);
            wait(10, SC_NS);
        }
    }

    void reset_generator() {
        reset.write(1);
        wait(15, SC_NS);
        reset.write(0);
        wait(100, SC_NS); // 等待一段时间后结束仿真
        sc_stop(); // 终止仿真
    }

    ~Top() {
        delete ilog2_cal; // 释放内存
    }
};

int sc_main(int argc, char* argv[]) {
    Top top("Top");
    sc_start(); // 启动仿真
    return 0;
}
