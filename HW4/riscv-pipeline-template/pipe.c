// RISCV pipeline timing simulator

#include "pipe.h"
#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* global pipeline state */
//CPU_State CURRENT_STATE;

Pipe_Reg_IFtoDE Reg_IFtoDE;
Pipe_Reg_DEtoEX Reg_DEtoEX;
Pipe_Reg_EXtoMEM Reg_EXtoMEM;
Pipe_Reg_MEMtoWB Reg_MEMtoWB;

// Global variables
uint32_t instr;             // Current instruction being processed
uint32_t opcode;            // Opcode for instruction
uint32_t rd, rs1, rs2;      // Destination and source registers
uint32_t funct3, funct7;    // Function fields
int32_t imm;                // Immediate value

void pipe_init()
{
  memset(&CURRENT_STATE, 0, sizeof(CPU_State));
  CURRENT_STATE.PC = 0x00000000;
}

void pipe_cycle()
{
  pipe_stage_wb();
  pipe_stage_mem();
  pipe_stage_execute();
  pipe_stage_decode();
  pipe_stage_fetch();
}

void pipe_stage_wb()
{
    printf("Write Back: Opcode=0x%X, rd=%d, funct3=0x%X, rs1=%d, rs2=%d, funct7=0x%X, imm=%d\n", 
           opcode, Reg_MEMtoWB.rd, funct3, rs1, rs2, funct7, imm);

    // Check if there's a register to update
    if (Reg_MEMtoWB.rd != 0) {  // Assume that rd==0 is not written back (follwing RISC-V conventions)
        if (Reg_MEMtoWB.memRead) {
            // If the instruction was load, write the data into rd
            CURRENT_STATE.REGS[Reg_MEMtoWB.rd] = Reg_MEMtoWB.memData;
        } 
        else {
            // Else write ALU result into rd
            CURRENT_STATE.REGS[Reg_MEMtoWB.rd] = Reg_MEMtoWB.aluResult;
        }
    }

    // If a branch was taken, update PC
    if (Reg_MEMtoWB.branchTaken) {
        CURRENT_STATE.PC = Reg_MEMtoWB.branchTarget;
    }

    memset(&Reg_MEMtoWB, 0, sizeof(Reg_MEMtoWB)); // Clear MEM to WB register after use
}

void pipe_stage_mem()
{
    printf("Memory: Opcode=0x%X, rd=%d, funct3=0x%X, rs1=%d, rs2=%d, funct7=0x%X, imm=%d\n", 
           opcode, Reg_EXtoMEM.rd, funct3, rs1, rs2, funct7, imm);

    // Initialize the next pipeline register
    memset(&Reg_MEMtoWB, 0, sizeof(Pipe_Reg_MEMtoWB));

    // Handle memory operations
    if (Reg_EXtoMEM.memRead) {
        // Perform memory read
        Reg_MEMtoWB.memData = mem_read_32(Reg_EXtoMEM.memAddress);
        Reg_MEMtoWB.memRead = true; // Indicate that data was read into this register
    }
    if (Reg_EXtoMEM.memWrite) {
        // Perform memory write
        mem_write_32(Reg_EXtoMEM.memAddress, Reg_EXtoMEM.storeData);
    }

    // Pass ALU result and rd for the Write Back stage
    Reg_MEMtoWB.aluResult = Reg_EXtoMEM.aluResult;
    Reg_MEMtoWB.rd = Reg_EXtoMEM.rd;

    // Pass control signals for branch handling
    Reg_MEMtoWB.branchTaken = Reg_EXtoMEM.branchTaken;
    Reg_MEMtoWB.branchTarget = Reg_EXtoMEM.branchTarget;
}

