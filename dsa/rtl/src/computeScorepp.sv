module computeScorepp(
	input wire clk,
	input wire reset,
	input wire [31:0] riX,
	input wire [31:0] riY,
	input wire [31:0] qiX,
	input wire [31:0] qiY,
	input wire [31:0] W, 
	input wire [31:0] W_avg,
	output reg [31:0] result
);

    parameter [31:0] CONST_15 = 32'h3E19999A;  // 15 * 0.01 = 0.15
    parameter [31:0] CONST_21 = 32'h3E6B851F;  // 21 * 0.01 = 0.21
    parameter [31:0] CONST_25 = 32'h3E800000;  // 25 * 0.01 = 0.25
    parameter [31:0] CONST_31 = 32'h3EA1C28F;  // 31 * 0.01 = 0.31

    reg [31:0] diffR, diffQ;
	reg [31:0] tmpRXY, tmpRYX;
	always @(posedge clk or posedge reset) begin
		if(reset) begin
			tmpRXY <= 0;
		end else begin
			tmpRXY <= riX - riY;
		end	
	end
	always @(posedge clk or posedge reset) begin
		if(reset) begin
			tmpRYX <= 0;
		end else begin
			tmpRYX <= riY - riX;
		end	
	end
	always @(posedge clk or posedge reset) begin
		if(reset) begin
			diffR <= 32'd0;
		end else begin
			diffR <= (riX > riY) ? tmpRXY : tmpRYX;
		end	
	end

	reg [31:0] tmpQXY, tmpQYX;
	always @(posedge clk or posedge reset) begin
		if(reset)begin
			tmpQXY <= 0;
		end else begin
			tmpQXY <= qiX - qiY;
		end	
	end
	always @(posedge clk or posedge reset) begin
		if(reset) begin
			tmpQYX <= 0;
		end else begin
			tmpQYX <= qiY - qiX;
		end	
	end
	always @(posedge clk or posedge reset) begin
		if(reset) begin
			diffQ <= 32'd0;
		end else begin
			diffQ <= (qiX > qiY) ? tmpQXY : tmpQYX;
		end	
	end


	reg [31:0] tmp_RQ, tmp_QR;
	always @(posedge clk or posedge reset) begin
		if(reset) begin
			tmp_QR <= 32'b0;
		end else begin
			tmp_QR <= diffQ - diffR;
		end
	end
	always @(posedge clk or posedge reset) begin
		if(reset) begin
			tmp_RQ <= 32'b0;
		end else begin 
			tmp_RQ <= diffR - diffQ;
		end	
	end
	
	/**************************************************************
	 @Computed Value 
		||riX - riY| - |qiX - qiY||	——> absDiff
		min{|riX - riY|, |qiX - qiY|, W} ——> min

	 @Floating Computing
		1. specifying FLOATING CONSTANT. 
		2. INT to 32FLOAT
		3. FLOAT MULOp
		4. 32FLOAT to INT
	**************************************************************/

	wire ioutput_en, iinput_a_ack, iinput_a_stb;
	reg ioutput_z_ack;
    reg [31:0] absDiff, A, min;
	always @(posedge clk or posedge reset) begin
		if(reset)begin
			absDiff <= 32'd0;
			min <= 32'b0;
			iinput_a_stb <= 0;
		end else if(iinput_a_ack == 0)begin
			iinput_a_stb <= 0;
			if(diffR > diffQ) begin
				absDiff <= tmp_RQ;
				min <= diffQ;
			end else begin
				absDiff <= tmp_QR;
				min <= diffR;
			end
		end
	end
	always @(posedge clk or posedge reset) begin
		if(reset) begin
			A <= 32'd0;
		end else begin
			A <= (min < W) ? min : W;
		end	
	end

	wire moutput_en, minput_a_ack, minput_b_ack;
	reg moutput_z_ack;
	reg [31:0] float_W;
	always @(posedge clk or posedge reset) begin
		if(reset) begin
			float_W <= 0;	
		end else if(minput_b_ack)begin
			case (W_avg)
				15:  float_W <= CONST_15;
				21:  float_W <= CONST_21;
				25:  float_W <= CONST_25;
				31:  float_W <= CONST_31;
				default:  float_W <= CONST_15;
			endcase
		end
	end

	reg [31:0] float_absDiff, float_absDiff_inter;
	int2float i2fuut(
		.input_a(absDiff),
		.input_a_stb(iinput_a_stb),
		.output_z_ack(ioutput_z_ack),
		.clk(clk),
		.rst(reset),
		.output_z(float_absDiff_inter),
		.input_a_ack(iinput_a_ack),
		.output_z_stb(ioutput_en)
	);
	always @(posedge clk or posedge reset) begin
		if(reset)begin
			float_absDiff <= 32'b0;
			ioutput_z_ack <= 0;
		end else if(ioutput_en && minput_a_ack)begin
			float_absDiff <= float_absDiff_inter;
			ioutput_z_ack <= 1;
		end else begin
			float_absDiff <= 32'b0;
		end
	end

    reg [31:0] float_mult_result;
	wire [31:0] float_mult_result_inter;
	multiplier mul_uut(
		.input_a(float_absDiff),
		.input_b(float_W),
		.input_a_stb(1'b1),
		.input_b_stb(1'b1),
		.output_z_ack(moutput_z_ack),
		.clk(clk),
		.rst(reset),
		.output_z(float_mult_result_inter),
		.output_z_stb(moutput_en),
		.input_a_ack(minput_a_ack),
		.input_b_ack(minput_b_ack)
	);
	always @(posedge clk or posedge reset) begin
		if(reset) begin
			float_mult_result <= 0;
			moutput_z_ack <= 0;
		end else if(moutput_en && finput_a_ack)begin
			float_mult_result <= float_mult_result_inter;  
			moutput_z_ack <= 1;
		end	
	end

	wire foutput_en, finput_a_ack;
	reg foutput_z_ack;
	reg [31:0] mult_result_inter, mult_result;
	float2int f2iuut(
		.input_a(float_mult_result),
		.input_a_stb(1'b1),
		.output_z_ack(foutput_z_ack),
		.clk(clk),
		.rst(rst),
		.output_z(mult_result_inter),
		.output_z_stb(foutput_en),
		.input_a_ack(finput_a_ack)
	);
	always @(posedge clk or posedge reset) begin
		if(reset) begin
			mult_result <= 0;	
			foutput_z_ack <= 0;
		end else if(foutput_en)begin
			mult_result <= mult_result_inter;
			foutput_z_ack <= 1;
		end	
	end



    wire [4:0] log2_val;
    wire valid;
    ilog2pp log2_cal(
	    .clk(clk),
		.reset(reset),
		.v(absDiff),	
		.log2(log2_val),
		.valid(valid)
	); 

	reg [31:0] log2_result;
	reg valid_result;
	always @(posedge clk or posedge reset) begin
		if(reset)begin
			log2_result <= 0;
			valid_result <= 0;
		end else begin
			log2_result <= log2_val;
			valid_result <= valid;
		end	
	end

	reg [31:0] partialSum;
	always @(posedge clk or posedge reset) begin
		if(reset) begin
			partialSum <= 0;
		end else begin
			partialSum <= mult_result + log2_result;	 // >>1 done in ilog2pp.v
		end	
	end

	reg [31:0] B;
	always @(posedge clk or posedge reset) begin
		if(reset) begin
			B <= 32'd0;
		end else begin
			B <= (absDiff == 0) ? 32'd0 : partialSum;
		end	
	end


    always @(posedge clk or posedge reset) begin
		if(reset) begin
			result <= 32'd0;
		end else begin 
			result <= A - B;
		end	
	end

endmodule
