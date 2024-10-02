#ifndef ScCompute_MODULE_H
#define ScCompute_MODULE_H

#include <systemc.h>
#include <iostream>

SC_MODULE(ScCompute) {

    SC_CTOR(ScCompute) {
        SC_THREAD(process);
    }

    void process();

};

#endif   // SC_MODULE_H