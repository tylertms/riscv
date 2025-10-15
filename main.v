`default_nettype none

module top (
  input CLK, SW1,
  output LED1, LED2, LED3, LED4
);

// Instruction Decoder
// ---------------------------------------------
// Defined on page 130 of:
// https://github.com/riscv/riscv-isa-manual/releases/download/Ratified-IMAFDQC/riscv-spec-20191213.pdf

reg [31:0] mem [0:255];
reg [31:0] pc;
reg [31:0] instr;

// Decode instruction type
wire is_alu_reg = (instr[6:0] == 7'b0110011); // reg <= reg op reg
wire is_alu_imm = (instr[6:0] == 7'b0010011); // reg <= reg op imm
wire is_branch  = (instr[6:0] == 7'b1100011); // if (reg op reg) pc <= pc + imm
wire is_jalr    = (instr[6:0] == 7'b1100111); // reg <= pc + 4 ; pc <= reg + imm
wire is_jal     = (instr[6:0] == 7'b1101111); // reg <= pc + 4 ; pc <= pc + imm
wire is_auipc   = (instr[6:0] == 7'b0010111); // reg <= pc + (imm << 12)
wire is_lui     = (instr[6:0] == 7'b0110111); // reg <= (imm << 12) 
wire is_load    = (instr[6:0] == 7'b0000011); // reg <= mem[reg + imm]
wire is_store   = (instr[6:0] == 7'b0100011); // mem[reg + imm] <= reg
wire is_system  = (instr[6:0] == 7'b1110011); // ...

// Decode source/destination registers
wire [3:0] rs1_id = instr[18:15];
wire [3:0] rs2_id = instr[23:20];
wire [3:0] rd_id  = instr[10:7];

// Decode instruction funct within instruction types
wire [2:0] funct3 = instr[14:12];
wire [6:0] funct7 = instr[31:25];

// Decode immediates for different instruction types
wire [31:0] imm_i = {{21{instr[31]}}, instr[30:20]};
wire [31:0] imm_s = {{21{instr[31]}}, instr[30:25], instr[11:7]};
wire [31:0] imm_b = {{20{instr[31]}}, instr[7], instr[30:25], instr[11:8], 1'b0};
wire [31:0] imm_u = {instr[31], instr[30:12], {12{1'b0}}};
wire [31:0] imm_j = {{12{instr[31]}}, instr[19:12], instr[20], instr[30:21], 1'b0};
// ---------------------------------------------

// Register Bank
// ---------------------------------------------
reg [31:0] registers [0:15];
reg [31:0] rs1, rs2;
wire [31:0] write_back_data = 32'b0;
wire write_back_enable = 0;
// ---------------------------------------------

// State Machine
// ---------------------------------------------
localparam FETCH_INSTR = 0, FETCH_REGS = 1, EXECUTE = 2;
reg [1:0] state = FETCH_INSTR;

always @(posedge CLK) begin
  if (write_back_enable && rd_id != 0)
    registers[rd_id] <= write_back_data;

  case(state)
    FETCH_INSTR: begin
      instr <= mem[pc];
      state <= FETCH_REGS;
    end
    FETCH_REGS: begin
      rs1 <= registers[rs1_id];
      rs2 <= registers[rs2_id];
      state <= EXECUTE;
    end
    EXECUTE: begin
      pc <= pc + 1;
      state <= FETCH_INSTR;
    end
  endcase
end
// ---------------------------------------------

endmodule
