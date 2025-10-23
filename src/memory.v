`default_nettype none

module memory (
  input clk,
  input mem_rstrb,
  input [31:0] mem_addr,
  input [31:0] mem_wdata,
  input [3:0] mem_wmask,
  output reg [31:0] mem_rdata
);

reg [31:0] mem [1536];

wire [10:0] word_addr = mem_addr[12:2];
always @(posedge clk) begin
  if(mem_rstrb) begin
      mem_rdata <= mem[word_addr];
  end

  if(mem_wmask[0]) mem[word_addr][7:0] <= mem_wdata[7:0];
  if(mem_wmask[1]) mem[word_addr][15:8] <= mem_wdata[15:8];
  if(mem_wmask[2]) mem[word_addr][23:16] <= mem_wdata[23:16];
  if(mem_wmask[3]) mem[word_addr][31:24] <= mem_wdata[31:24];
end

endmodule
