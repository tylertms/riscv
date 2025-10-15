module decoder (
    input reg [31:0] instr
);

// Defined on page 130 of:
// https://github.com/riscv/riscv-isa-manual/releases/download/Ratified-IMAFDQC/riscv-spec-20191213.pdf

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

wire [4:0] rs1_id = instr[19:15]; // Source Register 1 for R/I/S/B type instructions
wire [4:0] rs2_id = instr[24:20]; // Source Register 2 for R/S/B type instructions
wire [4:0] rd_id  = instr[11:7]; // Destination Register for R/I/U/J type instructions

wire [3:0] funct3 = instr[14:12];
wire [6:0] funct7 = instr[31:25];

wire [31:0] imm_i = {{21{instr[31]}}, instr[30:20]};
wire [31:0] imm_s = {{21{instr[31]}}, instr[30:25], instr[11:7]};
wire [31:0] imm_b = {{20{instr[31]}}, instr[7], instr[30:25], instr[11:8], 1'b0};
wire [31:0] imm_u = {instr[31], instr[30:12], {12{1'b0}}};
wire [31:0] imm_j = {{12{instr[31]}}, instr[19:12], instr[20], instr[30:21], 1'b0};

endmodule