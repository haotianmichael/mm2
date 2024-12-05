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
    std::vector<std::vector<sc_signal<sc_int<WIDTH>>*>> anchorIdx;
    std::vector<std::vector<sc_signal<sc_uint<WIDTH>>*>> anchorSegNum;
    int **anchorSuccessiveRange;// successive range of every anchor 
    std::vector<sc_signal<sc_int<WIDTH>>*> anchorW;  //number of anchors

    /*ResultArray consists of both HCU's resultArray(UB<=65) and ReductionPool's resultArray(UB>65) */
    std::vector<sc_out<sc_int<WIDTH>>> resultArray;
    std::vector<sc_out<sc_int<WIDTH>>> resultAddrArray;

    // @RC Unit
    /*
        taking all 200000 reads at rst.neg()
    */
    RangeCountUnit *rc;

    // @LocalRAM (Two Ports)
    /*
       sizeof(ram_data) : 40018 Bytes 
       sizeof(RAM): 40018B x 40 x 2 = 3.053MB
    */
    // UpperBound <= 65 elements  Lane[0, 64]
    sc_int<WIDTH> readIdx = 0; //read which currently being cut
    std::vector<ram_dataForShort> localRAMForShort;
    std::vector<ram_dataForLong> localRAMForLong;
    std::vector<bool> freeListForShort;
    std::vector<bool> freeListForLong;
    int ramIndexForShort;
    int ramIndexForLong;
    sc_event ram_not_full;

    // @SchedulerTable
    /*
        FIXME: schedulerTable needs to be filled more than one time within one cycle.
    */
    SchedulerTable *schedulerTable;

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
    std::vector<std::vector<sc_signal<sc_int<WIDTH>>*>> mri;
    std::vector<std::vector<sc_signal<sc_int<WIDTH>>*>> mqi;
    std::vector<std::vector<sc_signal<sc_int<WIDTH>>*>> mw;
    std::vector<std::vector<sc_signal<sc_int<WIDTH>>*>> midx;
    std::vector<std::vector<sc_signal<sc_int<WIDTH>>*>> eri;
    std::vector<std::vector<sc_signal<sc_int<WIDTH>>*>> eqi;
    std::vector<std::vector<sc_signal<sc_int<WIDTH>>*>> ew;
    std::vector<std::vector<sc_signal<sc_int<WIDTH>>*>> eidx;
    std::vector<sc_signal<sc_int<WIDTH>>*> ecu_ri;
    std::vector<sc_signal<sc_int<WIDTH>>*> ecu_qi;
    std::vector<sc_signal<sc_int<WIDTH>>*> ecu_w;
    std::vector<sc_signal<sc_int<WIDTH>>*> ecu_idx;
    std::vector<sc_signal<sc_int<WIDTH>>*> ecu_score;

    // @ReductionPool
    ReductionController *rtController;
    ReductionTree *reductionTree[Reduction_USAGE];
    sc_in<sc_int<WIDTH>> ROCC[Reduction_KIND];
    sc_fifo<reductionInput> *reductionInputArray[Reduction_FIFO_NUM];        
    sc_signal<sc_int<WIDTH>> *notifyArray[Reduction_FIFO_NUM];

    std::ofstream result_file;

    void chain_ram_check();
    void chain_hcu_cut();
    void chain_hcu_execute();
    void chain_hcu_fillTable();
    void chain_hcu_allocate();
    void chain_ram_write_score();
    void chain_ram_read_score();
    void chain_rt_allocate();

	SC_CTOR(Chain) : 
         ramIndexForLong(0),
         ramIndexForShort(0),
         alloParam(0),
         freeParam(0),
         anchorNum(ReadNumProcessedOneTime),
         anchorW(ReadNumProcessedOneTime),
         anchorRi(ReadNumProcessedOneTime, std::vector<sc_signal<sc_int<WIDTH>>*>(MAX_READ_LENGTH)),
         anchorQi(ReadNumProcessedOneTime, std::vector<sc_signal<sc_int<WIDTH>>*>(MAX_READ_LENGTH)),
         anchorIdx(ReadNumProcessedOneTime, std::vector<sc_signal<sc_int<WIDTH>>*>(MAX_READ_LENGTH)),
         anchorSegNum(ReadNumProcessedOneTime, std::vector<sc_signal<sc_uint<WIDTH>>*>(MAX_READ_LENGTH)),
         mri(HCU_NUM, std::vector<sc_signal<sc_int<WIDTH>>*>(MCUInputLaneWIDTH+1)),
         mqi(HCU_NUM, std::vector<sc_signal<sc_int<WIDTH>>*>(MCUInputLaneWIDTH+1)),
         mw(HCU_NUM, std::vector<sc_signal<sc_int<WIDTH>>*>(MCUInputLaneWIDTH+1)),
         midx(HCU_NUM, std::vector<sc_signal<sc_int<WIDTH>>*>(MCUInputLaneWIDTH+1)),
         eri(HCU_NUM, std::vector<sc_signal<sc_int<WIDTH>>*>(MCUInputLaneWIDTH+1)),
         eqi(HCU_NUM, std::vector<sc_signal<sc_int<WIDTH>>*>(MCUInputLaneWIDTH+1)),
         ew(HCU_NUM, std::vector<sc_signal<sc_int<WIDTH>>*>(MCUInputLaneWIDTH+1)),
         eidx(HCU_NUM, std::vector<sc_signal<sc_int<WIDTH>>*>(MCUInputLaneWIDTH+1)),
         ecu_ri(HCU_NUM),
         ecu_qi(HCU_NUM),
         ecu_w(HCU_NUM),
         ecu_idx(HCU_NUM),
         ecu_score(HCU_NUM),
         resultArray(Reduction_USAGE),
         resultAddrArray(Reduction_USAGE){

        SC_THREAD(chain_ram_check);
        sensitive << clk.pos();

        // prepare the segmentQueue
        // one read per cycle.
        SC_THREAD(chain_hcu_cut);
        sensitive << clk.pos();

        // fill SchedulerTable for every Segment
        SC_THREAD(chain_hcu_fillTable);
        sensitive << clk.pos();

        // allocate HCU for every Segment based on SchedulerTable
        SC_THREAD(chain_hcu_allocate);
        sensitive << clk.pos();

        // cache former final-score 
        SC_THREAD(chain_ram_write_score);
        sensitive << clk.pos();

        // read former final-score for computing other anchor's schore
        SC_THREAD(chain_ram_read_score);
        sensitive << clk.pos();

        // HCU IO Wiring and Enable
        SC_THREAD(chain_hcu_execute);
        sensitive << clk.pos();

        // check SchedulerTable and allocate ReductionTrees
        SC_THREAD(chain_rt_allocate);
        sensitive << clk.pos();

        /************@RC Unit ****************************************/
        for(int i = 0; i < ReadNumProcessedOneTime; i ++) {
            anchorNum[i] = new sc_signal<sc_int<WIDTH>>();
            anchorW[i] = new sc_signal<sc_int<WIDTH>>();
            for(int j = 0; j < MAX_READ_LENGTH; j ++) {
                try {
                    anchorRi[i][j] = new sc_signal<sc_int<WIDTH>>();
                    anchorQi[i][j] = new sc_signal<sc_int<WIDTH>>();
                    anchorIdx[i][j] = new sc_signal<sc_int<WIDTH>>();
                    anchorSegNum[i][j] = new sc_signal<sc_uint<WIDTH>>();
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
        rc->readDone(start);
        anchorSuccessiveRange = new int*[ReadNumProcessedOneTime];
        for(int readIdx = 0; readIdx < ReadNumProcessedOneTime; readIdx++) {
            rc->anchorNum[readIdx]->bind(*anchorNum[readIdx]);
            rc->anchorW[readIdx]->bind(*anchorW[readIdx]);
            anchorSuccessiveRange[readIdx] = new int[MAX_READ_LENGTH];
            for(int i = 0; i < MAX_READ_LENGTH; i ++) {
                rc->anchorRi[readIdx][i]->bind(*anchorRi[readIdx][i]);
                rc->anchorQi[readIdx][i]->bind(*anchorQi[readIdx][i]);
                rc->anchorIdx[readIdx][i]->bind(*anchorIdx[readIdx][i]);
                rc->anchorSegNum[readIdx][i]->bind(*anchorSegNum[readIdx][i]);
            }
        }

        /************@SchedulerTable ****************************************/
        schedulerTable = new SchedulerTable;
        try {
            // FIXME: memory maybe not enough.
            localRAMForLong.resize(RAM_SIZE);
            localRAMForShort.resize(RAM_SIZE);
            freeListForLong.resize(RAM_SIZE, true);
            freeListForShort.resize(RAM_SIZE, true);
        }catch(const std::bad_alloc& e) {
            std::cerr << "Scheduler's localRAM allocation failed:" << e.what() << std::endl;
        }

        /************@Wiring****************************************/
        for(int i = 0; i < HCU_NUM; i ++) {
            ecu_ri[i] = new sc_signal<sc_int<WIDTH>>();
            ecu_qi[i] = new sc_signal<sc_int<WIDTH>>();
            ecu_w[i] = new sc_signal<sc_int<WIDTH>>();
            ecu_idx[i] = new sc_signal<sc_int<WIDTH>>();
            ecu_score[i] = new sc_signal<sc_int<WIDTH>>();
            for(int j = 0; j < MCUInputLaneWIDTH + 1; j ++) {
                try {
                    mri[i][j] = new sc_signal<sc_int<WIDTH>>();
                    mqi[i][j] = new sc_signal<sc_int<WIDTH>>();
                    mw[i][j] = new sc_signal<sc_int<WIDTH>>();
                    midx[i][j] = new sc_signal<sc_int<WIDTH>>();
                    eri[i][j] = new sc_signal<sc_int<WIDTH>>();
                    eqi[i][j] = new sc_signal<sc_int<WIDTH>>();
                    ew[i][j] = new sc_signal<sc_int<WIDTH>>();
                    eidx[i][j] = new sc_signal<sc_int<WIDTH>>();
                }catch(const std::bad_alloc& e) {
                    std::cerr << "Scheduler Memory allocation failed:" << e.what() << std::endl;
                }
            }
        }

        /************@IODispatcher ****************************************/
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
                mcuIODisPatcherPool[i]->ri_out[j](*mri[i][j]);
                mcuIODisPatcherPool[i]->qi_out[j](*mqi[i][j]);
                mcuIODisPatcherPool[i]->w_out[j](*mw[i][j]);
                mcuIODisPatcherPool[i]->idx_out[j](*midx[i][j]);
                ecuIODisPatcherPool[i]->ri_out[j](*eri[i][j]);
                ecuIODisPatcherPool[i]->qi_out[j](*eqi[i][j]);
                ecuIODisPatcherPool[i]->w_out[j](*ew[i][j]);
                ecuIODisPatcherPool[i]->idx_out[j](*eidx[i][j]);
            }
            ecuIODisPatcherPool[i]->ecu_ri_out(*ecu_ri[i]);
            ecuIODisPatcherPool[i]->ecu_qi_out(*ecu_qi[i]);
            ecuIODisPatcherPool[i]->ecu_w_out(*ecu_w[i]);
            ecuIODisPatcherPool[i]->ecu_idx_out(*ecu_idx[i]);
            ecuIODisPatcherPool[i]->ecu_score_out(*ecu_score[i]);
            mcuIO_name.str("");
            ecuIO_name.str("");
        }

        
        /************@HCU ****************************************/
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
                mcuPool[i]->riArray[j](*mri[i][j]);
                mcuPool[i]->qiArray[j](*mqi[i][j]);
                mcuPool[i]->W[j](*mw[i][j]);
                mcuPool[i]->Idx[j](*midx[i][j]);
                ecuPool[i]->riArray[j](*eri[i][j]);
                ecuPool[i]->qiArray[j](*eqi[i][j]);
                ecuPool[i]->W[j](*ew[i][j]);
                ecuPool[i]->Idx[j](*eidx[i][j]);
                ecuPool[i]->ecu_ri[j](*ecu_ri[i]);
                ecuPool[i]->ecu_qi[j](*ecu_qi[i]);
                ecuPool[i]->ecu_w[j](*ecu_w[i]);
                ecuPool[i]->ecu_idx[j](*ecu_idx[i]);
                ecuPool[i]->ecu_final_score[j](*ecu_score[i]);
            }

            
            mcuPool[i]->currentReadID.write(static_cast<sc_int<WIDTH> >(-1));
            mcuPool[i]->currentSegID.write(static_cast<sc_int<WIDTH> >(-1));
            mcuPool[i]->LowerBound.write(static_cast<sc_int<WIDTH> >(-1));
            mcuPool[i]->UpperBound.write(static_cast<sc_int<WIDTH> >(-1));
            mcuPool[i]->type.write(static_cast<bool>(0));
            mcuPool[i]->executeTime.write(sc_time(0, SC_NS));
            mcuPool[i]->freeTime.write(sc_time(0, SC_NS));

            ecuPool[i]->currentReadID.write(static_cast<sc_int<WIDTH> >(-1));
            ecuPool[i]->currentSegID.write(static_cast<sc_int<WIDTH> >(-1));
            ecuPool[i]->LowerBound.write(static_cast<sc_int<WIDTH> >(-1));
            ecuPool[i]->UpperBound.write(static_cast<sc_int<WIDTH> >(-1));
            ecuPool[i]->type.write(static_cast<bool>(0));
            ecuPool[i]->executeTime.write(sc_time(0, SC_NS));
            ecuPool[i]->freeTime.write(sc_time(0, SC_NS));

            mcu_name.str("");
            ecu_name.str("");
        }


        /************@ReductionTree ****************************************/
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
            reductionTree[i]->address(rtController->reductionOutArrayAddress[i]);
            for(int j = 0; j < Reduction_NUM; j ++) {
                reductionTree[i]->vecFromController[j](*(rtController->reductionOutArrayToTree[i][j]));
                reductionTree[i]->vecPredecessor[j](*(rtController->reductionOutArrayPredecessor[i][j]));
            }
            rtController->reduction_done[i](reductionTree[i]->done);
            assert(rIndex < RESULT_NUM && "Error: exceeding resultArray's bounds!");
            resultArray[rIndex](reductionTree[i]->result);
            resultAddrArray[rIndex++](reductionTree[i]->out_address);
            r_name.str("");
        }

        // ReductionTree FIFO Binding
        for(int i = 0; i < Reduction_FIFO_NUM; i ++) {
            reductionInputArray[i] = new sc_fifo<reductionInput>(MAX_SEGLENGTH);
            notifyArray[i] = new sc_signal<sc_int<WIDTH>>();
            rtController->reductionInputArrayPorts[i].bind(*reductionInputArray[i]);
            rtController->notifyArray[i](*notifyArray[i]);
        }

        /************@Output ****************************************/
        result_file.open("output/out.txt", std::ios::out | std::ios::app);
        if(!result_file.is_open()) {
            std::cerr << "Error: cannot open file for output!" << std::endl;
            sc_stop();
        }
	}
};

#endif