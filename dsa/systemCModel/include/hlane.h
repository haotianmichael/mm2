#ifndef HLane_H
#define HLane_H

#include <systemc.h>
#include <cmath>
#include "helper.h"

/*Anchor (do only within one read)*/
struct Anchor {
    sc_in<sc_int<WIDTH> > ri, qi;
    sc_in<sc_int<WIDTH> > W;
};

/*ScoreCompute*/
SC_MODULE(Score) {
    sc_in<bool> rst;
    sc_in<bool> en;
    sc_in<sc_int<WIDTH> > riX, riY, qiX, qiY;
    sc_in<sc_int<WIDTH> > W, W_avg;
    sc_out<sc_int<WIDTH> > result;

    double  absDiff;
    void compute() {
        while(true) {
            if(rst.read()) {
                result.write(-1);
            }else {
                /*if(riX.read()>0) {
                    std::cout << riX.read() << std::endl;
                }*/
                if(en.read()) {
                    //std::cout << "hcu enable start at " << sc_time_stamp() << std::endl;
                    if((riX.read() == 0 && riY.read() == 0) && (qiX.read() == 0 && qiY.read() == 0)) {
                        result.write(-1);
                    }else {
                        if((riX.read() == 0 && qiX.read() == 0) || (riY.read() == 0 && qiY.read() == 0)) {
                            // this only happens when hcu only has element ri/qiArray[0] and ri/qiArray[i]=-1(i=1,2,3...)
                            result.write(static_cast<sc_int<WIDTH> >(-1));
                        }else {
                            //std::cout << riX.read() << " " << riY.read()  << " " << qiX.read() << " " << qiY.read() << " " << std::endl;
                            absDiff = fabs(fabs(riX.read().to_double()-riY.read().to_double()) - fabs(qiX.read().to_double() - qiY.read().to_double()));
                            double tmpB = (int)(absDiff * 15 * 0.01 + 0.5 * (log(absDiff)/log(2.0)));
                            double B = absDiff == 0 ? 0 : tmpB;
                            double tmpA = (fabs(riX.read().to_double() - riY.read().to_double()) > fabs(qiX.read().to_double() - qiY.read().to_double())) ? fabs(qiX.read().to_double() - qiY.read().to_double()) : fabs(riX.read().to_double() - riY.read().to_double());
                            double A;
                            if(tmpA > 15.0) {
                               A = 15.0;
                            }else {
                               A = tmpA;
                            }
                           
                            result.write(static_cast<sc_int<WIDTH> >(A - B));
                        }
                    }
                }else {
                    result.write(static_cast<sc_int<WIDTH> >(-1));
                }
           }
            wait(10, SC_NS);
        }
    }

    SC_CTOR(Score) {
        SC_THREAD(compute);
        sensitive << riX << riY << qiX << qiY << W;
    }
};


/*Comparator*/
SC_MODULE(Comparator) {

    sc_in<bool> rst;
    sc_in<bool> clk;
    sc_in<bool> en;
    sc_in<sc_int<WIDTH> > cmpA, cmpB;
    sc_out<sc_int<WIDTH> > bigger;
    //sc_out<bool> comResult;

    void compare(){
        if(rst.read()) {
            bigger.write(-1);
        }else {
            if(en.read()) {
                bigger.write(cmpA.read() > cmpB.read() 
                  ? cmpA.read() : cmpB.read());
                //comResult.write(cmpA.read() > cmpB.read() 
                 //? true : false);
            }else {
                bigger.write(static_cast<sc_int<WIDTH> >(-1));
                //comResult.write(false);
            }
        }
    }

    SC_CTOR(Comparator) {
        SC_METHOD(compare);
        sensitive << clk.pos();
    } 
};

/*SC_MODULE(HAdder) {
    sc_in<bool> clk, rst;
    sc_in <bool> en;
    sc_in<sc_int<WIDTH>> in1;
    sc_in<sc_int<WIDTH>> in2;
    sc_out<sc_int<WIDTH>> out;

    void process() {
        if(!rst.read()) {
            if(en.read()) {
                int result = in1.read() + in2.read();
                out.write(static_cast<sc_int<WIDTH>>(result));
            }
        }
    }

    SC_CTOR(HAdder) {
       SC_THREAD(process); 
       sensitive << clk.pos();
    }

};*/
/*Computing Lane for One-Pair of Anchors*/
SC_MODULE(HLane) {
    
    sc_in<bool> clk, rst;
    sc_in<bool> en;
    sc_in<sc_int<WIDTH> > id;   // Id of each Lane within one HCU (1-65)
    sc_in<sc_int<WIDTH> > lastCmp;  // input of this lane's  comparator
    //sc_in<sc_int<WIDTH> > current_ScoreOfZeroLane;
    sc_signal<sc_int<WIDTH> > computeResult; // result of ScCompute
    sc_signal<sc_int<WIDTH> > biggerScore;  // output of this lane/input of next lane's comparator
    //sc_signal<bool> comResult;

    /*pipeline*/
    Anchor inputA;  
    Anchor inputB;
    Score *compute;
    Comparator *comparator;
    /*HAdder *adderA;
    HAdder *adderB;
    sc_signal<sc_int<WIDTH>> adderAOut;
    sc_signal<sc_int<WIDTH>> lhs;
    sc_signal<sc_int<WIDTH>> adderBOut;*/

    SC_CTOR(HLane) {
        
        compute = new Score("compute");
        compute->rst(rst);
        compute->en(en);
        compute->riX(inputA.ri);
        compute->riY(inputB.ri);
        compute->qiX(inputA.qi);
        compute->qiY(inputB.qi);
        compute->W(inputA.W);   // inputA has the same span with inputB
        compute->W_avg(inputA.W);
        compute->result(computeResult);
        /*adderA = new HAdder("adderA"); 
        adderB = new HAdder("adderB"); 

        adderA->clk(clk);
        adderA->rst(rst);
        adderA->en(en);
        adderA->in1(computeResult);
        adderA->in2(current_ScoreOfZeroLane);
        adderA->out(adderAOut);

        adderB->clk(clk);
        adderB->rst(rst);
        adderB->en(en);
        adderB->in1(lastCmp);
        //lhs.write(0);
        adderB->in2(lhs);
        adderB->out(adderBOut);*/

        comparator = new Comparator("comparator");
        comparator->clk(clk);
        comparator->rst(rst);
        comparator->en(en);
        comparator->cmpA(computeResult);
        comparator->cmpB(lastCmp);
        comparator->bigger(biggerScore);
        //comparator->comResult(comResult);
    }
};

#endif