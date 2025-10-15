module clock_div #(
    parameter WIDTH = 8
)(
    input wire reset,
    input wire clk_in,
    input wire [WIDTH-1:0] div_num,
    output wire clk_out
);

reg A, B_pos, B_neg;
reg [WIDTH-1:0] counter;
wire [WIDTH-1:0] next_counter = (counter == div_num) ? 0 : counter + 1;

always @(posedge clk_in or posedge reset) begin
    if (reset) begin
        counter <= 0;
        A <= 0; B_pos <= 0;
    end else begin
        if (next_counter == div_num) 
            A <= ~A;

        if (div_num[0] && next_counter == (div_num >> 1))
            B_pos <= ~B_pos;

        counter <= next_counter;
    end
end

always @(negedge clk_in or posedge reset) begin
    if (reset)
        B_neg <= 0;
        
    else if (!div_num[0] && next_counter == (div_num >> 1))
        B_neg <= ~B_neg;
end

assign clk_out = A ^ B_pos ^ B_neg;

endmodule
