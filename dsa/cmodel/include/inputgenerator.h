#include <systemc.h>
#include <fstream>
#include <string>

SC_MODULE(InputGenerator) {
    sc_out<sc_int<32>> output; 
    sc_in<bool> clk; 
    std::ifstream file; 

    SC_CTOR(InputGenerator) {
        SC_THREAD(read_from_file);
        sensitive << clk.pos(); 
        file.open("input.txt"); 
        if (!file.is_open()) {
            SC_REPORT_ERROR("InputGenerator", "Cannot open input file");
        }
    }

    void read_from_file() {
        sc_int<32> value;
        while (file >> value) { 
            output.write(value); 
            wait(); 
        }
        file.close();
    }

    ~InputGenerator() {
        if (file.is_open()) {
            file.close(); // 确保在模块析构时关闭文件
        }
    }
};
