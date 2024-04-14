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
    printf("Write Back: Destination=R%d, ALU=%d\n", 
        Reg_MEMtoWB.rd, Reg_MEMtoWB.aluResult);

    // Check if there's a register to update
    if (Reg_MEMtoWB.rd != 0) {  // Assume that rd==0 is not written back (follwing RISC-V conventions)
        if (Reg_MEMtoWB.memRead) {
            // If the instruction was load, write the data into rd
            CURRENT_STATE.REGS[Reg_MEMtoWB.rd] = Reg_MEMtoWB.memData;
            printf("Write Back: Loading %d to R%d\n", Reg_MEMtoWB.memData, Reg_MEMtoWB.rd);
        } 
        else {
            // Else write ALU result into rd
            printf("Write Back: Writing %d to R%d\n", Reg_MEMtoWB.aluResult, Reg_MEMtoWB.rd);
            CURRENT_STATE.REGS[Reg_MEMtoWB.rd] = Reg_MEMtoWB.aluResult;
        }
    }

    // If a branch was taken, update PC
    if (Reg_MEMtoWB.branchTaken) {
        CURRENT_STATE.PC = Reg_MEMtoWB.branchTarget;
        printf("Write Back: New Current state=%d\n", Reg_MEMtoWB.branchTarget);
    }

    memset(&Reg_MEMtoWB, 0, sizeof(Reg_MEMtoWB)); // Clear MEM to WB after use
}

