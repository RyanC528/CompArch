/* Ryan Copeland | Gabriel Gonzalez | EECE 4821 | HW4 | pipe.h | 4/14/2024
  REPO: https://github.com/RyanC528/CompArch/tree/main/HW4
*/

#ifndef _PIPE_H_
#define _PIPE_H_

#include "shell.h"
#include "stdbool.h"
#include <limits.h>

#define RISCV_REGS 32

typedef struct CPU_State_Struct {
  uint32_t PC;		          /* program counter */
  int32_t REGS[RISCV_REGS]; /* register file. */
  int FLAG_NV;              /* invalid operation */
  int FLAG_DZ;              /* divide by zero */
  int FLAG_OF;              /* overflow */
  int FLAG_UF;              /* underflow */
  int FLAG_NX;              /* inexact */
} CPU_State;

typedef struct Pipe_Reg_IFtoDE {
  uint32_t instr;   // Input instruction
  uint32_t pc;      // Program counter
} Pipe_Reg_IFtoDE;

typedef struct Pipe_Reg_DEtoEX {
  uint32_t opcode;  // Opcode
  uint32_t rd;      // Desitnation register
  uint32_t rs1;     // First rource register
  uint32_t rs2;     // Second source register
  uint32_t funct3;  // Funct3 field
  uint32_t funct7;  // Funct7 field (for R-type instructions)
  int32_t imm;      // Immediate value
  uint32_t pc;      // Program counter
} Pipe_Reg_DEtoEX;

typedef struct Pipe_Reg_EXtoMEM {
  uint32_t rd;            // Holds destination register
  uint32_t memRead;       // Flag to indicate data was read into register
  uint32_t memWrite;      // Flag to indicate data needs to be written to register
  uint32_t memAddress;    // Holds memory address for read/write operations
  uint32_t storeData;     // Holds data for write operations
  uint32_t aluResult;     // Holds ALU result for arithmetic operations
  uint32_t aluDone;       // Flag to indicate WB needs to write ALU data to register
  uint32_t branchTaken;   // Flag to indicate WB should write branch target to PC
  uint32_t branchTarget;  // Holds target address for branch operations
} Pipe_Reg_EXtoMEM;

typedef struct Pipe_Reg_MEMtoWB {
  uint32_t rd;            // Holds destination register
  uint32_t memRead;       // Flag to indicate data was read into register
  uint32_t memData;       // Holds data found at memAddress for read operations
  uint32_t aluResult;     // Holds ALU result for arithmetic operations
  uint32_t aluDone;       // Flag to indicate WB needs to write ALU data to register
  uint32_t branchTaken;   // Flag to indicate WB should write branch target to PC
  uint32_t branchTarget;  // Holds target address for branch operations
} Pipe_Reg_MEMtoWB;


extern int RUN_BIT;

/* global variable -- pipeline state */
extern CPU_State CURRENT_STATE,NEXT_STATE;

/* called during simulator startup */
void pipe_init();

/* this function calls the others */
void pipe_cycle();

/* each of these functions implements one stage of the pipeline */
void pipe_stage_fetch();
void pipe_stage_decode();
void pipe_stage_execute();
void pipe_stage_mem();
void pipe_stage_wb();

#endif
