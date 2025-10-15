module decoder (
    input reg [31:0] instruction
);

wire is_alu_reg = (instruction[6:0] == 7'b0110011); // reg <= reg op reg
wire is_alu_imm = (instruction[6:0] == 7'b0010011); // reg <= reg op imm
wire is_branch  = (instruction[6:0] == 7'b1100011); // if (reg op reg) pc <= pc + imm
wire is_jalr    = (instruction[6:0] == 7'b1100111); // reg <= pc + 4 ; pc <= reg + imm
wire is_jal     = (instruction[6:0] == 7'b1101111); // reg <= pc + 4 ; pc <= pc + imm
wire is_auipc   = (instruction[6:0] == 7'b0010111); // reg <= pc + (imm << 12)
wire is_lui     = (instruction[6:0] == 7'b0110111); // reg <= (imm << 12) 
wire is_load    = (instruction[6:0] == 7'b0000011); // reg <= mem[reg + imm]
wire is_store   = (instruction[6:0] == 7'b0100011); // mem[reg + imm] <= reg
wire is_system  = (instruction[6:0] == 7'b1110011); // ...

endmodule