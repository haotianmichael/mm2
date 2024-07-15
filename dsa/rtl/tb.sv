`include "chain.v"

module topTb();

    parameter FILE = "in.txt";    
    parameter READ_NUM = 30000;
    parameter MAX_ANCHORS = 400000;
    parameter INT32_WIDTH = 32;
    parameter INT64_WIDTH = 64;
    typedef logic signed [INT32_WIDTH-1 : 0] loc_t;
    typedef logic signed [INT32_WIDTH-1 : 0] width_t;
    typedef logic signed [INT32_WIDTH-1 : 0] tag_t;
    typedef struct packed {
        tag_t tag;
        loc_t x;
        loc_t y;
        width_t w; 
    } anchor_t;
    typedef struct packed {
        logic [INT64_WIDTH-1 : 0] n;
        float avg_span;
        int max_dist_x, max_dist_y, bw;
        anchor_t [MAX_ANCHORS_1 : 0] anchors;
    } read_t;
    typedef struct packed {
        logic [INT64_WIDTH-1 : 0] x;
        logic [INT64_WIDTH-1 : 0] y;
    } mm128_t;

    module mm_chain_dp(
        input logic clk,
        input logic rst,
        input logic start,
        input longint n,
        input mm128_t [MAX_ANCHORS - 1 : 0] a,
        output int [] f,
        output int [] p,
        output int [] t,
        output int [] v
    );


    int max_dist_x = 5000;
    int max_dist_y = 5000;
    int bw = 50;



   initial begin
        fd = $fopen(FILE, "r");
        if(!fd) begin
            $$display("Could not open the sequences\n");
            $finish;
        end

        



   end 



endmodule