#ifndef HCU_HELP_H
#define HCU_HELP_H

#include <systemc.h>
#include <sstream>
#include <string.h>

#define WIDTH 32
#define HCU_NUM 80  //64 is tested
#define LaneWIDTH 65
#define MCUInputLaneWIDTH 65
#define ECUInputLaneWIDTH 65

#define MAX_SEGLENGTH 5000 
#define MIN_SEGLENGTH 70
#define MAX_READ_LENGTH 200000 
#define RAM_SIZE 10   
#define MAX_SEG_NUM 5000
#define ReadNumProcessedOneTime  1

#define Reduction_NUM 128
#define RESULT_NUM 414 // 254 reductionResult + 80(160) hcuResult (FIXME: now we need 80 mcu and 80 ecu)
#define Reduction_USAGE 254
#define Reduction_KIND 7
#define Reduction_FIFO_NUM 1000
#endif