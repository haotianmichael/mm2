#ifndef ILOG2_H
#define ILOG2_H

#include <systemc.h>
#define FULL_WIDTH 32
#define LOGWIDTH 5

SC_MODULE(ilog2) {

    sc_in<bool> clk, rst;
    sc_in<sc_uint<FULL_WIDTH> > in;  // input
    sc_out<sc_uint<LOGWIDTH> > outlog2;  // output

    // pipeline port
    sc_signal<sc_uint<FULL_WIDTH> > stage1_v;
    sc_signal<bool> valid;
    sc_signal<sc_uint<16> > stage2_v;
    sc_signal<sc_uint<5> > stage2_log2;

    sc_signal<sc_uint<8> > stage3_indx;
    sc_signal<sc_uint<8> > stage3_log2;

    sc_signal<sc_uint<5> > stage4_logtable_value;
    sc_signal<sc_uint<5> > stage5_log2;
   
    sc_uint<4>  LogTable256[256] = {
        0, 0, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        6, 6, 6, 6, 6, 6, 6, 6,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7
    };

    SC_CTOR(ilog2) {
        SC_THREAD(initialize);
        sensitive_pos << clk;
        async_reset_signal_is(rst, true);

        SC_THREAD(stage1);
        sensitive_pos << clk;
        async_reset_signal_is(rst, true);

        SC_THREAD(stage2);
        sensitive_pos << clk;
        async_reset_signal_is(rst, true);

        SC_THREAD(stage3);
        sensitive_pos << clk;
        async_reset_signal_is(rst, true);

        SC_THREAD(stage4);
        sensitive_pos << clk;
        async_reset_signal_is(rst, true);

        SC_THREAD(end);
        sensitive_pos << clk;
        async_reset_signal_is(rst, true);
    }
/*       
LogTable256[0] = 0;
LogTable256[1] = 0;
LogTable256[2] = 1;
LogTable256[3] = 1; 
LogTable256[4] = 2; 
LogTable256[5] = 2; 
LogTable256[6] = 2; 
LogTable256[7] = 2; 
LogTable256[8] = 3; 
LogTable256[9] = 3; 
LogTable256[10] = 3; 
LogTable256[11] = 3; 
LogTable256[12] = 3; 
LogTable256[13] = 3; 
LogTable256[14] = 3; 
LogTable256[15] = 3; 
LogTable256[16] = 4; 
LogTable256[17] = 4; 
LogTable256[18] = 4;
LogTable256[19] = 4; 
LogTable256[20] = 4; 
LogTable256[21] = 4; 
LogTable256[22] = 4; 
LogTable256[23] = 4; 
LogTable256[24] = 4; 
LogTable256[25] = 4; 
LogTable256[26] = 4; 
LogTable256[27] = 4; 
LogTable256[28] = 4; 
LogTable256[29] = 4; 
LogTable256[30] = 4; 
LogTable256[31] = 4; 
LogTable256[32] = 5; 
LogTable256[33] = 5;
LogTable256[34] = 5; 
LogTable256[35] = 5; 
LogTable256[36] = 5; 
LogTable256[37] = 5; 
LogTable256[38] = 5; 
LogTable256[39] = 5; 
LogTable256[40] = 5; 
LogTable256[41] = 5; 
LogTable256[42] = 5;
LogTable256[43] = 5;
LogTable256[44] = 5;
LogTable256[45] = 5;
LogTable256[46] = 5;
LogTable256[47] = 5;
LogTable256[48] = 5;
LogTable256[49] = 5;
LogTable256[50] = 5;
LogTable256[51] = 5;
LogTable256[52] = 5;
LogTable256[53] = 5;
LogTable256[54] = 5;
LogTable256[55] = 5;
LogTable256[56] = 5;
LogTable256[57] = 5;
LogTable256[58] = 5;
LogTable256[59] = 5;
LogTable256[60] = 5;
LogTable256[61] = 5;
LogTable256[62] = 5;
LogTable256[63] = 5;
LogTable256[64] = 6;
LogTable256[65] = 6;
LogTable256[66] = 6;
LogTable256[67] = 6;
LogTable256[68] = 6;
LogTable256[69] = 6;
LogTable256[70] = 6;
LogTable256[71] = 6;
LogTable256[72] = 6;
LogTable256[73] = 6;
LogTable256[74] = 6;
LogTable256[75] = 6;
LogTable256[76] = 6;
LogTable256[77] = 6;
LogTable256[78] = 6;
LogTable256[79] = 6;
LogTable256[80] = 6;
LogTable256[81] = 6;
LogTable256[82] = 6;
LogTable256[83] = 6;
LogTable256[84] = 6;
LogTable256[85] = 6;
LogTable256[86] = 6;
LogTable256[87] = 6;
LogTable256[88] = 6;
LogTable256[89] = 6;
LogTable256[90] = 6;
LogTable256[91] = 6;
LogTable256[92] = 6;
LogTable256[93] = 6;
LogTable256[94] = 6;
LogTable256[95] = 6;
LogTable256[96] = 6;
LogTable256[97] = 6;
LogTable256[98] = 6;
LogTable256[99] = 6;
LogTable256[100] = 6;
LogTable256[101] = 6;
LogTable256[102] = 6;
LogTable256[103] = 6;
LogTable256[104] = 6;
LogTable256[105] = 6;
LogTable256[106] = 6;
LogTable256[107] = 6;
LogTable256[108] = 6;
LogTable256[109] = 6;
LogTable256[110] = 6;
LogTable256[111] = 6;
LogTable256[112] = 6;
LogTable256[113] = 6;
LogTable256[114] = 6;
LogTable256[115] = 6;
LogTable256[116] = 6;
LogTable256[117] = 6;
LogTable256[118] = 6;
LogTable256[119] = 6;
LogTable256[120] = 6;
LogTable256[121] = 6;
LogTable256[122] = 6;
LogTable256[123] = 6;
LogTable256[124] = 6;
LogTable256[125] = 6;
LogTable256[126] = 6;
LogTable256[127] = 6;
LogTable256[128] = 7;
LogTable256[129] = 7;
LogTable256[130] = 7;
LogTable256[131] = 7;
LogTable256[132] = 7;
LogTable256[133] = 7;
LogTable256[134] = 7;
LogTable256[135] = 7;
LogTable256[136] = 7;
LogTable256[137] = 7;
LogTable256[138] = 7;
LogTable256[139] = 7;
LogTable256[140] = 7;
LogTable256[141] = 7;
LogTable256[142] = 7;
LogTable256[143] = 7;
LogTable256[144] = 7;
LogTable256[145] = 7;
LogTable256[146] = 7;
LogTable256[147] = 7;
LogTable256[148] = 7;
LogTable256[149] = 7;
LogTable256[150] = 7;
LogTable256[151] = 7;
LogTable256[152] = 7;
LogTable256[153] = 7;
LogTable256[154] = 7;
LogTable256[155] = 7;
LogTable256[156] = 7;
LogTable256[157] = 7;
LogTable256[158] = 7;
LogTable256[159] = 7;
LogTable256[160] = 7;
LogTable256[161] = 7;
LogTable256[162] = 7;
LogTable256[163] = 7;
LogTable256[164] = 7;
LogTable256[165] = 7;
LogTable256[166] = 7;
LogTable256[167] = 7;
LogTable256[168] = 7;
LogTable256[169] = 7;
LogTable256[170] = 7;
LogTable256[171] = 7;
LogTable256[172] = 7;
LogTable256[173] = 7;
LogTable256[174] = 7;
LogTable256[175] = 7;
LogTable256[176] = 7;
LogTable256[177] = 7;
LogTable256[178] = 7;
LogTable256[179] = 7;
LogTable256[180] = 7;
LogTable256[181] = 7;
LogTable256[182] = 7;
LogTable256[183] = 7;
LogTable256[184] = 7;
LogTable256[185] = 7;
LogTable256[186] = 7;
LogTable256[187] = 7;
LogTable256[188] = 7;
LogTable256[189] = 7;
LogTable256[190] = 7;
LogTable256[191] = 7;
LogTable256[192] = 7;
LogTable256[193] = 7;
LogTable256[194] = 7;
LogTable256[195] = 7;
LogTable256[196] = 7;
LogTable256[197] = 7;
LogTable256[198] = 7;
LogTable256[199] = 7;
LogTable256[200] = 7;
LogTable256[201] = 7;
LogTable256[202] = 7;
LogTable256[203] = 7;
LogTable256[204] = 7;
LogTable256[205] = 7;
LogTable256[206] = 7;
LogTable256[207] = 7;
LogTable256[208] = 7;
LogTable256[209] = 7;
LogTable256[210] = 7;
LogTable256[211] = 7;
LogTable256[212] = 7;
LogTable256[213] = 7;
LogTable256[214] = 7;
LogTable256[215] = 7;
LogTable256[216] = 7;
LogTable256[217] = 7;
LogTable256[218] = 7;
LogTable256[219] = 7;
LogTable256[220] = 7;
LogTable256[221] = 7;
LogTable256[222] = 7;
LogTable256[223] = 7;
LogTable256[224] = 7;
LogTable256[225] = 7;
LogTable256[226] = 7;
LogTable256[227] = 7;
LogTable256[228] = 7;
LogTable256[229] = 7;
LogTable256[230] = 7;
LogTable256[231] = 7;
LogTable256[232] = 7;
LogTable256[233] = 7;
LogTable256[234] = 7;
LogTable256[235] = 7;
LogTable256[236] = 7;
LogTable256[237] = 7;
LogTable256[238] = 7;
LogTable256[239] = 7;
LogTable256[240] = 7;
LogTable256[241] = 7;
LogTable256[242] = 7;
LogTable256[243] = 7;
LogTable256[244] = 7;
LogTable256[245] = 7;
LogTable256[246] = 7;
LogTable256[247] = 7;
LogTable256[248] = 7;
LogTable256[249] = 7;
LogTable256[250] = 7;
LogTable256[251] = 7;
LogTable256[252] = 7;
LogTable256[253] = 7;
LogTable256[254] = 7;
LogTable256[255] = 7;
}
*/

    void initialize();
    void stage1();
    void stage2();
    void stage3();
    void stage4();
    void end();
};

#endif