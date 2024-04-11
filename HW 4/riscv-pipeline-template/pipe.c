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
}

void pipe_stage_mem()
{
}

void pipe_stage_execute()
{
}

void pipe_stage_decode()
{
}

void pipe_stage_fetch()
{
  Reg_IFtoDE.instr = mem_read_32(CURRENT_STATE.PC); 
  CURRENT_STATE.PC = CURRENT_STATE.PC+4; // increment PC
}
