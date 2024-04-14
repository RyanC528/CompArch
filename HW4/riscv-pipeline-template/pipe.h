#ifndef _PIPE_H_
#define _PIPE_H_

/* HEADER
  Name: Ryan Copeland, Gaberial Gonzelas
  Assignment: HW4, pipe.h
  REPO: https://github.com/RyanC528/CompArch/tree/main/HW4
*/

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

  uint32_t instr; //input instruction
  uint32_t pc;//program counter

} Pipe_Reg_IFtoDE;

typedef struct Pipe_Reg_DEtoEX {
  //taken from instruction
  uint32_t opcode;//opcode
  uint32_t rd; //desitnation register
  uint32_t rs1; //register 1
  uint32_t rs2; //register 2
  uint32_t funct3; //function 3 code
  uint32_t funct7; //function 7 code
  int32_t imm; //intermediate

  uint32_t pc;//program counter

} Pipe_Reg_DEtoEX;

typedef struct Pipe_Reg_EXtoMEM {
  //taken largely from opcode
  uint32_t memRead;// read memory bit
  uint32_t memWrite;// write memory bit
  uint32_t memAddress;//memory address
  uint32_t storeData;//data to be store
  uint32_t aluResult;// alu result
  uint32_t aluDone;//alu completion bit

  uint32_t rd;// destination register
  uint32_t branchTaken;//branch taken
  uint32_t branchTarget;//target branch

} Pipe_Reg_EXtoMEM;

typedef struct Pipe_Reg_MEMtoWB {
  uint32_t rd; //destination register
  uint32_t memRead; //read memory bit
  uint32_t memData; //write memory bit
  uint32_t aluResult; //alu result
  uint32_t branchTaken; //branch taken
  uint32_t branchTarget; // target branch
  uint32_t aluDone; // alu done bit


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
