// RISC-V Instruction Level Simulator 
//
// Do not modify this file!!!
//

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "shell.h"

// main memory
#define MEM_DATA_START  0x10000000 
#define MEM_DATA_SIZE   0x00100000
#define MEM_TEXT_START  0x00400000
#define MEM_TEXT_SIZE   0x00400000
#define MEM_STACK_START 0xfffffff0
#define MEM_STACK_SIZE  0x00100000

typedef struct {
  uint32_t start, size; // uint64_t in case of 64-bit RISCV
  uint8_t *mem;
} mem_region_t;

/* memory will be dynamically allocated at initialization */
mem_region_t MEM_REGIONS[] = {
  { MEM_TEXT_START, MEM_TEXT_SIZE, NULL },
  { MEM_DATA_START, MEM_DATA_SIZE, NULL },
  { MEM_STACK_START, MEM_STACK_SIZE, NULL },
};

#define MEM_NREGIONS (sizeof(MEM_REGIONS)/sizeof(mem_region_t))

char *reg_mnemonic[RISCV_REGS] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0/fp", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};
  
// CPU State
CPU_State CURRENT_STATE, NEXT_STATE;
int RUN_BIT;	/* run bit */
int INSTRUCTION_COUNT;

FILE *dumpsim_file;

uint32_t mem_read_32(uint32_t address)
{
  int i;
  for (i = 0; i < MEM_NREGIONS; i++) {
    if (address >= MEM_REGIONS[i].start &&
	address < (MEM_REGIONS[i].start + MEM_REGIONS[i].size)) {
      uint32_t offset = address - MEM_REGIONS[i].start;
      
      return
	(MEM_REGIONS[i].mem[offset+3] << 0) |
	(MEM_REGIONS[i].mem[offset+2] << 8) |
	(MEM_REGIONS[i].mem[offset+1] << 16) |
	(MEM_REGIONS[i].mem[offset+0] << 24);
    }
  }
  
  return 0;
}

void mem_write_32(uint32_t address, uint32_t value)
{
  int i;
  for (i = 0; i < MEM_NREGIONS; i++) {
    if (address >= MEM_REGIONS[i].start &&
	address < (MEM_REGIONS[i].start + MEM_REGIONS[i].size)) {
      uint32_t offset = address - MEM_REGIONS[i].start;
      
      MEM_REGIONS[i].mem[offset+3] = (value >> 0) & 0xFF;
      MEM_REGIONS[i].mem[offset+2] = (value >> 8) & 0xFF;
      MEM_REGIONS[i].mem[offset+1] = (value >> 16) & 0xFF;
      MEM_REGIONS[i].mem[offset+0] = (value >> 24) & 0xFF;
      return;
    }
  }
}

