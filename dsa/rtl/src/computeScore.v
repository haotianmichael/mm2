module computeScore(
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
	always @(posedge clk or posedge reset) begin
		if(reset) begin
			diffR <= 32'd0;
			diffQ <= 32'd0;
		end else begin
			diffR <= (riX > riY) ? (riX - riY) : (riY - riX);
			diffQ <= (qiX > qiY) ? (qiX - qiY) : (qiY - qiX);
		end	
	end


	reg [31:0] absDiff, A, min;
	always @(posedge clk or posedge reset) begin
		if(reset)begin
			absDiff <= 32'd0;
			A <= 32'd0;
			min <= 32'd0;
		end else begin
			absDiff = (diffR > diffQ) ? (diffR - diffQ) : (diffQ - diffR);
			min <= (diffR < diffQ) ? diffR : diffQ;
			A <= (min < W) ? min : W;
		end
	end


	wire [4:0] log2_val;
	wire valid;
			ilog2 log2_cal(
				.clk(clk),
				.reset(reset),
				.v(absDiff),	
				.log2(log2_val),
				.valid(valid)
			);
	reg [31:0] log_component, mult_result;
	reg [31:0] B;
	always @(posedge clk or posedge reset) begin
		if(reset) begin
			B <= 32'd0;
			log_component <= 32'd0;
			mult_result <= 32'd0;
		end else begin
		mult_result = absDiff * W_avg / 100;
			log_component = log2_val >> 1;
			B = (absDiff == 0) ? 32'd0 : (mult_result + log_component);
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