void pipe_stage_execute()
{
    // Pull data from decode
    opcode = Reg_DEtoEX.opcode;
    rd = Reg_DEtoEX.rd;
    rs1 = Reg_DEtoEX.rs1;
    rs2 = Reg_DEtoEX.rs2;
    funct3 = Reg_DEtoEX.funct3;
    imm = Reg_DEtoEX.imm;

    printf("Execute: Opcode=0x%X, rd=%d, funct3=0x%X, rs1=%d, rs2=%d, funct7=0x%X, imm=%d\n", 
           opcode, rd, funct3, rs1, rs2, Reg_DEtoEX.funct7, imm);

    // Swtich to determine and execute type of instruction based on opcode
    switch (opcode) {
        case 0x33: // R-type instructions
            // ADD
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] + CURRENT_STATE.REGS[rs2]; // Adds rs1 and rs2, stores in rd
            printf("ADD: R%d = R%d + R%d -> %d\n", rd, rs1, rs2, NEXT_STATE.REGS[rd]);
            break;

        case 0x13: // I-type instructions
            if (funct3 == 0x0) {
                // ADDI
                NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs1] + imm; // Add immediate to rs1 and store in rd
                printf("ADDI: R%d = R%d + %d -> %d\n", rd, rs1, imm, NEXT_STATE.REGS[rd]);
            } else if (funct3 == 0x02) {
                // LW
                Reg_EXtoMEM.memAddress = CURRENT_STATE.REGS[rs1] + imm; // Calculate load address
                Reg_EXtoMEM.memRead = true;  // Indicate that mem should perform a read operation
                printf("LW: Address=0x%X\n", Reg_EXtoMEM.memAddress);
            }
            break;

        case 0x23: // S-type instructions
            // SW
            Reg_EXtoMEM.memAddress = CURRENT_STATE.REGS[rs1] + imm;
            Reg_EXtoMEM.storeData = CURRENT_STATE.REGS[rs2];
            Reg_EXtoMEM.memWrite = true;  // Indicate that mem should perform a write operation
            printf("SW: Mem[0x%X] = R%d -> %d\n", Reg_EXtoMEM.memAddress, rs2, CURRENT_STATE.REGS[rs2]);
            break;

        case 0x63: // SB-type instructions
            // BLT
            if (funct3 == 0x4) {  // 'blt' uses funct3 == 0x4
                if (CURRENT_STATE.REGS[rs1] < CURRENT_STATE.REGS[rs2]) {
                    NEXT_STATE.PC = CURRENT_STATE.PC + imm;  // Update PC if the branch is taken
                    printf("BLT: Branch taken to PC=0x%X\n", NEXT_STATE.PC);
                }
            }
            break;

        case 0x17: // U-type instructions
            // AUIPC
            NEXT_STATE.REGS[rd] = (CURRENT_STATE.PC + imm) << 12; // Add immediate to current state and shift left by 12 bits
            printf("AUIPC: R%d -> 0x%X\n", rd, NEXT_STATE.REGS[rd]);
            break;
    }

    // Pass rd to mem
    Reg_EXtoMEM.rd = rd;
}

void pipe_stage_decode()
{
    uint32_t instr = Reg_IFtoDE.instr;
    printf("Decode: Instruction=0x%X \n", instr);

    // Decodes fields based on order: opcode, destination register, funct3, rs1, rs2, and funct7
    Reg_DEtoEX.opcode = instr & 0x7F;          // Get opcode
    Reg_DEtoEX.rd = (instr >> 7) & 0x1F;       // Get destination register
    Reg_DEtoEX.funct3 = (instr >> 12) & 0x7;   // Get funct3 field
    Reg_DEtoEX.rs1 = (instr >> 15) & 0x1F;     // Get first source register
    Reg_DEtoEX.rs2 = (instr >> 20) & 0x1F;     // Get second source register
    Reg_DEtoEX.funct7 = (instr >> 25) & 0x7F;  // Get funct7 (for R-type instructions)

    // Switch to decode immediate value based on opcode
    switch (opcode) {
        case 0x13:  // I-type instructions
            Reg_DEtoEX.imm = (int32_t)instr >> 20; // Sign extend the immediate
            printf("Decode: I-Type instruction with imm=0x%X \n", Reg_DEtoEX.imm);
            break;
        case 0x23:  // S-type instructions
            Reg_DEtoEX.imm = ((instr >> 7) & 0x1F) | ((instr >> 25) << 5); // Assemble immediate value for store (bits 4:0, 11:5)
            Reg_DEtoEX.imm = ((int32_t)Reg_DEtoEX.imm << 20) >> 20; // Sign extend the immediate
            printf("Decode: S-Type instruction with imm=0x%X \n", Reg_DEtoEX.imm);
            break;
        case 0x63:  // SB-type instructions
            Reg_DEtoEX.imm = ((instr >> 7) & 0x1E) | ((instr >> 25) << 5) | ((instr & 0x80) << 4) | ((instr >> 31) << 11); // Assemble immediate value for branch (bits 11, 4:1, 10:5, 12)
            Reg_DEtoEX.imm = ((int32_t)Reg_DEtoEX.imm << 20) >> 20; // Sign extend the immediate
            printf("Decode: SB-Type instruction with imm=0x%X \n", Reg_DEtoEX.imm);
            break;
        case 0x17:  // U-type instructions
            Reg_DEtoEX.imm = (instr & 0xFFFFF000) >> 12; // Get 20-bit upper immediate with leading zeroes
            printf("Decode: U-Type instruction with imm=0x%X \n", Reg_DEtoEX.imm);
            break;
    }

    printf("Decode: Opcode=0x%X, rd=%d, funct3=0x%X, rs1=%d, rs2=%d, funct7=0x%X, imm=%d\n", 
           Reg_DEtoEX.opcode, Reg_DEtoEX.rd, Reg_DEtoEX.funct3, Reg_DEtoEX.rs1, Reg_DEtoEX.rs2, Reg_DEtoEX.funct7, Reg_DEtoEX.imm);
}

void pipe_stage_fetch()
{
  Reg_IFtoDE.instr = mem_read_32(CURRENT_STATE.PC);
  printf("Fetch: PC=0x%08x, Instr=0x%08x\n", CURRENT_STATE.PC, Reg_IFtoDE.instr);
  CURRENT_STATE.PC = CURRENT_STATE.PC+4; // increment PC
}
