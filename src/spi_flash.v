`default_nettype none

module spi_flash (
    input  wire        clk,             // 25 MHz
    input  wire        rstrb,           // Read strobe
    input  wire [14:0] word_address,    // 128KB addressable
    output wire [31:0] rdata,           // data word
    output wire        rbusy,           // busy bit

    output wire        spi_clk,         // SPI_CLK  pin 48
    output reg         spi_cs_n,        // SPI_CS   pin 49
    output wire        spi_mosi,        // SPI_MOSI pin 45
    input  wire        spi_miso         // SPI_MISO pin 46
);

// Send 32 bits (8 cmd + 24 address)
reg [5:0]  snd_bitcount = 6'd0;
reg [31:0] cmd_addr     = 32'h0;

// RX: receive 32 bits of data (1 word)
reg [5:0]  rcv_bitcount = 6'd0;
reg [31:0] rcv_data     = 32'h0;

// Status flags
wire sending   = (snd_bitcount != 0);
wire receiving = (rcv_bitcount != 0);
wire busy      = sending | receiving;

initial spi_cs_n = 1'b1;
assign spi_clk  = (!spi_cs_n) ? clk : 1'b0;
assign rbusy = !spi_cs_n;

// Drive MOSI with the MSB of the cmd/address shifter during TX phase.
assign spi_mosi = cmd_addr[31];

// Re-order bytes correctly
assign rdata = { rcv_data[7:0], rcv_data[15:8], rcv_data[23:16], rcv_data[31:24] };

always @(negedge clk) begin
    if (rstrb) begin
        // 24-bit byte address = { 7'b0, word_address[14:0], 2'b00 }.
        spi_cs_n     <= 1'b0;                                       // assert CS#
        cmd_addr     <= {8'h03, 7'b0000000, word_address, 2'b00};   // READ (0x03) + addr
        snd_bitcount <= 6'd32;                                      // send 32 bits total
    end else begin
        if (sending) begin
            if (snd_bitcount == 6'd1) rcv_bitcount <= 6'd32;   // next: receive 32 bits
            snd_bitcount <= snd_bitcount - 6'd1;
            cmd_addr     <= {cmd_addr[30:0], 1'b0};            // shift left, pad 0
        end

        if (receiving) begin
            rcv_bitcount <= rcv_bitcount - 6'd1;
            rcv_data     <= {rcv_data[30:0], spi_miso};        // shift left, bring in MISO
        end

        if (!busy) begin
            spi_cs_n <= 1'b1; // Done, deassert CS
        end
    end
end

endmodule
