`default_nettype none

module system (
    input CLK, SW1,
    output LED1, LED2, LED3, LED4,
    output S1_A, S1_B, S1_C, S1_D, S1_E, S1_F, S1_G,
    output S2_A, S2_B, S2_C, S2_D, S2_E, S2_F, S2_G,
    output SPI_CLK, SPI_CS, SPI_MOSI,
    input SPI_MISO
);

wire [31:0] mem_addr;
wire [31:0] mem_rdata;
wire mem_rstrb;
wire [31:0] mem_wdata;
wire [3:0] mem_wmask;
wire mem_rbusy;

(* init = 0 *) reg [15:0] por_count;
wire por_active = (por_count != {16{1'b1}});

always @(posedge CLK) begin
    if (por_active)
        por_count <= por_count + 1'b1;
end

wire reset = por_active | SW1;

riscv_32i cpu (
  .clk(CLK),
  .reset(reset),
  .mem_addr(mem_addr),
  .mem_rdata(mem_rdata),
  .mem_rbusy(mem_rbusy),
  .mem_rstrb(mem_rstrb),
  .mem_wdata(mem_wdata),
  .mem_wmask(mem_wmask)
);

wire [31:0] ram_rdata;
wire [29:0] mem_word_addr = mem_addr[31:2];
wire is_spi = mem_addr[23];
wire is_io = mem_addr[23:22] == 2'b01;
wire is_ram = mem_addr[23:22] == 2'b00;
wire mem_wstrb = |mem_wmask;

memory ram (
  .clk(CLK),
  .mem_addr(mem_addr),
  .mem_rdata(ram_rdata),
  .mem_rstrb(is_ram & mem_rstrb),
  .mem_wdata(mem_wdata),
  .mem_wmask({4{is_ram}} & mem_wmask)
);

wire [31:0] spi_rdata;
wire spi_rbusy;

spi_flash flash (
    .clk(CLK),
    .rstrb(is_spi & mem_rstrb),
    .word_address(mem_word_addr[14:0]),
    .rdata(spi_rdata),
    .rbusy(spi_rbusy),
    .spi_clk(SPI_CLK),
    .spi_cs_n(SPI_CS),
    .spi_mosi(SPI_MOSI),
    .spi_miso(SPI_MISO)
);

assign mem_rbusy = spi_rbusy;

localparam IO_LEDS_BIT = 0;
localparam IO_SEG_ONE_BIT = 1;
localparam IO_SEG_TWO_BIT = 2;

reg [3:0] leds;
reg [6:0] seg_one;
reg [6:0] seg_two;

always @(posedge CLK) begin
    if (reset) begin
        leds <= {4{1'b0}};
        seg_one <= {7{1'b1}};
        seg_two <= {7{1'b1}};
    end else if (is_io & mem_wstrb) begin
        if (mem_word_addr[IO_LEDS_BIT])
            leds <= mem_wdata[3:0];
        else if (mem_word_addr[IO_SEG_ONE_BIT])
            seg_one <= mem_wdata[6:0];
        else if (mem_word_addr[IO_SEG_TWO_BIT])
            seg_two <= mem_wdata[6:0];
    end
end

assign {LED1, LED2, LED3, LED4} = leds;
assign {S1_A, S1_B, S1_C, S1_D, S1_E, S1_F, S1_G} = seg_one;
assign {S2_A, S2_B, S2_C, S2_D, S2_E, S2_F, S2_G} = seg_two;

wire [31:0] io_rdata = 32'b0;
assign mem_rdata = is_ram ? ram_rdata :
    is_spi ? spi_rdata :
    io_rdata;

endmodule
