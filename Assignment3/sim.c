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

uint32_t instr;

uint32_t opcode = 0b0000000;
uint32_t func3 = 0b000;
uint32_t func7 = 0b0000000;

uint32_t rs1 = 0b00000;
uint32_t rs2 = 0b00000;
uint32_t rsd = 0b00000;

uint32_t i_imm = 0b000000000000;

uint32_t s_imm_1 = 0b0000000;
uint32_t s_imm_2 = 0b00000;

uint32_t sb_imm_1 = 0b0000000;
uint32_t sb_imm_2 = 0b00000;

uint32_t u_imm = 0b00000000000000000000;

uint32_t uj_imm = 0b00000000000000000000;

uint32_t rs1_data = 0b00000;
uint32_t rs2_data = 0b00000;
uint32_t rsd_data = 0b00000;

char instr_type;

void fetch()
{
  if ( RUN_BIT == 1){
    //get instruction

    instr = mem_read_32(CURRENT_STATE.REGS[CURRENT_STATE.PC]);

    printf("%ls \n", &instr);

    //get instruction

    //update state

    NEXT_STATE.PC = CURRENT_STATE.PC;

    //update state

  }

} 

void decode()
{
  //get opcode

  opcode = instr & 0x7f;//get opcode from bits 7-0. 0x7f = 1111110000000000

  //get opcode

  //find instruction

  switch(opcode){
    case 0b0110011: //add and slt
      rsd =(instr >> 7) & 0x1f; //0x1f = 11111 in binary
      func3 = (instr >> 12) & 0x07; //0x07 = 111 in binary
      rs1 = (instr >> 15) & 0x1f;
      rs2 = (instr >> 20) & 0x1f;
      func7 = (instr>> 25) & 0x7f;

      instr_type = 'r';
      if(func3 == 0b111){//add

      }else if(func3 == 0b010){//slt

      }
      break;

    case 0b0010011://addi and slli
      i_imm = (instr >> 20) & 0x0fff;// 0x0777 = 111111111111 in binary
      rs1 = (instr >> 15) & 0x1f;
      func3 = (instr >> 12) & 0x07;
      rsd = (instr >> 7) & 0x1f;

      instr_type = 'i';
      if(func3 == 0b000){//addi

      }else if(func3 == 0b001){//slli

      }
      break;

    case 0b0100011: //sw
      s_imm_1 = (instr >> 25) & 0x7f;
      rs2 = (instr >> 20) & 0x1f;
      rs1 = (instr >> 15) & 0x1f;
      func3 = (instr >> 13) & 0x07;
      s_imm_2 = (instr >> 7) & 0x1f;

      instr_type = 's';
      break;

    case 0b1100011: //bne
      s_imm_1 = (instr >> 25) & 0x7f;
      rs2 = (instr >> 20) & 0x1f;
      rs1 = (instr >> 15) & 0x1f;
      func3 = (instr >> 13) & 0x07;
      s_imm_2 = (instr >> 7) & 0x1f;

      instr_type = 'b';
      break;

    case 0b0010111: //auipc
      u_imm = (instr >> 12) & 0x3fffff;
      rsd = (instr >> 7) & 0x1f;

      instr_type = 'u';
      break;

    case 0b1101111: //jal
      uj_imm = (instr >> 12) & 0x3fffff;
      rsd = (instr >>7) & 0x1f;

      instr_type = 'j';
      break;

  }

  //find instruction

}

void execute()
{
  switch(instr_type){
    case 'r': //add and slt
      if(func3 == 0b111){//add
        rs1_data = mem_read_32(rs1);
        rs2_data = mem_read_32(rs2);
        rsd_data = rs1_data + rs2_data;

        mem_write_32(rsd, rsd_data);

      }else if(func3 == 0b010){//slt
        rs1_data = mem_read_32(rs1);
        rs2_data = mem_read_32(rs2);
        rsd_data = rs1_data - rs2_data;

        mem_write_32(rsd, rsd_data);

      }
      break;

    case 'i': //addi and slli
      if(func3 == 0b000){//addi
        rs1_data = mem_read_32(rs1);
        rsd_data = rs1_data + i_imm;

        mem_write_32(rsd, rsd_data);
      }else if(func3 == 0b001){//slli
        rs1_data = mem_read_32(rs1);
        rsd_data = i_imm - rs1_data;

        mem_write_32(rsd, rsd_data);

      }
      break;

    case 's': //sw
      rs1_data = mem_read_32(rs1);
      mem_write_32(rsd,rs1_data);
      break;

    case 'b': //bne
      rs1_data = mem_read_32(rs1);
      rs2_data = mem_read_32(rs1);
      
      if((rs1_data - rs2_data) != 0){
        NEXT_STATE.PC = (sb_imm_1 >> 5) | sb_imm_2;
      }
      break;

    case 'u': //auipc
      uint32_t pc_update = 0b000000000000;
      pc_update = pc_update | (u_imm >>11);
      break;

    case 'j': //jal
      uint32_t pc_update = 0b000000000000;
      pc_update = pc_update | (u_imm >>11);
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