int help(char **args) {                                                    
  printf("-----------------RISCV SIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("input reg_no reg_value - set GPR reg_no to reg_value  \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
  return 1;
}

void cycle() {                                                
  process_instruction();
  CURRENT_STATE = NEXT_STATE;
  INSTRUCTION_COUNT++;
}

int run(char **args) {
  int num_cycles;
  int i;

  if (args[1] == NULL) {
    printf("Incorrect run cmd: missing # of instrucitons to run\n\n");
    return 1;
  }

  num_cycles = atoi(args[1]);
  if (num_cycles <= 0) {
    printf("Incorrect run cmd: num_cycles should be positive!\n\n");
    return 1;
  }

  if (RUN_BIT == FALSE) {
    printf("Can't simulate: Simulator is halted\n\n");
    return 1;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (RUN_BIT == FALSE) {
      printf("Simulator halted\n\n");
      break;
    }
    cycle();
  }

  return 1;
}

int go(char **args) {                                                     
  if (RUN_BIT == FALSE) {
    printf("Can't simulate, Simulator is halted\n\n");
    return 1;
  }

  printf("Simulating...\n\n");
  while (RUN_BIT)
    cycle();
  printf("Simulator halted\n\n");

  return 1;
}

int mdump(char **args) {
  int address, start, stop;

  if (args[1] == NULL || args[2] == NULL) {
    printf("incorrect mdump syntax: missing start and/or stop address\n\n");
    return 1;
  }

  start = strtol(args[1], NULL, 16);
  stop = strtol(args[2], NULL, 16);

  printf("\nMemory content [0x%08x..0x%08x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = start; address <= stop; address += 4)
    printf("  0x%08x (%d) : 0x%08x\n", address, address, mem_read_32(address));
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%08x..0x%08x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = start; address <= stop; address += 4)
    fprintf(dumpsim_file, "  0x%08x (%d) : 0x%x\n", address, address, mem_read_32(address));
  fprintf(dumpsim_file, "\n");
  
  return 1;
}

int rdump(char **args) {
  int k; 

  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Instruction Count : %u\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%08" PRIx32 "\n", CURRENT_STATE.PC);
  printf("Registers:\n");
  for (k = 0; k < RISCV_REGS; k++)
    printf("x%d (%s):\t0x%08" PRIx32 "\n", k, reg_mnemonic[k], CURRENT_STATE.REGS[k]);
  printf("FLAG_NV: %d\n", CURRENT_STATE.FLAG_NV);
  printf("FLAG_DZ: %d\n", CURRENT_STATE.FLAG_DZ);
  printf("FLAG_OF: %d\n", CURRENT_STATE.FLAG_OF);
  printf("FLAG_UF: %d\n", CURRENT_STATE.FLAG_UF);
  printf("FLAG_NX: %d\n", CURRENT_STATE.FLAG_NX);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %u\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%" PRIx32 "\n", CURRENT_STATE.PC);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < RISCV_REGS; k++)
    fprintf(dumpsim_file, "x%d: 0x%" PRIx32 "\n", k, CURRENT_STATE.REGS[k]);
  fprintf(dumpsim_file, "FLAG_NV: %d\n", CURRENT_STATE.FLAG_NV);
  fprintf(dumpsim_file, "FLAG_DZ: %d\n", CURRENT_STATE.FLAG_DZ);
  fprintf(dumpsim_file, "FLAG_OF: %d\n", CURRENT_STATE.FLAG_OF);
  fprintf(dumpsim_file, "FLAG_UF: %d\n", CURRENT_STATE.FLAG_UF);
  fprintf(dumpsim_file, "FLAG_NX: %d\n", CURRENT_STATE.FLAG_NX);
  fprintf(dumpsim_file, "\n");

  return 1;
}

void init_memory() {
  int i;
  for (i = 0; i < MEM_NREGIONS; i++) {
    MEM_REGIONS[i].mem = malloc(MEM_REGIONS[i].size);
    memset(MEM_REGIONS[i].mem, 0, MEM_REGIONS[i].size);
  }
}

void load_program(char *program_filename) {                   
  FILE * prog;
  int ii, word;

  /* Open program file. */
  prog = fopen(program_filename, "r");
  if (prog == NULL) {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
  }

  /* Read in the program. */
  ii = 0;
  while (fscanf(prog, "%x\n", &word) != EOF) {
    mem_write_32(MEM_TEXT_START + ii, word);
    ii += 4;
  }

  CURRENT_STATE.PC = MEM_TEXT_START;

  printf("Read %d words from program into memory.\n\n", ii/4);
}

void initialize(char *program_filename, int num_prog_files) { 
  int i;

  init_memory();
  for ( i = 0; i < num_prog_files; i++ ) {
    load_program(program_filename);
    while (*program_filename++ != '\0');
  }
  NEXT_STATE = CURRENT_STATE;
    
  RUN_BIT = TRUE;
}

int exit_shell(char **args)
{
  printf("Bye.\n");
  return 0;
}

int input_cmd(char **args)
{
  int reg_no, reg_value;

  if (args[1] == NULL || args[2] == NULL) {
    printf("Incorrect input syntax: missing reg_no and/or reg_value\n\n");
    return 1;
  }

  reg_no = atoi(args[1]);
  reg_value = atoi(args[2]);

  if (reg_no < 0 || reg_no >= RISCV_REGS) {
    printf ("Incorrect register number: should be 0, 1, ..., 31\n\n");
    return 1;
  }

  CURRENT_STATE.REGS[reg_no] = reg_value;
  NEXT_STATE.REGS[reg_no] = reg_value;

  return 1;
}

/*
  List of builtin commands, followed by their corresponding functions.
*/
char *builtin_str[] = {
  "g",
  "G",
  "go",
  "r",
  "R",
  "run",
  "mdump",
  "?",
  "h",
  "help",
  "q",
  "Q",
  "quit",
  "rdump",
  "i",
  "I",
  "input"
};

int (*builtin_func[]) (char **) = {
  &go,
  &go,
  &go,
  &run,
  &run,
  &run,
  &mdump,
  &help,
  &help,
  &help,
  &exit_shell,
  &exit_shell,
  &exit_shell,
  &rdump,
  &input_cmd,
  &input_cmd,
  &input_cmd
};

int num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
*/
int execute_cmd(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  printf("Invalid Command\n\n");
  return 1;
}

#define RL_BUFSIZE 1024
/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
*/
char *read_line(void)
{
  int bufsize = RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "shell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"
/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
*/
char **split_line(char *line)
{
  int bufsize = TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "shell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
	free(tokens_backup);
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

int main (int argc, char *argv[]) {                              
  int status;
  char *line;
  char **args;

  /* Error Checking */
  if (argc < 2) {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("RISCV Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  do {
    printf("RISCV-SIM> ");
    line = read_line();
    args = split_line(line);
    status = execute_cmd(args);
  } while (status);
    
}
