#ifndef BASELINE_H
#define BASELINE_H

#include <hcu.h>

/*Baseline of FCCM*/
struct BCU : public HCU{
    
    /*
        Input Module
        Compute Module
        Output Module 
    */
    SC_CTOR(BCU) : HCU("HCU") {
    
    }

};

#endif