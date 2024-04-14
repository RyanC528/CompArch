/* Ryan Copeland | Gabriel Gonzalez | EECE 4821 | HW4 | pipe.c | 4/14/2024
  REPO: https://github.com/RyanC528/CompArch/tree/main/HW4
*/

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
int stall = 0;              // Global flag for stalling logic

void pipe_init()
{
  memset(&CURRENT_STATE, 0, sizeof(CPU_State));
  CURRENT_STATE.PC = 0x00000000;
}

void pipe_cycle()
{
    // Check for hazards that would require stalling:
    stall = 0; // Reset stall each cycle

    /* Hazards: a store or load followed immediately by use of the same register.
    Stall if the previous instruction is writing to a register used by the current instruction */

    // Stall if SW is writing to register used by current instruction (if memWrite flag is true)
    if ((Reg_DEtoEX.rs1 == Reg_EXtoMEM.rd || Reg_DEtoEX.rs2 == Reg_EXtoMEM.rd) && Reg_EXtoMEM.memWrite) {
        stall = 1;
    }
    // Stall if LW is reading or writing register used by current instruction (if memRead flag is true)
    if ((Reg_DEtoEX.rs1 == Reg_EXtoMEM.rd || Reg_DEtoEX.rs2 == Reg_EXtoMEM.rd) && Reg_MEMtoWB.memRead) {
        stall = 1;
    }

    // Always run write back and memory stages
    pipe_stage_wb();
    pipe_stage_mem();

    // Delay EX, DE, and IF if there is a stall
    if (!stall) {
        pipe_stage_execute();
        pipe_stage_decode();
        pipe_stage_fetch();
    }

    // If no destination register, alu flag, memread, or next instruction, set RUN_BIT to 0 to terminate program
    if(Reg_MEMtoWB.rd == 0 && Reg_EXtoMEM.aluDone == false && Reg_MEMtoWB.aluDone == false 
        && Reg_MEMtoWB.memRead == false && Reg_IFtoDE.instr == 0){
            RUN_BIT = 0;
        }
}

void pipe_stage_wb()
{
    // Check if there's a destination register (rd) to update
    if (Reg_MEMtoWB.rd != 0) {  // Assume that rd==0 is not written back (follwing RISC-V conventions)
        if (Reg_MEMtoWB.memRead) {
            // If the instruction was load, write the data into rd
            CURRENT_STATE.REGS[Reg_MEMtoWB.rd] = Reg_MEMtoWB.memData;
        } // Else if EX to MEM or MEM to WB pipeline contain flag for an ALU operation
        else if (Reg_EXtoMEM.aluDone || Reg_MEMtoWB.aluDone) {
            // Write ALU result into rd
            CURRENT_STATE.REGS[Reg_MEMtoWB.rd] = Reg_MEMtoWB.aluResult;
            Reg_MEMtoWB.aluDone = false;
        }
    }

    // If a branch was taken, set current PC to branch target
    if (Reg_MEMtoWB.branchTaken) {
        CURRENT_STATE.PC = Reg_MEMtoWB.branchTarget;
    }

    memset(&Reg_MEMtoWB, 0, sizeof(Reg_MEMtoWB)); // Clear MEM to WB pipeline after use
}

void pipe_stage_mem()
{
    // Handle memory operations
    if (Reg_EXtoMEM.memRead) {
        // Perform memory read
        Reg_MEMtoWB.memData = mem_read_32(Reg_EXtoMEM.memAddress);
        Reg_MEMtoWB.memRead = true; // Indicate that data was read into this register
    }
    if (Reg_EXtoMEM.memWrite) {
        // Perform memory write
        mem_write_32(Reg_EXtoMEM.memAddress, Reg_EXtoMEM.storeData);
        Reg_MEMtoWB.aluDone = false; // Indicate that operation is done, no ALU operation necessary
    }

    // Pass ALU result, ALU operation flag, and rd for the Write Back stage
    Reg_MEMtoWB.aluDone = Reg_EXtoMEM.aluDone;
    Reg_MEMtoWB.aluResult = Reg_EXtoMEM.aluResult;
    Reg_MEMtoWB.rd = Reg_EXtoMEM.rd;

    // Pass control signals for branch handling
    Reg_MEMtoWB.branchTaken = Reg_EXtoMEM.branchTaken;
    Reg_MEMtoWB.branchTarget = Reg_EXtoMEM.branchTarget;

    memset(&Reg_EXtoMEM, 0, sizeof(Reg_EXtoMEM)); // Clear MEM to WB pipeline after use
}

