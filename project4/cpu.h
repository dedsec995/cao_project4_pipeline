#ifndef _CPU_H_
#define _CPU_H_
#include <stdbool.h>
#include <assert.h>

#define MAX_LENGTH 1024
#define NO_OF_LINE 256

typedef struct Register
{
    long value;          // contains register value
	int is_writing;    // indicate that the register is current being written
	                    // True: register is not ready
						// False: register is ready
	int freed_this_cycle;
	int is_issued; // IN the pipeline
	int status;
	int tag;
} Register;

typedef struct REbuff
{
	char renamed_reg[20];
	int dest;
	int result;
	int head;
	int tail;
	int completed;
} REbuff;

typedef struct Btb
{
    int tag;
	int target;
} Btb;

typedef struct Pt
{
    int pattern;
} Pt;

typedef struct Stages
{
	char opcode_str[128]; // The instruction
    int pc;          // Program Counter
    bool has_inst;   // Is this the turn of this Pipeline Stage
	int instAddr; // Instruction what??
	int instLen; // Current Instruction Length
	char opcode[128]; // Opcode
	char rm[20]; // Renamed Register
	char rg1[20]; // Register 1
	char renamed_rg1[20]; // The renamed Registered
	char rg2[20]; // Register 2
	char rg3[20]; // Register 3
	int rg1_val; // Register 1 Value
	int rg2_val; // Register 2 value
	int rg3_val; // Register 3 Value
	long buffer; // Temp value
	char or1[20]; // oprand 1
	char or2[20]; // oprand 2
	int halt_triggered; // ???
	int branch_stall;
	int unfreeze;
	int branch_taken; // Is the branch Taken at Fetch

} Stages;

/* Model of CPU */
typedef struct CPU
{
	/* Integer register file */
	Register *regs;	// The registers
	Btb *btb; // The Branch Target Buffer
	Pt *pt;
	REbuff *rebuff;
	char* filename; // File to be read
	char instructions[NO_OF_LINE][MAX_LENGTH]; // Instructions Char array
	int instructionLength; // Total Instruction Length
	int hazard; // Total structural hazard done
	float ipc; // Instructions per cycle
	int pc; // Program Counter
	int clock; // Total Cycle Completed
	char mem1_reg[20]; // Save forwarded value from Mem1
	int mem1_val; // Save forwarded value from Mem1
	char br_reg[20]; // Save forwarded value from BR
	int br_val; // Save forwarded value from BR
	char div_reg[20]; // Save forwarded value from DIV
	int div_val; // Save forwarded value from DIV
	char mul_reg[20]; // Save forwarded value from MUL
	int mul_val; // Save forwarded value from MUL
	char add_reg[20]; // Save forwarded value from ADD
	int add_val; // Save forwarded value from ADD
	int raw; // Read After Write
	char freedit[20]; // Which REG was freed??
	int reverse_branch; // Not branch as BR has Squashed THE pipeline
	int executed_instruction; // Executed Instruction Counter
	int reserve_count; // Counter for Reverse Station

	// The Pipeline
	Stages fetch_latch; 
	Stages decode_latch;
	Stages analysis_latch;
	Stages instruction_rename_latch;
	Stages reserve_station[20];
	Stages issue_latch;
	Stages memory1_latch;
	Stages memory2_latch;
	Stages divider1_latch;
	Stages memory3_latch;
	Stages divider2_latch;
	Stages multiplier1_latch;
	Stages memory4_latch;
	Stages divider3_latch;
	Stages multiplier2_latch;
	Stages adder_latch;
	Stages writeback1_latch;
	Stages writeback2_latch;
	Stages writeback3_latch;
	Stages writeback4_latch;
	Stages reorder2_latch;
	Stages reorder1_latch;	

} CPU;

//	    |						      |
//	    | Function Declarations Below |
//	    |						      |
//	    v						      v


CPU*
CPU_init(char* filename);

Register*
create_registers(int size);

Btb*
create_btb(int size);

Pt*
create_pt(int size);

REbuff*
create_rebuff(int size);

int
write_the_memory(long val, int num);

void
make_memory_map();

int
CPU_run(CPU* cpu);

void
CPU_stop(CPU* cpu);

void 
simulate(CPU* cpu);

void 
super_scalar(CPU* cpu);

void
clear_forwarding(CPU* cpu);

void
reorder_buff(CPU* cpu);

void
reserve_station_buff(CPU* cpu);

void 
fetch_unit(CPU* cpu);

void 
decode_unit(CPU* cpu);

void 
analysis_unit(CPU* cpu);

void 
instruction_rename_unit(CPU* cpu);

void 
issue_unit(CPU* cpu);

void
memory1_unit(CPU* cpu);

void
memory2_unit(CPU* cpu);

void 
divider1_unit(CPU* cpu);

void
memory3_unit(CPU* cpu);

void 
divider2_unit(CPU* cpu);

void 
multiplier1_unit(CPU* cpu);

void
memory4_unit(CPU* cpu);

void 
divider3_unit(CPU* cpu);

void 
multiplier2_unit(CPU* cpu);

void 
adder_unit(CPU* cpu);

void
writeback1_unit(CPU* cpu);

void
writeback2_unit(CPU* cpu);

void
writeback3_unit(CPU* cpu);

void
writeback4_unit(CPU* cpu);

void
reorder1_unit(CPU* cpu);

void
reorder2_unit(CPU* cpu);

int
get_tag(long num);

#endif
