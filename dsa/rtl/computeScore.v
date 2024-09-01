module computeScore(
	input wire [31:0] riX,
	input wire [31:0] riY,
	input wire [31:0] qiX,
	input wire [31:0] qiY,
	input wire [31:0] W, 
	input wire [31:0] W_avg,
	output wire [31:0] result
)

	wire [31:0] diffX, diffY;
	wire [31:0] absDiff;
	wire [31:0] log2_val;
	wire valid;

	assign diffX = (riX > qiX) ? (riX - qiX) : (qiX - riX);
	assign diffY = (riY > qiY) ? (riY - qiY) : (qiY - riY);

	wire [31:0] minXY;
	assign minXY = (diffX > diffY) ? diffX : diffY;
	wire [31:0] A;
	assign A = (minXY < W) ? minXY : W;

	wire [31:0] B;
	wire [31:0] mult_result, log_component;
	wire [63:0] mult_temp;

	assign absDiff = (diffX > diffY) ? (diffX - diffY) : (diffY - diffX);
	assign mult_result = absDiff * W_avg / 100;

	ilog2 log2_cal(
		.v(absDiff),	
		.log2(log2_val),
		.valid(valid)
	);

	assign log_component = log2_val >> 1;
	assign B = (absDiff == 0) ? 32'd0 : (mult_result + log_component);
	assign result = A - B;

endmodule




