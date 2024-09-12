module tb;

	reg [31:0] riX, riY, qiX, qiY, W, W_avg;
	wire [31:0] result;
	reg clk, reset;
	reg [7:0] cycle_count;


	computeScorepp uut(
    .clk(clk),
    .reset(reset),
		.riX(riX),
		.riY(riY),
		.qiX(qiX),
		.qiY(qiY),
		.W(W),
		.W_avg(W_avg),
		.result(result)
	);

	initial begin
		clk = 0;
	    reset = 0;
	end
	always #5 clk = ~clk;

	initial begin
		#10;
		reset = 1;
		#20;
	    reset = 0;
		riX = 32'd100;
		riY = 32'd30;
		qiX = 32'd50;
		qiY = 32'd20;
		W = 32'd40;
		W_avg = 32'd40;
		cycle_count = 0;
		$display("Cycle: %d, Result: %d", cycle_count, result);

		@(posedge clk);
		cycle_count = 1;

		while(cycle_count < 100) begin
			@(posedge clk);
			$display("Cycle: %d, Result: %d", cycle_count, result);

			cycle_count = cycle_count + 1;
		end

    $finish;
	end

endmodule
