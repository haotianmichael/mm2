#ifndef HCU_H
#define HCU_H

#include <systemc.h>
#include <sstream>
#include <string.h>
#include <cmath>

#define LaneWIDTH  64
#define WIDTH 32
/*Anchor (do only within one read)*/
struct Anchor {
    sc_in<sc_uint<WIDTH> > ri, qi;
    sc_in<sc_uint<WIDTH> > W;
};

/*ScoreCompute*/
SC_MODULE(Score) {
    sc_in<bool> rst;
    sc_in<sc_uint<WIDTH> > riX, riY, qiX, qiY;
    sc_in<sc_uint<WIDTH> > W, W_avg;
    sc_out<sc_uint<WIDTH> > result;

    double  absDiff;
    void compute() {
        while(true) {
            if(rst.read()) {
                result.write(0);
            }else {
                absDiff = fabs(fabs(riX.read().to_double()-riY.read().to_double()) - fabs(qiX.read().to_double() - qiY.read().to_double()));
                double tmpB = (uint)(absDiff * 15 * 0.01 + 0.5 * (log(absDiff)/log10(2.0)));
                double B = absDiff == 0 ? 0 : tmpB;
                double tmpA = (fabs(riX.read().to_double() - riY.read().to_double()) > fabs(qiX.read().to_double() - qiY.read().to_double())) ? fabs(qiX.read().to_double() - qiY.read().to_double()) : fabs(riX.read().to_double() - riY.read().to_double());
                double A;
                if(tmpA > W.read().to_double()) {
                    A = W.read().to_double();
                }else {
                    A = tmpA;
                }
                result.write(static_cast<sc_uint<WIDTH> >(A - B));
            }
            wait(1, SC_NS);
        }
    }


    SC_CTOR(Score) {
        SC_THREAD(compute);
    }
};


/*Comparatot*/
SC_MODULE(Comparator) {

    sc_in<bool> rst;
    sc_in<sc_uint<WIDTH> > cmpA, cmpB;
    sc_out<sc_uint<WIDTH> > bigger;

    void compare(){
          while(true) {
            if(rst.read()) {
                 bigger.write(0);
            }else {
                 bigger.write(cmpA.read() > cmpB.read() 
                    ? cmpA.read() : cmpB.read());
            }
            wait(1, SC_NS);
        } 
    }

    SC_CTOR(Comparator) {
        SC_THREAD(compare);
    } 
};

/*Computing Lane for One-Pair of Anchors*/
SC_MODULE(HLane) {
    
    sc_in<bool> clk, rst;
    //sc_in<sc_int<32> > id;   // Id of each Lane within one HCU (1-65)
    sc_signal<sc_uint<32> > lastCmp;  // input of this lane's  comparator
    sc_signal<sc_uint<WIDTH> > biggerScore;  // output of this lane/input of next lane's comparator

    /*pipeline*/
    Anchor inputA;  
    Anchor inputB;
    Score *compute;
    sc_out<sc_uint<WIDTH> > computeResult; // result of ScCompute
    Comparator *comparator;

    void process();
    SC_CTOR(HLane) {
        

        compute = new Score("compute");
        compute->rst(rst);
        compute->riX(inputA.ri);
        compute->riY(inputB.ri);
        compute->qiX(inputA.qi);
        compute->qiY(inputB.qi);
        compute->W(inputA.W);   // inputA has the same span with inputB
        compute->W_avg(inputA.W);
        compute->result(computeResult);
        /*
        comparator = new Comparator("comparator");
        comparator->rst(rst);
        comparator->cmpA(computeResult);
        comparator->cmpB(lastCmp);
        comparator->bigger(biggerScore);
        */

        SC_THREAD(process);
        sensitive << clk.pos();
    }
};



/*Hybrid Chaining Unit*/
SC_MODULE(HCU) {

    sc_in<bool> clk, rst;
    sc_in<sc_uint<WIDTH> > riArray[LaneWIDTH + 1];
    sc_in<sc_uint<WIDTH> > qiArray[LaneWIDTH + 1];
    sc_in<sc_uint<WIDTH> > W;
    sc_out<sc_uint<WIDTH> > Hout;

    /* HCU has 64 Lane, 65 InputAnchor*/
    HLane* hlane[LaneWIDTH];
    /* Registers for staging Lane's output for 1 cycle*/
    sc_signal<sc_uint<WIDTH> >  regBiggerScore[LaneWIDTH + 1];
  
    SC_CTOR(HCU){}
};

/*Main Chaining Unit for every anchor */
struct MCU : public HCU{

    void process();

    SC_CTOR(MCU) : HCU("HCU"){
        SC_THREAD(process);
        sensitive << clk.pos();
    }
};

/*Extensive Chaining Unit for every anchor whose range exceeds 65*/
struct ECU : public HCU{

    SC_CTOR(ECU) : HCU("HCU") {

    }
};

#endif