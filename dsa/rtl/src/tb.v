module tb;

	reg [31:0] riX, riY, qiX, qiY, W, W_avg;
	wire [31:0] result;
	reg clk;
	reg [7:0] cycle_count;


	computeScore uut(
		.riX(riX),
		.riY(riY),
		.qiX(qiX),
		.qiY(qiY),
		.W(w),
		.W_avg(W_avg),
		.result(result)
	);

	initial begin

		clk = 0;
		forever #5 clk = ~clk;
	end

	initial begin

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

		while(cycle_count < 20) begin
			@(posedge clk);
			$display("Cycle: %d, Result: %d", cycle_count, result);

			cycle_count = cycle_count + 1;
		end

    $fnish;
	end

endmodule
