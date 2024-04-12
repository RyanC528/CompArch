#ifndef _PIPE_H_
#define _PIPE_H_

#include "shell.h"
#include "stdbool.h"
#include <limits.h>

#define RISCV_REGS 32

typedef struct CPU_State_Struct {
  uint32_t PC;		/* program counter */
  int32_t REGS[RISCV_REGS]; /* register file. */
  int FLAG_NV;        /* invalid operation */
  int FLAG_DZ;        /* divide by zero */
  int FLAG_OF;        /* overflow */
  int FLAG_UF;        /* underflow */
  int FLAG_NX;        /* inexact */
} CPU_State;

typedef struct Pipe_Reg_IFtoDE {
  uint32_t instr;
  // fill additional information to store here
} Pipe_Reg_IFtoDE;

typedef struct Pipe_Reg_DEtoEX {
  // fill additional information to store here
  uint32_t opcode;
  uint32_t rd;
  uint32_t rs1;
  uint32_t rs2;
  uint32_t funct3;
  uint32_t funct7;
  int32_t imm;

} Pipe_Reg_DEtoEX;

typedef struct Pipe_Reg_EXtoMEM {
  // fill additional information to store here
  uint32_t memRead;
  uint32_t memWrite;
  uint32_t memAddress;
  uint32_t storeData;
  uint32_t aluResult;

  uint32_t rd;
  uint32_t branchTaken;
  uint32_t branchTarget;

} Pipe_Reg_EXtoMEM;

typedef struct Pipe_Reg_MEMtoWB {
  // fill additional information to store here
  uint32_t rd;
  uint32_t memRead;
  uint32_t memData;
  uint32_t aluResult;
  uint32_t branchTaken;
  uint32_t branchTarget;


} Pipe_Reg_MEMtoWB;


extern int RUN_BIT;

/* global variable -- pipeline state */
extern CPU_State CURRENT_STATE;

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