void pipe_stage_execute()
{
    // Pull data from decode pipeline
    opcode = Reg_DEtoEX.opcode;
    rd = Reg_DEtoEX.rd;
    rs1 = Reg_DEtoEX.rs1;
    rs2 = Reg_DEtoEX.rs2;
    funct3 = Reg_DEtoEX.funct3;
    imm = Reg_DEtoEX.imm;

    // Set value of r1 and r2 to local variable for manipulation
    int32_t rs1_val = CURRENT_STATE.REGS[rs1];
    int32_t rs2_val = CURRENT_STATE.REGS[rs2];

    // Forwarding Logic
    // Check MEM to WB forwarding (if result has reached WB stage)
    if (Reg_MEMtoWB.rd == rs1 && Reg_MEMtoWB.rd != 0) {
        rs1_val = Reg_MEMtoWB.aluResult;  // Forward from EX to MEM/WB if rs1 needs the value from a recent ALU operation
        Reg_MEMtoWB.aluDone = true;       // Flag that WB should process ALU operation
    }
    if (Reg_MEMtoWB.rd == rs2 && Reg_MEMtoWB.rd != 0) {
        rs2_val = Reg_MEMtoWB.aluResult;  // Forward from EX to MEM/WB if rs2 needs the value from a recent ALU operation
        Reg_MEMtoWB.aluDone = true;       // Flag that WB should process ALU operation
    }

    // Check EX to MEM forwarding (handles the scenario where the result needed hasn't reached the WB stage yet)
    if (Reg_EXtoMEM.rd == rs1 && Reg_EXtoMEM.rd != 0) {
        rs1_val = Reg_EXtoMEM.aluResult;  // Immediate forwarding from EX to EX/MEM if rs1 needs the value from the current EX stage
        Reg_EXtoMEM.aluDone = true;       // Flag that MEM should send to WB to process ALU operation
    }
    if (Reg_EXtoMEM.rd == rs2 && Reg_EXtoMEM.rd != 0) {
        rs2_val = Reg_EXtoMEM.aluResult;  // Immediate forwarding from EX to EX/MEM if rs2 needs the value from the current EX stage
        Reg_EXtoMEM.aluDone = true;       // Flag that MEM should send to WB to process ALU operation
    }

    // Swtich to determine and execute type of instruction based on opcode
    switch (opcode) {
        case 0x33: // R-type instructions
            // ADD
            Reg_EXtoMEM.aluResult = rs1_val + rs2_val; // Adds rs1 and rs2, stores in ALU result
            Reg_EXtoMEM.aluDone = true; // Flag that MEM should send to WB to process ALU operation
            break;

        case 0x13: // I-type instructions
            // ADDI
            Reg_EXtoMEM.aluResult = rs1_val + imm; // Add immediate to rs1 and store in ALU result
            Reg_EXtoMEM.aluDone = true; // Flag that MEM should send to WB to process ALU operation
            break;
        
        case 0x3: // I-type instructions
            // LW
            Reg_EXtoMEM.memAddress = rs1_val + imm; // Calculate load address
            Reg_EXtoMEM.memRead = true;  // Indicate that mem should perform a read operation
            break;

        case 0x23: // S-type instructions
            // SW
            Reg_EXtoMEM.memAddress = rs1_val + imm;
            Reg_EXtoMEM.storeData = rs2_val;
            Reg_EXtoMEM.memWrite = true;  // Indicate that mem should perform a write operation
            break;

        case 0x63: // SB-type instructions
            // BLT
            if (rs1_val < rs2_val) {
                Reg_EXtoMEM.branchTarget = Reg_IFtoDE.pc + imm;  // Update PC if the branch is taken
                Reg_EXtoMEM.branchTaken = true;
            }
            break;

        case 0x17: // U-type instructions
            // AUIPC
            Reg_EXtoMEM.aluResult = (Reg_DEtoEX.pc + imm) << 12; // Use PC from decode stage
            Reg_EXtoMEM.aluDone = true; // Flag that MEM should send to WB to process ALU operation
            break;
    }

    // Pass destination register to mem
    Reg_EXtoMEM.rd = rd;

    memset(&Reg_DEtoEX, 0, sizeof(Reg_DEtoEX)); // Clear DE to EX after use
}

void pipe_stage_decode()
{
    // Pass instruction to local variable
    uint32_t instr = Reg_IFtoDE.instr;

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
            break;
        case 0x3:   // I-type instructions (LW)
            imm = (int32_t)instr >> 20; // Sign extend the immediate
            break;
        case 0x23:  // S-type instructions (SW)
            imm = ((instr >> 7) & 0x1F) | ((instr >> 25) << 5); // Assemble immediate value for store (bits 4:0, 11:5)
            imm = ((int32_t)imm << 20) >> 20; // Sign extend the immediate
            break;
        case 0x63:  // SB-type instructions (BLT)
            imm = ((instr >> 7) & 0x1E) | ((instr >> 25) << 5) | ((instr & 0x80) << 4) | ((instr >> 31) << 11); // Assemble immediate value for branch (bits 11, 4:1, 10:5, 12)
            imm = ((int32_t)imm << 20) >> 20; // Sign extend the immediate
            break;
        case 0x17:  // U-type instructions (AUIPC)
            imm = (instr & 0xFFFFF000) >> 12; // Get 20-bit upper immediate with leading zeroes
            break;
    }

    // Pass immediate to execute
    Reg_DEtoEX.imm = imm;
}

void pipe_stage_fetch()
{
    // Pull instruction at current pc to be sent to DE through pipeline
    Reg_IFtoDE.instr = mem_read_32(CURRENT_STATE.PC);

    // If instruction is not 0 and EX has not flagged a branch operation
    if (Reg_IFtoDE.instr != 0 && Reg_EXtoMEM.branchTaken == false){
        Reg_IFtoDE.pc = CURRENT_STATE.PC;   // Record current PC for instruction being processed
        CURRENT_STATE.PC = CURRENT_STATE.PC+4; // Increment PC
    }
}
