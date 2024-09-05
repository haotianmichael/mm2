`include "./float/multiplier.sv"

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
	

    reg [31:0] absDiff, A, min;
	always @(posedge clk or posedge reset) begin
		if(reset)begin
			absDiff <= 32'd0;
			min <= 32'b0;
		end else begin
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

    reg [31:0] mult_result;
	wire [31:0] mult_result_inter;
	wire output_en;
	multiplier mul_uut(
		.input_a(absDiff),
		.input_b(W_avg),
		.input_a_stb(1'b1),
		.input_b_stb(1'b1),
		.clk(clk),
		.rst(reset),
		.output_z(mult_result_inter),
		.output_z_stb(output_en)
	);
	always @(posedge clk or posedge reset) begin
		if(reset) begin
			mult_result <= 0;
		end else if(output_en)begin
			mult_result <= mult_result_inter;  // W_avg should be larger
		end	
	end

	reg [31:0] div_result;
	always @(posedge clk or posedge reset) begin
		if(reset) begin
			div_result <= 0;
		end else begin
			div_result <= (mult_result >> 6);
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
			partialSum <= div_result + log2_result;	 // >>1 done in ilog2pp.v
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