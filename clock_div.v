module clock_div #(
    parameter WIDTH = 8
)(
    input wire reset,
    input wire clk_in,
    input wire [WIDTH-1:0] div_num,
    output wire clk_out
);

// A and B toggle on rising and falling edges, respectively, when appropriate
reg A, B;

// The counter state is used to time the toggling of A and B at the beginning
// and middle of the didived output clock, respectively.
reg [WIDTH-1:0] counter;

// If the divisor is even, only A can be used to toggle the output clock
// as all toggles happen on rising edges. Thus, the counter maximum value
// is half of what it would be otherwise.
wire [WIDTH-1:0] reset_num = div_num >> div_num[0];

// The next counter state either increments or resets back to 0 at the reset_num
wire [WIDTH-1:0] next_counter = (counter == reset_num) ? 0 : counter + 1;

always @(posedge clk_in or posedge reset) begin
    // A/Counter reset logic
    if (reset) begin
        A <= 0; counter <= 0;
    end

    else begin
        // If the counter is at the maximum, toggle A
        // This occurs at the start/end of a new divided clock
        if (counter == reset_num)
            A <= ~A;

        // Assign the next state
        counter <= next_counter;
    end
end

always @(negedge clk_in or posedge reset) begin
    // B reset logic
    if (reset)
        B <= 0;

    // If the divisor is odd (B is XOR'ed with A), then toggle
    // B on the falling edge halfway through the divided clock cycle
    else if (!div_num[0] && counter == (div_num >> 1))
        B <= ~B;
end

// By XORing A and B, we can either produce an output using only A
// with even divisors and rising edges, or toggle the output clock
// on a falling edge by toggling B
assign clk_out = A ^ B;

endmodule
