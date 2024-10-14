#ifndef ECU_H
#define ECU_H

#include "hcu.h"

/*Extensive Chaining Unit for every anchor whose range exceeds 65*/
struct ECU : public HCU{

    SC_CTOR(ECU) : HCU("HCU") {

    }
};


#endif