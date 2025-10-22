`default_nettype none

module riscv_32i (
  input clk, reset,
  input [31:0] mem_rdata,
  input mem_rbusy,
  output [31:0] mem_addr,
  output mem_rstrb,
  output [31:0] mem_wdata,
  output [3:0] mem_wmask
);

localparam RESET_ADDR = 32'h00000000;
localparam ADDR_WIDTH = 24;
localparam ADDR_PAD = 32 - ADDR_WIDTH;

wire is_alu_reg = (instr[6:2] == 5'b01100); // reg <= reg op reg
wire is_alu_imm = (instr[6:2] == 5'b00100); // reg <= reg op imm
wire is_branch  = (instr[6:2] == 5'b11000); // if (reg op reg) pc <= pc + imm
wire is_jalr    = (instr[6:2] == 5'b11001); // reg <= pc + 4 ; pc <= reg + imm
wire is_jal     = (instr[6:2] == 5'b11011); // reg <= pc + 4 ; pc <= pc + imm
wire is_auipc   = (instr[6:2] == 5'b00101); // reg <= pc + (imm << 12)
wire is_lui     = (instr[6:2] == 5'b01101); // reg <= (imm << 12)
wire is_load    = (instr[6:2] == 5'b00000); // reg <= mem[reg + imm]
wire is_store   = (instr[6:2] == 5'b01000); // mem[reg + imm] <= reg
wire is_system  = (instr[6:2] == 5'b11100); // ...
wire is_alu = is_alu_reg | is_alu_imm;

wire [4:0] rs1_id = instr[19:15];
wire [4:0] rs2_id = instr[24:20];
wire [4:0] rd_id  = instr[11:7];

(* onehot *)
wire [7:0] funct3_is = 8'h01 << instr[14:12];

wire [31:0] imm_u = {instr[31], instr[30:12], {12{1'b0}}};
wire [31:0] imm_i = {{21{instr[31]}}, instr[30:20]};
wire [31:0] imm_s = {{21{instr[31]}}, instr[30:25], instr[11:7]};
wire [31:0] imm_b = {{20{instr[31]}}, instr[7], instr[30:25], instr[11:8], 1'b0};
wire [31:0] imm_j = {{12{instr[31]}}, instr[19:12], instr[20], instr[30:21], 1'b0};

(* no_rw_check *)
reg [31:0] registers [32];
reg [31:0] rs1, rs2;


always @(posedge clk) begin
    if (write_back && rd_id != 0) begin
        registers[rd_id] <= write_back_data;
    end
end

wire [31:0] alu_a = rs1;
wire [31:0] alu_b = (is_alu_reg | is_branch) ? rs2 : imm_i;

wire [31:0] alu_plus = alu_a + alu_b;
wire [32:0] alu_minus = {1'b0, alu_a} - {1'b0, alu_b};

wire EQ = (alu_minus[31:0] == 0);
wire LT = (alu_a[31] ^ alu_b[31]) ? alu_a[31] : alu_minus[32];
wire LTU = alu_minus[32];

wire [31:0] shifter_in = funct3_is[1] ? {
    alu_a[00], alu_a[01], alu_a[02], alu_a[03],
    alu_a[04], alu_a[05], alu_a[06], alu_a[07],
    alu_a[08], alu_a[09], alu_a[10], alu_a[11],
    alu_a[12], alu_a[13], alu_a[14], alu_a[15],
    alu_a[16], alu_a[17], alu_a[18], alu_a[19],
    alu_a[20], alu_a[21], alu_a[22], alu_a[23],
    alu_a[24], alu_a[25], alu_a[26], alu_a[27],
    alu_a[28], alu_a[29], alu_a[30], alu_a[31]
} : alu_a;

wire signed [32:0] shifter_wide =
    $signed({instr[30] & alu_a[31], shifter_in}) >>> alu_b[4:0];

wire [31:0] shifter = shifter_wide[31:0];

wire [31:0] left_shift = {
    shifter[00], shifter[01], shifter[02], shifter[03],
    shifter[04], shifter[05], shifter[06], shifter[07],
    shifter[08], shifter[09], shifter[10], shifter[11],
    shifter[12], shifter[13], shifter[14], shifter[15],
    shifter[16], shifter[17], shifter[18], shifter[19],
    shifter[20], shifter[21], shifter[22], shifter[23],
    shifter[24], shifter[25], shifter[26], shifter[27],
    shifter[28], shifter[29], shifter[30], shifter[31]
};

wire [31:0] alu_out = (
    (funct3_is[0] ? (instr[30] & instr[5] ? alu_minus[31:0] : alu_plus) : 32'b0) |
    (funct3_is[1] ? left_shift                                          : 32'b0) |
    (funct3_is[2] ? {31'b0, LT}                                         : 32'b0) |
    (funct3_is[3] ? {31'b0, LTU}                                        : 32'b0) |
    (funct3_is[4] ? alu_a ^ alu_b                                       : 32'b0) |
    (funct3_is[5] ? shifter                                             : 32'b0) |
    (funct3_is[6] ? alu_a | alu_b                                       : 32'b0) |
    (funct3_is[7] ? alu_a & alu_b                                       : 32'b0)
);

wire predicate = (
    funct3_is[0] &   EQ |
    funct3_is[1] &  !EQ |
    funct3_is[4] &   LT |
    funct3_is[5] &  !LT |
    funct3_is[6] &  LTU |
    funct3_is[7] & !LTU
);

reg [ADDR_WIDTH-1:0] pc;
reg [31:2] instr;

wire [ADDR_WIDTH-1:0] pc_plus_4 = pc + 4;

wire [ADDR_WIDTH-1:0] pc_plus_imm = pc + (
    instr[3] ? imm_j[ADDR_WIDTH-1:0] :
    instr[4] ? imm_u[ADDR_WIDTH-1:0] :
    imm_b[ADDR_WIDTH-1:0]
);

wire [ADDR_WIDTH-1:0] load_store_addr = rs1[ADDR_WIDTH-1:0] + (
    instr[5] ? imm_s[ADDR_WIDTH-1:0] : imm_i[ADDR_WIDTH-1:0]
);

wire [ADDR_WIDTH-1:0] next_pc = (
    is_jalr ? {alu_plus[ADDR_WIDTH-1:1], 1'b0} :
    (is_jal | (is_branch & predicate)) ? pc_plus_imm :
    pc_plus_4
);

assign mem_addr = (state[WAIT_INSTR_BIT] | state[FETCH_INSTR_BIT]) ? {{ADDR_PAD{1'b0}}, pc} : (
    state[EXECUTE_BIT] & ~is_load & ~is_store ?
        {{ADDR_PAD{1'b0}}, next_pc} :
        {{ADDR_PAD{1'b0}}, load_store_addr}
);

wire [31:0] write_back_data = (
    (is_lui           ? imm_u       : 32'b0) |
    (is_alu           ? alu_out     : 32'b0) |
    (is_auipc         ? {{ADDR_PAD{1'b0}}, pc_plus_imm} : 32'b0) |
    (is_jalr | is_jal ? {{ADDR_PAD{1'b0}}, pc_plus_4}   : 32'b0) |
    (is_load          ? load_data   : 32'b0)
);

wire mem_byte_access = (instr[13:12] == 2'b00);
wire mem_half_word_access = (instr[13:12] == 2'b01);

wire load_sign = !instr[14] & (mem_byte_access ? load_byte[7] : load_half_word[15]);

wire [31:0] load_data = (
  mem_byte_access ? {{24{load_sign}}, load_byte} :
  mem_half_word_access ? {{16{load_sign}}, load_half_word} :
  mem_rdata
);

wire [15:0] load_half_word = load_store_addr[1] ? mem_rdata[31:16] : mem_rdata[15:0];
wire [7:0] load_byte = load_store_addr[0] ? load_half_word[15:8] : load_half_word[7:0];

assign mem_wdata[7:0] = rs2[7:0];
assign mem_wdata[15:8] = load_store_addr[0] ? rs2[7:0] : rs2[15:8];
assign mem_wdata[23:16] = load_store_addr[1] ? rs2[7:0] : rs2[23:16];
assign mem_wdata[31:24] = load_store_addr[0] ? rs2[7:0] : (
    load_store_addr[1] ? rs2[15:8] : rs2[31:24]
);

wire [3:0] store_wmask = mem_byte_access ? (
    load_store_addr[1] ?
        (load_store_addr[0] ? 4'b1000 : 4'b0100) :
        (load_store_addr[0] ? 4'b0010 : 4'b0001)
) : (
    mem_half_word_access ?
        (load_store_addr[1] ? 4'b1100 : 4'b0011) :
        4'b1111
);

localparam FETCH_INSTR_BIT     = 0;
localparam WAIT_INSTR_BIT      = 1;
localparam EXECUTE_BIT         = 2;
localparam WAIT_ALU_OR_MEM_BIT = 3;
localparam NB_STATES           = 4;

localparam FETCH_INSTR     = 1 << FETCH_INSTR_BIT;
localparam WAIT_INSTR      = 1 << WAIT_INSTR_BIT;
localparam EXECUTE         = 1 << EXECUTE_BIT;
localparam WAIT_ALU_OR_MEM = 1 << WAIT_ALU_OR_MEM_BIT;

(* onehot *)
reg [NB_STATES-1:0] state;

wire write_back = ~(is_branch | is_store) &
    (state[EXECUTE_BIT] | state[WAIT_ALU_OR_MEM_BIT]);

assign mem_rstrb = state[EXECUTE_BIT] & ~is_store | state[FETCH_INSTR_BIT];
assign mem_wmask = {4{state[EXECUTE_BIT] & is_store}} & store_wmask;

wire need_to_wait = is_load | is_store;

always @(posedge clk) begin
  if (reset) begin
      state <= WAIT_ALU_OR_MEM;
      pc <= RESET_ADDR[ADDR_WIDTH-1:0];
  end else

  (* parallel_case *)
  case (1'b1)
    state[WAIT_INSTR_BIT]: begin
        if (!mem_rbusy) begin
            rs1 <= (mem_rdata[19:15] == 0) ? 32'b0 : registers[mem_rdata[19:15]];
            rs2 <= (mem_rdata[24:20] == 0) ? 32'b0 : registers[mem_rdata[24:20]];
            instr <= mem_rdata[31:2];
            state <= EXECUTE;
        end
    end

    state[EXECUTE_BIT]: begin
        if (!is_system) pc <= next_pc;
        state <= need_to_wait ? WAIT_ALU_OR_MEM : WAIT_INSTR;
    end

    state[WAIT_ALU_OR_MEM_BIT]: begin
        if (!mem_rbusy) state <= FETCH_INSTR;
    end

    default: begin
        state <= WAIT_INSTR;
    end

  endcase
end
endmodule
