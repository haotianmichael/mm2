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

    reg [31:0] mult_result;
	always @(posedge clk or posedge reset) begin
		if(reset) begin
			mult_result <= 0;
		end else begin
			mult_result <= absDiff * W_avg;
		end	
	end

	reg [31:0] div_result;
	always @(posedge clk or posedge reset) begin
		if(reset) begin
			div_result <= 0;
		end else begin
			div_result <= mult_result / 100;
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
			B = (absDiff == 0) ? 32'd0 : partialSum;
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