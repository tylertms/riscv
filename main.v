`default_nettype none

module system (
  input CLK, SW1,
  output LED1, LED2, LED3, LED4
);

reg [31:0] pc;
reg [31:0] mem [0:255];

wire [31:0] mem_addr;
reg [31:0] mem_rdata;
wire mem_rstrb;

always @(posedge CLK) begin
  if(mem_rstrb) begin
      mem_rdata <= mem[mem_addr[31:2]];
  end
end

// assign {LED1, LED2, LED3, LED4} = registers[x10][3:0];

// Instruction Decoder
// ---------------------------------------------
// Defined on page 130 of:
// https://github.com/riscv/riscv-isa-manual/releases/download/Ratified-IMAFDQC/riscv-spec-20191213.pdf

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
wire [31:0] alu_b = (is_alu_reg | is_branch) ? rs2 : imm_i;

wire [4:0] shift_amount = is_alu_reg ? rs2[4:0] : instr[24:20];

wire [31:0] alu_plus = alu_a + alu_b;
wire [32:0] alu_minus = {1'b0, alu_a} - {1'b0, alu_b};

wire equal = (alu_minus[31:0] == 0);
wire less_than = (alu_a[31] ^ alu_b[31]) ? alu_a[31] : alu_minus[32];
wire less_than_unsigned = alu_minus[32];

function [31:0] flip32;
  input [31:0] x;
  flip32 = {x[00], x[01], x[02], x[03], x[04], x[05], x[06], x[07], 
            x[08], x[09], x[10], x[11], x[12], x[13], x[14], x[15], 
            x[16], x[17], x[18], x[19], x[20], x[21], x[22], x[23],
            x[24], x[25], x[26], x[27], x[28], x[29], x[30], x[31]};
endfunction

wire [31:0] shifter_in = (funct3 == 3'b001) ? flip32(alu_a) : alu_a;
wire [31:0] shifter = $signed({instr[30] & alu_a[31], shifter_in}) >>> alu_b[4:0];
wire [31:0] leftshift = flip32(shifter);

reg [31:0] alu_out;
always @(*) begin
  case(funct3)
    3'b000: alu_out = (funct7[5] & instr[5]) ? alu_minus[31:0] : alu_plus;
    3'b001: alu_out = leftshift;
    3'b010: alu_out = {31'b0, less_than};
    3'b011: alu_out = {31'b0, less_than_unsigned};
    3'b100: alu_out = (alu_a ^ alu_b);
    3'b101: alu_out = shifter; 
    3'b110: alu_out = (alu_a | alu_b);
    3'b111: alu_out = (alu_a & alu_b);	
  endcase
end
// ---------------------------------------------


// Branch
// ---------------------------------------------
reg take_branch;
always @(*) begin
  case (funct3)
    3'b000: take_branch = equal;
    3'b001: take_branch = !equal;
    3'b100: take_branch = less_than;
    3'b101: take_branch = !less_than;
    3'b110: take_branch = less_than_unsigned;
    3'b111: take_branch = !less_than_unsigned;
    default: take_branch = 1'b0;
  endcase
end
// ---------------------------------------------

// PC / Next State
// ---------------------------------------------
wire [31:0] pc_plus_imm = pc + (instr[3] ? imm_j[31:0] : instr[4] ? imm_u[31:0] : imm_b[31:0]);
wire [31:0] pc_plus_4 = pc + 4;

assign write_back_data = (is_jal || is_jalr) ? pc_plus_4 :
  is_lui   ? imm_u :
  is_auipc ? pc_plus_imm :
  is_load  ? load_data :
  alu_out;

assign write_back_enable = ((state == EXECUTE) && !is_branch && !is_store && !is_load) || (state == WAIT_DATA);

wire [31:0] next_pc = ((is_branch && take_branch) || is_jal) ? pc_plus_imm :
  is_jalr ? {alu_plus[31:1], 1'b0} :
  pc_plus_4;
// ---------------------------------------------


// Load / Store
// ---------------------------------------------
wire [31:0] load_store_addr = rs1 + (is_store ? imm_s : imm_i);
wire [15:0] load_half_word = load_store_addr[1] ? mem_rdata[31:16] : mem_rdata[15:0];
wire [7:0] load_byte = load_store_addr[0] ? load_half_word[15:8] : load_half_word[7:0];

wire load_sign = !funct3[2] & (mem_byte_access ? load_byte[7] : load_half_word[15]);
wire mem_byte_access = (funct3[1:0] == 2'b00);
wire mem_half_word_access = (funct3[1:0] == 2'b01);

wire [31:0] load_data = 
  mem_byte_access ? {{24{load_sign}}, load_byte} :
  mem_half_word_access ? {{16{load_sign}}, load_half_word} :
  mem_rdata;
// ---------------------------------------------


// Processor State
// ---------------------------------------------
localparam FETCH_INSTR = 0;
localparam WAIT_INSTR  = 1;
localparam FETCH_REGS  = 2;
localparam EXECUTE     = 3;
localparam LOAD        = 4;
localparam WAIT_DATA   = 5;
reg [2:0] state = FETCH_INSTR;

always @(posedge CLK or posedge SW1) begin
  if (SW1) begin
    pc <= 0;
    state <= FETCH_INSTR;
  end 
  
  else begin
    if (write_back_enable && rd_id != 0) begin
      registers[rd_id] <= write_back_data;
    end

    case (state)
      FETCH_INSTR: begin
        state <= WAIT_INSTR;
      end
      WAIT_INSTR: begin
        instr <= mem_rdata;
        state <= FETCH_REGS;
      end
      FETCH_REGS: begin
        rs1 <= (rs1_id == 5'd0) ? 32'b0 : registers[rs1_id];
        rs2 <= (rs2_id == 5'd0) ? 32'b0 : registers[rs2_id];
        state <= EXECUTE;
      end
      EXECUTE: begin
        if (!is_system) pc <= next_pc;
        state <= is_load ? LOAD : FETCH_INSTR;
      end
      LOAD: begin
        state <= WAIT_DATA;
      end
      WAIT_DATA: begin
        state <= FETCH_INSTR;
      end
    endcase
  end
end

assign mem_addr = (state == WAIT_INSTR || state == FETCH_INSTR) ? pc : load_store_addr;
assign mem_rstrb = (state == FETCH_INSTR || state == LOAD);
// ---------------------------------------------


endmodule
