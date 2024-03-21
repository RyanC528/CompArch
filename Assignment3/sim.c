#include <stdio.h>
#include "shell.h"

/*
Name: Ryan Copeland
Assignment: HW3
*/

// CPU State - Taken from  shell.c
CPU_State CURRENT_STATE, NEXT_STATE;
int RUN_BIT;	/* run bit */
int INSTRUCTION_COUNT;

void fetch()
{

} 

void decode()
{

}

void execute()
{
  
}

void process_instruction()
{
  /* execute one instruction here. You should use CURRENT_STATE and modify
   * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
   * access memory. */
  fetch();
  decode();
  execute();
}
