module computeScore(
	input wire [31:0] riX,
	input wire [31:0] riY,
	input wire [31:0] qiX,
	input wire [31:0] qiY,
	input wire [31:0] W, 
	input wire [31:0] W_avg,
	output wire [31:0] result
)

	wire [31:0] diffR, diffQ;
	wire [31:0] absDiff;
	wire [31:0] log2_val;
	wire valid;

	assign diffR = (riX > riY) ? (riX - riY) : (riY - riX);
	assign diffQ = (qiX > qiY) ? (qiX - qiY) : (qiY - qiX);

	wire [31:0] min;
	assign min = (diffR < diffQ) ? diffR : diffQ;
	wire [31:0] A;
	assign A = (min < W) ? min : W;

	wire [31:0] B;
	wire [31:0] mult_result, log_component;
	wire [63:0] mult_temp;

	assign absDiff = (diffR > diffQ) ? (diffR - diffQ) : (diffQ - diffR);
	assign mult_result = absDiff * W_avg / 100;

	ilog2 log2_val(
		.v(absDiff),	
		.log2(log2_val),
		.valid(valid)
	);

	assign log_component = log2_val >> 1;
	assign B = (absDiff == 0) ? 32'd0 : (mult_result + log_component);
	assign result = A - B;

endmodule