void pipe_stage_mem()
{
    printf("Memory: Memory Destination=R%d\n", Reg_EXtoMEM.rd);

    // Initialize the next pipeline register
    memset(&Reg_MEMtoWB, 0, sizeof(Pipe_Reg_MEMtoWB));

    // Handle memory operations
    if (Reg_EXtoMEM.memRead) {
        // Perform memory read
        Reg_MEMtoWB.memData = mem_read_32(Reg_EXtoMEM.memAddress);
        Reg_MEMtoWB.memRead = true; // Indicate that data was read into this register
        printf("Memory: Reading %d at %d\n", Reg_MEMtoWB.memData, Reg_EXtoMEM.memAddress);
    }
    if (Reg_EXtoMEM.memWrite) {
        // Perform memory write
        printf("Memory: Storing %d at %d\n", Reg_EXtoMEM.storeData, Reg_EXtoMEM.memAddress);
        mem_write_32(Reg_EXtoMEM.memAddress, Reg_EXtoMEM.storeData);
    }

    // Pass ALU result and rd for the Write Back stage
    Reg_MEMtoWB.aluResult = Reg_EXtoMEM.aluResult;
    Reg_MEMtoWB.rd = Reg_EXtoMEM.rd;

    // Pass control signals for branch handling
    Reg_MEMtoWB.branchTaken = Reg_EXtoMEM.branchTaken;
    Reg_MEMtoWB.branchTarget = Reg_EXtoMEM.branchTarget;

    memset(&Reg_EXtoMEM, 0, sizeof(Reg_EXtoMEM)); // Clear EX to MEM after use
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

    // Forwarding logic
    if (Reg_MEMtoWB.rd == rs1 && Reg_MEMtoWB.rd != 0) {
        CURRENT_STATE.REGS[rs1] = Reg_MEMtoWB.aluResult;  // Forward from MEM/WB to EX
        printf("ALUCHECK %d",CURRENT_STATE.REGS[rs1]);
        printf("FORWARDING (r1)\n");
    }
    if (Reg_MEMtoWB.rd == rs2 && Reg_MEMtoWB.rd != 0) {
        CURRENT_STATE.REGS[rs2] = Reg_MEMtoWB.aluResult;  // Forward from MEM/WB to EX
        printf("ALUCHECK %d",CURRENT_STATE.REGS[rs2]);
        printf("FORWARDING (r2)\n");
    }

    printf("Execute: Opcode=0x%X, rd=%d, funct3=0x%X, rs1=%d, rs2=%d, funct7=0x%X, imm=%d\n", 
           opcode, rd, funct3, rs1, rs2, Reg_DEtoEX.funct7, imm);

    // Swtich to determine and execute type of instruction based on opcode
    switch (opcode) {
        case 0x33: // R-type instructions
            // ADD
            Reg_EXtoMEM.aluResult = CURRENT_STATE.REGS[rs1] + CURRENT_STATE.REGS[rs2]; // Adds rs1 and rs2, stores in rd
            printf("ADD: R%d = %d + %d -> %d\n", rd, CURRENT_STATE.REGS[rs1], CURRENT_STATE.REGS[rs2], Reg_EXtoMEM.aluResult);
            break;

        case 0x13: // I-type instructions
            // ADDI
            Reg_EXtoMEM.aluResult = CURRENT_STATE.REGS[rs1] + imm; // Add immediate to rs1 and store in rd
            printf("ADDI: R%d = R%d + %d -> %d\n", rd, rs1, imm, Reg_EXtoMEM.aluResult);
            break;
        
        case 0x3: // I-type instructions
            // LW
            Reg_EXtoMEM.memAddress = CURRENT_STATE.REGS[rs1] + imm; // Calculate load address
            Reg_EXtoMEM.memRead = true;  // Indicate that mem should perform a read operation
            printf("LW: Address=0x%X\n", Reg_EXtoMEM.memAddress);
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
            if (CURRENT_STATE.REGS[rs1] < CURRENT_STATE.REGS[rs2]) {
                Reg_EXtoMEM.branchTarget = CURRENT_STATE.PC + imm;  // Update PC if the branch is taken
                Reg_EXtoMEM.branchTaken = true;
                printf("Executing Branch: rs1=%d, rs2=%d, rs1_val=%d, rs2_val=%d, imm=%d, Target=%x\n",
       rs1, rs2, CURRENT_STATE.REGS[rs1], CURRENT_STATE.REGS[rs2], imm, Reg_EXtoMEM.branchTarget);
                printf("BLT: Branch taken to PC=0x%X\n", Reg_EXtoMEM.branchTarget);
            }
            break;

        case 0x17: // U-type instructions
            // AUIPC
            Reg_EXtoMEM.aluResult = (Reg_DEtoEX.pc + imm) << 12; // Use PC from decode stage
            printf("AUIPC: R%d -> 0x%X\n", rd, Reg_EXtoMEM.aluResult);
            break;
    }

    // Pass rd to mem
    Reg_EXtoMEM.rd = rd;

    memset(&Reg_DEtoEX, 0, sizeof(Reg_DEtoEX)); // Clear DE to EX after use
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
    switch (Reg_DEtoEX.opcode) {
        case 0x13:  // I-type instructions (ADDI)
            imm = (int32_t)instr >> 20; // Sign extend the immediate
            printf("Decode: I-Type instruction with imm=0x%X \n", imm);
            break;
        case 0x3:   // I-type instructions (LW)
            imm = (int32_t)instr >> 20; // Sign extend the immediate
            printf("Decode: I-Type instruction with imm=0x%X \n", imm);
            break;
        case 0x23:  // S-type instructions (SW)
            imm = ((instr >> 7) & 0x1F) | ((instr >> 25) << 5); // Assemble immediate value for store (bits 4:0, 11:5)
            imm = ((int32_t)imm << 20) >> 20; // Sign extend the immediate
            printf("Decode: S-Type instruction with imm=0x%X \n", imm);
            break;
        case 0x63:  // SB-type instructions (BLT)
            imm = ((instr >> 7) & 0x1E) | ((instr >> 25) << 5) | ((instr & 0x80) << 4) | ((instr >> 31) << 11); // Assemble immediate value for branch (bits 11, 4:1, 10:5, 12)
            imm = ((int32_t)imm << 20) >> 20; // Sign extend the immediate
            printf("Decode: SB-Type instruction with imm=0x%X \n", imm);
            break;
        case 0x17:  // U-type instructions (AUIPC)
            imm = (instr & 0xFFFFF000) >> 12; // Get 20-bit upper immediate with leading zeroes
            printf("Decode: U-Type instruction with imm=0x%X \n", imm);
            break;
    }

    // Pass immediate to execute
    Reg_DEtoEX.imm = imm;

    printf("Decode: Opcode=0x%X, rd=%d, funct3=0x%X, rs1=%d, rs2=%d, funct7=0x%X, imm=%d\n", 
           Reg_DEtoEX.opcode, Reg_DEtoEX.rd, Reg_DEtoEX.funct3, Reg_DEtoEX.rs1, Reg_DEtoEX.rs2, Reg_DEtoEX.funct7, imm);

    memset(&Reg_IFtoDE, 0, sizeof(Reg_IFtoDE)); // Clear IF to DE after use
}

void pipe_stage_fetch()
{
  Reg_IFtoDE.instr = mem_read_32(CURRENT_STATE.PC);
  Reg_IFtoDE.pc = CURRENT_STATE.PC;
  printf("Fetch: PC=0x%08x, Instr=0x%08x\n", Reg_IFtoDE.pc, Reg_IFtoDE.instr);
  CURRENT_STATE.PC = CURRENT_STATE.PC+4; // increment PC
}
