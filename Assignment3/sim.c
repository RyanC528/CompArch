/* HEADER
  Name: Ryan Copeland
  Assignment: HW3

  I use github since I worked across two computers. The LF version of file it would be located in repo
  REPO: https://github.com/RyanC528/CompArch/tree/main/Assignment3
*/

#include <stdio.h>
#include "shell.h"

/* Global variables pulled from shell.h for my sanity

typedef struct CPU_State_Struct {
  uint32_t PC;		 program counter 
  int32_t REGS[RISCV_REGS]; // register file. 
  int FLAG_NV;        // invalid operation 
  int FLAG_DZ;        // divide by zero 
  int FLAG_OF;        // overflow 
  int FLAG_UF;        // underflow 
  int FLAG_NX;        // inexact 
} CPU_State;

enum Opcode {SPECIAL, J};

extern CPU_State CURRENT_STATE, NEXT_STATE;

extern int RUN_BIT;	// run bit 

uint32_t mem_read_32(uint32_t address);
void     mem_write_32(uint32_t address, uint32_t value);

*/

uint32_t instrct;
int opcode = 0b0000000;
int func3 = 0b000;

char instr_type;

void fetch()
{
  if ( RUN_BIT == 1){
    //get instruction

    instrct = mem_read_32(CURRENT_STATE.REGS[CURRENT_STATE.PC]);

    printf("%ls \n", &instrct);

    //get instruction

    //update state

    NEXT_STATE.PC = CURRENT_STATE.PC;

    //update state

  }

} 

void decode()
{
  //get opcode

  opcode = (instrct >> 6);//get opcode from bits 7-0

  //get opcode

  //find instruction

  switch(opcode){
    case 0b0110011: //add and slt
      instr_type = "r";
      if(func3 == 0b111){//add

      }else if(func3 == 0b010){//slt

      }
      break;

    case 0b0010011://addi and slli
      instr_type = "i";
      if(func3 == 0b000){//addi

      }else if(func3 == 0b001){//slli

      }
      break;

    case 0b0100011: //sw
      instr_type = "s";
      break;

    case 0b1100011: //bne
      instr_type = "d";
      break;

    case 0b0010111: //auipc
      instr_type = "u";
      break;

    case 0b1101111: //jal
      instr_type = "j";
      break;

  }

  //find instruction

}

void execute()
{
  switch(instr_type){
    case "r": //add and slt
      if(func3 == 0b111){//add

      }else if(func3 == 0b010){//slt

      }
      break;

    case "i": //add and slt
      if(func3 == 0b000){//addi

      }else if(func3 == 0b001){//slli

      }
      break;

    case "s": //add and slt
      break;

    case "b": //add and slt
      break;

    case "u": //add and slt
      break;

    case "b": //add and slt
      break;
  }

  
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
