#include <systemc.h>
#include "sc.h"


int sc_main(int argc, char* argv[]) {
    ScCompute hello("ScCompute");
    sc_start();
    return 0;
}