module ilog2pp (
    input wire [31:0] v, 
    input wire clk,
    input wire reset,
	output wire [4:0] log2,
	output wire valid
);
    

// log近似值，1001和1111都选择3
	reg [3:0] LogTable256 [0:255];
	initial begin
LogTable256[0] = 4'd0;
LogTable256[1] = 4'd0;
LogTable256[2] = 4'd1;
LogTable256[3] = 4'd1;
LogTable256[4] = 4'd2;
LogTable256[5] = 4'd2;
LogTable256[6] = 4'd2;
LogTable256[7] = 4'd2;
LogTable256[8] = 4'd3;
LogTable256[9] = 4'd3;
LogTable256[10] = 4'd3;
LogTable256[11] = 4'd3;
LogTable256[12] = 4'd3;
LogTable256[13] = 4'd3;
LogTable256[14] = 4'd3;
LogTable256[15] = 4'd3;
LogTable256[16] = 4'd4;
LogTable256[17] = 4'd4;
LogTable256[18] = 4'd4;
LogTable256[19] = 4'd4;
LogTable256[20] = 4'd4;
LogTable256[21] = 4'd4;
LogTable256[22] = 4'd4;
LogTable256[23] = 4'd4;
LogTable256[24] = 4'd4;
LogTable256[25] = 4'd4;
LogTable256[26] = 4'd4;
LogTable256[27] = 4'd4;
LogTable256[28] = 4'd4;
LogTable256[29] = 4'd4;
LogTable256[30] = 4'd4;
LogTable256[31] = 4'd4;
LogTable256[32] = 4'd5;
LogTable256[33] = 4'd5;
LogTable256[34] = 4'd5;
LogTable256[35] = 4'd5;
LogTable256[36] = 4'd5;
LogTable256[37] = 4'd5;
LogTable256[38] = 4'd5;
LogTable256[39] = 4'd5;
LogTable256[40] = 4'd5;
LogTable256[41] = 4'd5;
LogTable256[42] = 4'd5;
LogTable256[43] = 4'd5;
LogTable256[44] = 4'd5;
LogTable256[45] = 4'd5;
LogTable256[46] = 4'd5;
LogTable256[47] = 4'd5;
LogTable256[48] = 4'd5;
LogTable256[49] = 4'd5;
LogTable256[50] = 4'd5;
LogTable256[51] = 4'd5;
LogTable256[52] = 4'd5;
LogTable256[53] = 4'd5;
LogTable256[54] = 4'd5;
LogTable256[55] = 4'd5;
LogTable256[56] = 4'd5;
LogTable256[57] = 4'd5;
LogTable256[58] = 4'd5;
LogTable256[59] = 4'd5;
LogTable256[60] = 4'd5;
LogTable256[61] = 4'd5;
LogTable256[62] = 4'd5;
LogTable256[63] = 4'd5;
LogTable256[64] = 4'd6;
LogTable256[65] = 4'd6;
LogTable256[66] = 4'd6;
LogTable256[67] = 4'd6;
LogTable256[68] = 4'd6;
LogTable256[69] = 4'd6;
LogTable256[70] = 4'd6;
LogTable256[71] = 4'd6;
LogTable256[72] = 4'd6;
LogTable256[73] = 4'd6;
LogTable256[74] = 4'd6;
LogTable256[75] = 4'd6;
LogTable256[76] = 4'd6;
LogTable256[77] = 4'd6;
LogTable256[78] = 4'd6;
LogTable256[79] = 4'd6;
LogTable256[80] = 4'd6;
LogTable256[81] = 4'd6;
LogTable256[82] = 4'd6;
LogTable256[83] = 4'd6;
LogTable256[84] = 4'd6;
LogTable256[85] = 4'd6;
LogTable256[86] = 4'd6;
LogTable256[87] = 4'd6;
LogTable256[88] = 4'd6;
LogTable256[89] = 4'd6;
LogTable256[90] = 4'd6;
LogTable256[91] = 4'd6;
LogTable256[92] = 4'd6;
LogTable256[93] = 4'd6;
LogTable256[94] = 4'd6;
LogTable256[95] = 4'd6;
LogTable256[96] = 4'd6;
LogTable256[97] = 4'd6;
LogTable256[98] = 4'd6;
LogTable256[99] = 4'd6;
LogTable256[100] = 4'd6;
LogTable256[101] = 4'd6;
LogTable256[102] = 4'd6;
LogTable256[103] = 4'd6;
LogTable256[104] = 4'd6;
LogTable256[105] = 4'd6;
LogTable256[106] = 4'd6;
LogTable256[107] = 4'd6;
LogTable256[108] = 4'd6;
LogTable256[109] = 4'd6;
LogTable256[110] = 4'd6;
LogTable256[111] = 4'd6;
LogTable256[112] = 4'd6;
LogTable256[113] = 4'd6;
LogTable256[114] = 4'd6;
LogTable256[115] = 4'd6;
LogTable256[116] = 4'd6;
LogTable256[117] = 4'd6;
LogTable256[118] = 4'd6;
LogTable256[119] = 4'd6;
LogTable256[120] = 4'd6;
LogTable256[121] = 4'd6;
LogTable256[122] = 4'd6;
LogTable256[123] = 4'd6;
LogTable256[124] = 4'd6;
LogTable256[125] = 4'd6;
LogTable256[126] = 4'd6;
LogTable256[127] = 4'd6;
LogTable256[128] = 4'd7;
LogTable256[129] = 4'd7;
LogTable256[130] = 4'd7;
LogTable256[131] = 4'd7;
LogTable256[132] = 4'd7;
LogTable256[133] = 4'd7;
LogTable256[134] = 4'd7;
LogTable256[135] = 4'd7;
LogTable256[136] = 4'd7;
LogTable256[137] = 4'd7;
LogTable256[138] = 4'd7;
LogTable256[139] = 4'd7;
LogTable256[140] = 4'd7;
LogTable256[141] = 4'd7;
LogTable256[142] = 4'd7;
LogTable256[143] = 4'd7;
LogTable256[144] = 4'd7;
LogTable256[145] = 4'd7;
LogTable256[146] = 4'd7;
LogTable256[147] = 4'd7;
LogTable256[148] = 4'd7;
LogTable256[149] = 4'd7;
LogTable256[150] = 4'd7;
LogTable256[151] = 4'd7;
LogTable256[152] = 4'd7;
LogTable256[153] = 4'd7;
LogTable256[154] = 4'd7;
LogTable256[155] = 4'd7;
LogTable256[156] = 4'd7;
LogTable256[157] = 4'd7;
LogTable256[158] = 4'd7;
LogTable256[159] = 4'd7;
LogTable256[160] = 4'd7;
LogTable256[161] = 4'd7;
LogTable256[162] = 4'd7;
LogTable256[163] = 4'd7;
LogTable256[164] = 4'd7;
LogTable256[165] = 4'd7;
LogTable256[166] = 4'd7;
LogTable256[167] = 4'd7;
LogTable256[168] = 4'd7;
LogTable256[169] = 4'd7;
LogTable256[170] = 4'd7;
LogTable256[171] = 4'd7;
LogTable256[172] = 4'd7;
LogTable256[173] = 4'd7;
LogTable256[174] = 4'd7;
LogTable256[175] = 4'd7;
LogTable256[176] = 4'd7;
LogTable256[177] = 4'd7;
LogTable256[178] = 4'd7;
LogTable256[179] = 4'd7;
LogTable256[180] = 4'd7;
LogTable256[181] = 4'd7;
LogTable256[182] = 4'd7;
LogTable256[183] = 4'd7;
LogTable256[184] = 4'd7;
LogTable256[185] = 4'd7;
LogTable256[186] = 4'd7;
LogTable256[187] = 4'd7;
LogTable256[188] = 4'd7;
LogTable256[189] = 4'd7;
LogTable256[190] = 4'd7;
LogTable256[191] = 4'd7;
LogTable256[192] = 4'd7;
LogTable256[193] = 4'd7;
LogTable256[194] = 4'd7;
LogTable256[195] = 4'd7;
LogTable256[196] = 4'd7;
LogTable256[197] = 4'd7;
LogTable256[198] = 4'd7;
LogTable256[199] = 4'd7;
LogTable256[200] = 4'd7;
LogTable256[201] = 4'd7;
LogTable256[202] = 4'd7;
LogTable256[203] = 4'd7;
LogTable256[204] = 4'd7;
LogTable256[205] = 4'd7;
LogTable256[206] = 4'd7;
LogTable256[207] = 4'd7;
LogTable256[208] = 4'd7;
LogTable256[209] = 4'd7;
LogTable256[210] = 4'd7;
LogTable256[211] = 4'd7;
LogTable256[212] = 4'd7;
LogTable256[213] = 4'd7;
LogTable256[214] = 4'd7;
LogTable256[215] = 4'd7;
LogTable256[216] = 4'd7;
LogTable256[217] = 4'd7;
LogTable256[218] = 4'd7;
LogTable256[219] = 4'd7;
LogTable256[220] = 4'd7;
LogTable256[221] = 4'd7;
LogTable256[222] = 4'd7;
LogTable256[223] = 4'd7;
LogTable256[224] = 4'd7;
LogTable256[225] = 4'd7;
LogTable256[226] = 4'd7;
LogTable256[227] = 4'd7;
LogTable256[228] = 4'd7;
LogTable256[229] = 4'd7;
LogTable256[230] = 4'd7;
LogTable256[231] = 4'd7;
LogTable256[232] = 4'd7;
LogTable256[233] = 4'd7;
LogTable256[234] = 4'd7;
LogTable256[235] = 4'd7;
LogTable256[236] = 4'd7;
LogTable256[237] = 4'd7;
LogTable256[238] = 4'd7;
LogTable256[239] = 4'd7;
LogTable256[240] = 4'd7;
LogTable256[241] = 4'd7;
LogTable256[242] = 4'd7;
LogTable256[243] = 4'd7;
LogTable256[244] = 4'd7;
LogTable256[245] = 4'd7;
LogTable256[246] = 4'd7;
LogTable256[247] = 4'd7;
LogTable256[248] = 4'd7;
LogTable256[249] = 4'd7;
LogTable256[250] = 4'd7;
LogTable256[251] = 4'd7;
LogTable256[252] = 4'd7;
LogTable256[253] = 4'd7;
LogTable256[254] = 4'd7;
LogTable256[255] = 4'd7;
	end

    // valid bit
    reg [31:0] stage1_v;
    reg stage1_valid;
    always @(posedge clk or posedge reset) begin
        if(reset) begin
            stage1_v <= 32'b0;
            stage1_valid <= 1'b0;
        end else begin
            stage1_v <= v;
            stage1_valid <= (v != 32'b0);
        end
    end

    //v[31:16]
    reg [15:0] stage2_v;
    reg [4:0] stage2_log2; 
    always @(posedge clk or posedge reset) begin
        if(reset) begin
           stage2_v <= 16'b0;
           stage2_log2 <= 5'b0; 
        end  else if(stage1_valid)begin
           if(stage1_v[31:16] != 0) begin
                stage2_v <= stage1_v[31:16]; 
                stage2_log2 <= 5'b10000;   // 16
           end else begin
                stage2_v <= stage1_v[15:0]; 
                stage2_log2 <= 5'b0000;
           end
        end
    end

    // v[31:24] / v[23:16]
    reg [4:0] stage3_v;
    always @(posedge clk or posedge reset) begin
        if(reset) begin
            stage3_v <= 5'b0;
        end else if(stage1_valid) begin
            if(stage2_v[15:8] != 8'b0) begin
                stage3_v <= stage2_log2 + 5'b01000 + LogTable256[stage2_v[15:8]];
            end else begin
                stage3_v <= stage2_log2 + LogTable256[stage2_v[7:0]];       
            end
        end 
    end

    assign valid = stage1_valid;
    assign log2 = stage3_v;

endmodule
