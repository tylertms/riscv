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

`include "riscv_assembly.vh"
integer L0_ = 8;

initial begin
  ADD(x1,x0,x0);
  ADDI(x2,x0,10);
  Label(L0_); 
  ADDI(x1,x1,1); 
  BNE(x1, x2, LabelRef(L0_));
  EBREAK();

  endASM();
end

assign {LED1, LED2, LED3, LED4} = registers[x1][3:0];

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
wire [4:0] rs1_id = instr[19:15];
wire [4:0] rs2_id = instr[24:20];
wire [4:0] rd_id  = instr[11:7];

// Decode instruction funct within instruction types
wire [2:0] funct3 = instr[14:12];
wire [6:0] funct7 = instr[31:25];

// Decode immediates for different instruction types
wire [31:0] imm_u = {instr[31:12], 12'b0};
wire [31:0] imm_i = {{20{instr[31]}}, instr[31:20]};
wire [31:0] imm_s = {{20{instr[31]}}, instr[31:25], instr[11:7]};
wire [31:0] imm_b = {{19{instr[31]}}, instr[31], instr[7], instr[30:25], instr[11:8], 1'b0};
wire [31:0] imm_j = {{11{instr[31]}}, instr[31], instr[19:12], instr[20], instr[30:21], 1'b0};

// ---------------------------------------------

// Register Bank
// ---------------------------------------------
reg [31:0] registers [0:31];
reg [31:0] rs1, rs2;
wire [31:0] write_back_data;
wire write_back_enable;
// ---------------------------------------------

// ALU
// ---------------------------------------------
// funct3 | operation
// -------------------------
// 3'b000 | ADD / SUB
// 3'b001 | lshift (<<)
// 3'b010 | signed comp. (<)
// 3'b011 | usign. comp. (<)
// 3'b100 | XOR (^)
// 3'b101 | rshift (>>)
// 3'b110 | OR (|)
// 3'b111 | AND (&)

wire [31:0] alu_a = rs1;
wire [31:0] alu_b = is_alu_reg ? rs2 : imm_i;
reg [31:0] alu_out;

wire [4:0] shift_amount = is_alu_reg ? rs2[4:0] : instr[24:20];

always @(*) begin
  case (funct3)
    3'b000: alu_out = ((is_alu_reg && funct7[5]) ? (alu_a - alu_b) : (alu_a + alu_b));
    3'b001: alu_out = (alu_a << shift_amount);
    3'b010: alu_out = {31'b0, ($signed(alu_a) < $signed(alu_b))};
    3'b011: alu_out = {31'b0, (alu_a < alu_b)};
    3'b100: alu_out = (alu_a ^ alu_b);
    3'b101: alu_out = (funct7[5] ? ($signed(alu_a) >>> shift_amount) : (alu_a >> shift_amount));
    3'b110: alu_out = (alu_a | alu_b);
    3'b111: alu_out = (alu_a & alu_b);
  endcase
end

reg take_branch;
always @(*) begin
  case (funct3)
    3'b000: take_branch = (rs1 == rs2);
    3'b001: take_branch = (rs1 != rs2);
    3'b100: take_branch = ($signed(rs1) < $signed(rs2));
    3'b101: take_branch = ($signed(rs1) >= $signed(rs2));
    3'b110: take_branch = (rs1 < rs2);
    3'b111: take_branch = (rs1 >= rs2);
    default: take_branch = 1'b0;
  endcase
end

assign write_back_data = (is_jal || is_jalr) ? (pc + 4) : alu_out;
assign write_back_enable = (state == EXECUTE && (is_alu_reg || is_alu_imm || is_jal || is_jalr));
// ---------------------------------------------

// State Machine
// ---------------------------------------------
localparam FETCH_INSTR = 0, FETCH_REGS = 1, EXECUTE = 2;
reg [1:0] state = FETCH_INSTR;

reg div_clk;
clock_div #(.WIDTH(32))
div (.reset(SW1), .clk_in(CLK), .div_num(24999999 >> 4), .clk_out(div_clk));

always @(posedge div_clk or posedge SW1) begin
  if (SW1) begin
    pc <= 0;
    state <= FETCH_INSTR;
  end else begin
    if (write_back_enable && rd_id != 0) begin
      registers[rd_id] <= write_back_data;
    end

    case (state)
      FETCH_INSTR: begin
        instr <= mem[pc[31:2]];
        state <= FETCH_REGS;
      end
      FETCH_REGS: begin
        rs1 <= (rs1_id == 5'd0) ? 32'd0 : registers[rs1_id];
        rs2 <= (rs2_id == 5'd0) ? 32'd0 : registers[rs2_id];
        state <= EXECUTE;
      end
      EXECUTE: begin
        if (!is_system)
          pc <= next_pc;
        state <= FETCH_INSTR;
      end
    endcase
    end
end

wire [31:0] next_pc = 
  (is_branch && take_branch) ? (pc + imm_b) :
  (is_jal) ? (pc + imm_j) : 
  (is_jalr) ? (rs1 + imm_i) : 
  (pc + 4);
// ---------------------------------------------

endmodule
