#ifndef CHAIN_H 
#define CHAIN_H 

#include "hcu.h"
#include "rangeCountUnit.h"
#include "schedulerTable.h"
#include "reductionPool.h"
#include "ioDispatcher.h"


SC_MODULE(Chain) {

    sc_in<bool> clk;
    sc_in<bool> rst;
    sc_signal<bool> start;     // initialization done signal

    std::vector<sc_signal<sc_int<WIDTH>>*> anchorNum;  //number of anchors
    std::vector<std::vector<sc_signal<sc_int<WIDTH>>*>> anchorRi;
    std::vector<std::vector<sc_signal<sc_int<WIDTH>>*>> anchorQi;
    std::vector<std::vector<sc_signal<sc_int<WIDTH>>*>> anchorW;
    std::vector<std::vector<sc_signal<sc_int<WIDTH>>*>> anchorSuccessiveRange;// successive range of every anchor 

    /*ResultArray consists of both HCU's resultArray(UB<=65) and ReductionPool's resultArray(UB>65) */
    std::vector<sc_out<sc_int<WIDTH>>> resultArray;

    // @RC Unit
    /*
        taking all 200000 reads at rst.neg()
    */
    RangeCountUnit *rc;

    // @SegmentsQueue (Two Ports)
    /*
        read from rc.array --> simulating read from DDR
        the width of fifo simulating DDR's width
    */
    // UpperBound <= 65 elements  Lane[0, 64]
    sc_fifo<riSegment> riSegQueueLong;  
    sc_fifo<qiSegment> qiSegQueueLong;
    sc_fifo<wSegment> wSegQueueLong;
    // UpperBound > 65
    sc_fifo<riSegment> riSegQueueShort; 
    sc_fifo<qiSegment> qiSegQueueShort;
    sc_fifo<wSegment> wSegQueueShort;
    sc_int<WIDTH> readIdx = 0; //read which currently being cut
    sc_event data_available;  // fifo signal
    sc_event space_available;

    // @SchedulerTable
    /*
        FIXME: schedulerTable needs to be filled more than one time within one cycle.
    */
    SchedulerTable *schedulerTable;

    // @LocalRAM store the Input
    std::vector<ram_data> localRAM;
    std::vector<bool> freeList;
    int ramIndex;

    // @HCU Pool
    // simulator can only use either mcuPool[i] or ecuPool[i], cannot use them both at the same time.
    MCU *mcuPool[HCU_NUM];
    ECU *ecuPool[HCU_NUM];
    sc_int<WIDTH> alloParam;
    sc_int<WIDTH> freeParam;

    // @IODispatcher
    mcuIODispatcher *mcuIODisPatcherPool[HCU_NUM];
    ecuIODispatcher *ecuIODisPatcherPool[HCU_NUM];

    // @Wiring
    sc_signal<sc_int<WIDTH> > mri[HCU_NUM][MCUInputLaneWIDTH + 1];
    sc_signal<sc_int<WIDTH> > mqi[HCU_NUM][MCUInputLaneWIDTH + 1];
    sc_signal<sc_int<WIDTH> > mw[HCU_NUM][MCUInputLaneWIDTH + 1];
    sc_signal<sc_int<WIDTH> > eri[HCU_NUM][MCUInputLaneWIDTH + 1];
    sc_signal<sc_int<WIDTH> > eqi[HCU_NUM][MCUInputLaneWIDTH + 1];
    sc_signal<sc_int<WIDTH> > ew[HCU_NUM][MCUInputLaneWIDTH + 1];
    sc_signal<sc_int<WIDTH> > ecu_ri[HCU_NUM];
    sc_signal<sc_int<WIDTH> > ecu_qi[HCU_NUM];
    sc_signal<sc_int<WIDTH> > ecu_w[HCU_NUM];

    // @ReductionPool
    ReductionController *rtController;
    ReductionTree *reductionTree[Reduction_USAGE];
    sc_in<sc_int<WIDTH>> ROCC[Reduction_KIND];
    sc_fifo<reductionInput> *reductionInputArray[Reduction_FIFO_NUM];        
    sc_signal<sc_int<WIDTH>> *notifyArray[Reduction_FIFO_NUM];

    void chain_hcu_pre();
    void chain_hcu_execute();
    void chain_hcu_fillTable();
    void chain_hcu_allocate();
    void chain_rt_checkTable();

	SC_CTOR(Chain) : 
         riSegQueueLong(MAX_SEG_NUM), 
         qiSegQueueLong(MAX_SEG_NUM), 
         wSegQueueLong(MAX_SEG_NUM),
         riSegQueueShort(MAX_SEG_NUM),
         qiSegQueueShort(MAX_SEG_NUM),
         wSegQueueShort(MAX_SEG_NUM),
         ramIndex(0),
         alloParam(0),
         freeParam(0),
         anchorNum(ReadNumProcessedOneTime),
         anchorRi(ReadNumProcessedOneTime, std::vector<sc_signal<sc_int<WIDTH>>*>(MAX_READ_LENGTH)),
         anchorQi(ReadNumProcessedOneTime, std::vector<sc_signal<sc_int<WIDTH>>*>(MAX_READ_LENGTH)),
         anchorW(ReadNumProcessedOneTime, std::vector<sc_signal<sc_int<WIDTH>>*>(MAX_READ_LENGTH)),
         anchorSuccessiveRange(ReadNumProcessedOneTime, std::vector<sc_signal<sc_int<WIDTH>>*>(MAX_READ_LENGTH)),
         resultArray(RESULT_NUM){


        // prepare the segmentQueue
        // one read per cycle.(延时问题后续重新考虑)
        SC_THREAD(chain_hcu_pre);
        sensitive << clk.pos();

        // fill SchedulerTable for every Segment based on SegQueue
        SC_THREAD(chain_hcu_fillTable);
        sensitive << clk.pos();

        // allocate HCU for every Segment based on SchedulerTable
        SC_THREAD(chain_hcu_allocate);
        sensitive << clk.pos();

        // HCU IO Wiring/Free and fill PSQTable
        SC_THREAD(chain_hcu_execute);
        sensitive << clk.pos();

        // check SchedulerTable and allocate ReductionTrees
        SC_THREAD(chain_rt_checkTable);
        sensitive << clk.pos();

        for(int i = 0; i < ReadNumProcessedOneTime; i ++) {
            anchorNum[i] = new sc_signal<sc_int<WIDTH>>();
            for(int j = 0; j < MAX_READ_LENGTH; j ++) {
                try {
                    anchorRi[i][j] = new sc_signal<sc_int<WIDTH>>();
                    anchorQi[i][j] = new sc_signal<sc_int<WIDTH>>();
                    anchorW[i][j] = new sc_signal<sc_int<WIDTH>>();
                    anchorSuccessiveRange[i][j] = new sc_signal<sc_int<WIDTH>>();
                }catch(const std::bad_alloc& e) {
                    std::cerr << "Scheduler Memory allocation failed:" << e.what() << std::endl;
                }
            }
        }

        try {
            rc = new RangeCountUnit("RangeCountUnit");
        }catch(const std::bad_alloc& e) {
            std::cerr << "Scheduler's RangeCountUnit allocation failed:" << e.what() << std::endl;
        }
        rc->rst(rst);
        rc->cutDone(start);
        for(int readIdx = 0; readIdx < ReadNumProcessedOneTime; readIdx++) {
            rc->anchorNum[readIdx]->bind(*anchorNum[readIdx]);
            for(int i = 0; i < MAX_READ_LENGTH; i ++) {
                rc->anchorRi[readIdx][i]->bind(*anchorRi[readIdx][i]);
                rc->anchorQi[readIdx][i]->bind(*anchorQi[readIdx][i]);
                rc->anchorW[readIdx][i]->bind(*anchorW[readIdx][i]);
                rc->anchorSuccessiveRange[readIdx][i]->bind(*anchorSuccessiveRange[readIdx][i]);
            }
        }

        schedulerTable = new SchedulerTable;
        try {
            // FIXME: memory maybe not enough.
            localRAM.resize(RAM_SIZE);
            freeList.resize(RAM_SIZE, true);
        }catch(const std::bad_alloc& e) {
            std::cerr << "Scheduler's localRAM allocation failed:" << e.what() << std::endl;
        }

        std::ostringstream mcu_name, ecu_name, mcuIO_name, ecuIO_name;
        for(int i = 0; i < HCU_NUM; i ++) {
            mcuIO_name << "mcuIODis(" << i << ")";
            ecuIO_name << "ecuIODis(" << i << ")";

            mcuIODisPatcherPool[i] = new mcuIODispatcher(mcuIO_name.str().c_str());
            mcuIODisPatcherPool[i]->clk(clk);
            mcuIODisPatcherPool[i]->rst(rst);
            mcuIODisPatcherPool[i]->en.write(static_cast<bool>(0));

            ecuIODisPatcherPool[i] = new ecuIODispatcher(ecuIO_name.str().c_str());
            ecuIODisPatcherPool[i]->clk(clk);
            ecuIODisPatcherPool[i]->rst(rst);
            ecuIODisPatcherPool[i]->en.write(static_cast<bool>(0));

            for(int j = 0; j < MCUInputLaneWIDTH + 1; j ++) {
                mcuIODisPatcherPool[i]->ri_out[j](mri[i][j]);
                mcuIODisPatcherPool[i]->qi_out[j](mqi[i][j]);
                mcuIODisPatcherPool[i]->w_out[j](mw[i][j]);
                ecuIODisPatcherPool[i]->ri_out[j](eri[i][j]);
                ecuIODisPatcherPool[i]->qi_out[j](eqi[i][j]);
                ecuIODisPatcherPool[i]->w_out[j](ew[i][j]);
            }
            ecuIODisPatcherPool[i]->ecu_ri_out(ecu_ri[i]);
            ecuIODisPatcherPool[i]->ecu_qi_out(ecu_qi[i]);
            ecuIODisPatcherPool[i]->ecu_w_out(ecu_w[i]);
            mcuIO_name.str("");
            ecuIO_name.str("");
        }

        
        int rIndex = 0;
        for(int i = 0; i < HCU_NUM && rIndex < HCU_NUM*2; i ++) {
            mcu_name << "mcuPool(" << i << ")";
            ecu_name << "ecuPool(" << i << ")";
            mcuPool[i] = new MCU(mcu_name.str().c_str());
            mcuPool[i]->clk(clk);
            mcuPool[i]->rst(rst);
            mcuPool[i]->en.write(static_cast<bool>(0));
            ecuPool[i] = new ECU(ecu_name.str().c_str());
            ecuPool[i]->clk(clk);
            ecuPool[i]->rst(rst);
            ecuPool[i]->en.write(static_cast<bool>(0));
            for(int j = 0; j < MCUInputLaneWIDTH + 1; j ++) {
                mcuPool[i]->riArray[j](mri[i][j]);
                mcuPool[i]->qiArray[j](mqi[i][j]);
                mcuPool[i]->W[j](mw[i][j]);
                ecuPool[i]->riArray[j](eri[i][j]);
                ecuPool[i]->qiArray[j](eqi[i][j]);
                ecuPool[i]->W[j](ew[i][j]);
            }
            ecuPool[i]->ecu_ri(ecu_ri[i]);
            ecuPool[i]->ecu_qi(ecu_qi[i]);
            ecuPool[i]->ecu_w(ecu_w[i]);
            
            mcuPool[i]->currentReadID.write(static_cast<sc_int<WIDTH> >(-1));
            mcuPool[i]->currentSegID.write(static_cast<sc_int<WIDTH> >(-1));
            mcuPool[i]->LowerBound.write(static_cast<sc_int<WIDTH> >(-1));
            mcuPool[i]->UpperBound.write(static_cast<sc_int<WIDTH> >(-1));
            mcuPool[i]->type.write(static_cast<bool>(0));
            mcuPool[i]->executeTime.write(sc_time(0, SC_NS));
            mcuPool[i]->freeTime.write(sc_time(0, SC_NS));
            resultArray[rIndex++](mcuPool[i]->regBiggerScore[0]);

            ecuPool[i]->currentReadID.write(static_cast<sc_int<WIDTH> >(-1));
            ecuPool[i]->currentSegID.write(static_cast<sc_int<WIDTH> >(-1));
            ecuPool[i]->LowerBound.write(static_cast<sc_int<WIDTH> >(-1));
            ecuPool[i]->UpperBound.write(static_cast<sc_int<WIDTH> >(-1));
            ecuPool[i]->type.write(static_cast<bool>(0));
            ecuPool[i]->executeTime.write(sc_time(0, SC_NS));
            ecuPool[i]->freeTime.write(sc_time(0, SC_NS));
            resultArray[rIndex++](ecuPool[i]->regBiggerScore[0]);

            mcu_name.str("");
            ecu_name.str("");
        }

        std::stringstream r_name;
        rtController = new ReductionController("ReductionController");
        rtController->clk(clk);
        rtController->rst(rst);
        rtController->start(start);
        for(int i = 0; i < Reduction_KIND; i ++) {
            ROCC[i](rtController->ROCC[i]);
        }
        for(int i = 0; i < Reduction_USAGE; i ++) {
            r_name << "reductionTree(" << i << ")";
            reductionTree[i] = new ReductionTree(r_name.str().c_str());
            reductionTree[i]->clk(clk);
            reductionTree[i]->rst(rst);
            reductionTree[i]->vecNotify(rtController->notifyOutArray[i]);
            reductionTree[i]->fifoIdx(rtController->fifoIdxArray[i]);
            for(int j = 0; j < Reduction_NUM; j ++) {
                reductionTree[i]->vecFromController[j](*(rtController->reductionOutArrayToTree[i][j]));
            }
            rtController->reduction_done[i](reductionTree[i]->done);
            assert(rIndex < RESULT_NUM && "Error: exceeding resultArray's bounds!");
            resultArray[rIndex++](reductionTree[i]->result);
            r_name.str("");
        }

        // ReductionTree FIFO Binding
        for(int i = 0; i < Reduction_FIFO_NUM; i ++) {
            reductionInputArray[i] = new sc_fifo<reductionInput>(MAX_SEGLENGTH);
            notifyArray[i] = new sc_signal<sc_int<WIDTH>>();
            rtController->reductionInputArrayPorts[i].bind(*reductionInputArray[i]);
            rtController->notifyArray[i](*notifyArray[i]);
        }
	}
};

#endif