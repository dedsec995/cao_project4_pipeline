
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include "cpu.h"

#define REG_COUNT 16
#define BTB_COUNT 16
#define REBUFF_COUNT 8
#define MAXY_LENGHT 512
#define MEMORY_MAP_LENGTH 16384
long memory_map_val[MEMORY_MAP_LENGTH] = {0};
int increase_head;
const char* print_pipeline;

CPU*
CPU_init(char* filename)
{
    CPU* cpu = malloc(sizeof(*cpu));
    if(!filename){
        printf("Error!! Missing filename");
        exit(0);
    }
    if (!cpu) {
        return NULL;
    }
    /* Create register files */
    cpu->regs= create_registers(REG_COUNT);
    cpu->btb= create_btb(BTB_COUNT);
    cpu->pt= create_pt(BTB_COUNT);
    cpu->rebuff = create_rebuff(REBUFF_COUNT);
    cpu->filename = filename;
    cpu->fetch_latch.has_inst = 1;
    cpu->fetch_latch.halt_triggered = 0;
    cpu->fetch_latch.branch_stall = 0;
    cpu->fetch_latch.unfreeze = 0;
    cpu->clock = 1;
    cpu->pc = 0;
    cpu->raw = 0;
    cpu->reverse_branch = 0;
    cpu->executed_instruction = 0;
    cpu->reserve_count = -1;
    increase_head = 0;
    make_memory_map();
    print_pipeline = getenv("PRINT_PIPELINE");
    strcpy(cpu->reserve_station[0].opcode,"none");
    return cpu;
}

/*
 * This function de-allocates CPU cpu.
 */
void
CPU_stop(CPU* cpu)
{
    free(cpu);
}

/*
 * This function prints the content of the registers.
 */
void
print_registers(CPU *cpu){
    
    
    printf("================================\n\n");

    printf("=============== STATE OF ARCHITECTURAL REGISTER FILE ==========\n\n");

    printf("--------------------------------\n");
    for (int reg=0; reg<REG_COUNT; reg++) {
        printf("REG[%2d]   |   Value=%ld  \n",reg,cpu->regs[reg].value);
        printf("--------------------------------\n");
    }
    printf("================================\n\n");
}

/*
 * This function prints the registers in the project 4 style.
 */
void
print_proj4_registers(CPU *cpu){
    
    printf("\n\n");
    printf("------------ STATE OF ARCHITECTURAL REGISTER FILE ----------\n");
    printf("R# [(status 0=invalid, 1=valid), tag, value] \n");
    for (int reg=0; reg<REG_COUNT; reg++) {
        printf("| R%d [(%d) %d, %d] ",reg,cpu->regs[reg].status,cpu->regs[reg].tag,cpu->regs[reg].value);
        if((reg+1)%4==0 && reg!=0){
            printf("\n");
        }
    }
    printf("\n--------------------------------------------------------------------------------\n\n");
}

/*
 * This function prints the content of the registers per cycle.
 */

//  dedsec995

void 
print_registers_cycle(CPU *cpu){
    printf("================================\n");
    printf("Clock Cycle #: %d\n", cpu->clock);
    printf("--------------------------------\n");

   for (int reg=0; reg<REG_COUNT; reg++) {
       
        printf("REG[%2d]   |   Value=%d  \n",reg,cpu->regs[reg].value);
        printf("--------------------------------\n");
    }
    printf("\n");
}

void print_btb(CPU *cpu){
    printf("\n");
    printf("============ BTB =================================\n");
    printf("\n");
    
    for (int i=0;i<BTB_COUNT; i++){
        printf("|	 BTB[%2d]	|	Tag=%d   |   Target=%d   |\n", i,cpu->btb[i].tag,cpu->btb[i].target);
    }
}

void print_pt(CPU *cpu){
    printf("\n");
    printf("============ Prediction Table  ==================\n");
    printf("\n");
    
    for (int i=0;i<BTB_COUNT; i++){
        printf("|	 PT[%2d]	|  Pattern=%d   |\n", i,cpu->pt[i].pattern);
    }
}

void print_rebuff(CPU *cpu){
    printf("\n");
    printf("------------ Reorder Buffer----------");
    printf("\n");
    
    for (int i=0;i<REBUFF_COUNT;i++){
        printf("| %s [dest: %d, result: %d, (e: 0, completed: %d)]",cpu->rebuff[i].renamed_reg,cpu->rebuff[i].dest,cpu->rebuff[i].result,cpu->rebuff[i].completed);
        printf("\n");
    }
    printf("Head: %d\nTail: %d",cpu->rebuff[0].head,cpu->rebuff[0].tail);
}

/*
 * This function read the file and add the data to string array 
 */
void
load_the_instructions(CPU *cpu){
    FILE *filePointer = fopen(cpu->filename, "r");
    int county = 0;
    if (filePointer == NULL)
    {
        printf("Error: could not open file %s", cpu->filename);
    }
    char buffer[MAXY_LENGHT];
    while (fgets(buffer, MAXY_LENGHT, filePointer)){
        strcpy(cpu->instructions[county],buffer);
        county++;
    }
    cpu->instructionLength = county ;
    fclose(filePointer);
}

int load_the_memory(){
    char *filename = "memory_map.txt"; 
    FILE *filePointer = fopen(filename, "r");
    int county = 0; 
    long n;
    if (filePointer == NULL) {
        printf("Error: could not open file %s", filename);
    }
    while (fscanf(filePointer, " %ld", &n) == 1) {
        memory_map_val[county] = n;
        county++;
    }
    fclose(filePointer);
    return (-1);
}

void make_memory_map(){
    char c;
    FILE *fptr1, *fptr2;
    char read_file[] = "memory_map.txt";
    char write_file[] = "output_memory_map.txt";
    fptr1 = fopen(read_file, "r");
    if (fptr1 == NULL)
    {
        printf("Cannot open file %s \n", read_file);
        exit(0);
    }
    fptr2 = fopen(write_file, "w+");
    if (fptr2 == NULL)
    {
        printf("Cannot open file %s \n", write_file);
        exit(0);
    }
    c = fgetc(fptr1);
    while (c != EOF)
    {
        fputc(c, fptr2);
        c = fgetc(fptr1);
    }
    fclose(fptr1);
    fclose(fptr2);
}

int write_the_memory(long val,int num){
    num = num/4;
    memory_map_val[num] = val;

    char *filename = "output_memory_map.txt"; 
    FILE *filePointer = fopen(filename, "w+");
    
    int county = 0;
    int n;
    if (filePointer == NULL)
    {
        printf("Error: could not open file %s", filename);
    }

    for (int i=0; i<MEMORY_MAP_LENGTH; i++){
        fprintf(filePointer, "%ld ", memory_map_val[i]);
    }
    fclose(filePointer);
    return (-1);
}

/*
 *  Get the tag using PC
 */

int get_tag(long num){
    long long bin = 0;
    int rem, q = 1;
    while (num!=0) {
        rem = num % 2;
        num /= 2;
        bin += rem * q;
        q *= 10;
    }
    for(int j=0;j<=5;j++){
        bin = bin/10;
    }
    int deci = 0, i = 0, reme;
    while (bin!=0) {
        reme = bin % 10;
        bin /= 10;
        deci += reme * pow(2, i);
        ++i;
    }
    return(deci);
}

/*
 *  CPU simulation loop
 */
int
CPU_run(CPU* cpu)
{
    load_the_instructions(cpu);
    cpu->hazard = 0;
    super_scalar(cpu);
    print_registers(cpu);
    cpu->ipc = (double)cpu->executed_instruction/(double)cpu->clock;
    printf("Stalled cycles due to data hazard: %d\n", cpu->hazard);
    printf("Total execution cycles: %d\n",cpu->clock);
    printf("Total instruction simulated: %d\n", cpu->executed_instruction);
    printf("IPC: %6f\n",cpu->ipc);
    return 0;
}

Register*
create_registers(int size){
    Register* regs = malloc(sizeof(*regs) * size);
    if (!regs) {
        return NULL;
    }
    for (int i=0; i<size; i++){
        regs[i].value = 0;
        regs[i].is_writing = 0;
        regs[i].is_issued = 0;
    }
    return regs;
}

REbuff*
create_rebuff(int size){
    REbuff* rebuff = malloc(sizeof(*rebuff) * size);
    if (!rebuff) {
        return NULL;
    }
    for (int i=0; i<size; i++){
        snprintf(rebuff[i].renamed_reg, 12, "ROB%d", i);
        rebuff[i].dest = -1;
        rebuff[i].result = -1;
        rebuff[i].head = 0;
        rebuff[i].tail = 0;
        rebuff[i].completed = 1;
    }
    return rebuff;
}

Btb*
create_btb(int size){
    Btb* btb = malloc(sizeof(*btb) * size);
    if (!btb) {
        return NULL;
    }
    for (int i=0; i<size; i++){
        btb[i].tag = -1;
        btb[i].target = -1;
    }
    return btb;
}

Pt*
create_pt(int size){
    Pt* pt = malloc(sizeof(*pt) * size);
    if (!pt) {
        return NULL;
    }
    for (int i=0; i<size; i++){
        pt[i].pattern = 3;
    }
    return pt;
}


/*
 *  The SuperScalar pipeline Implementation
 */ 
void super_scalar(CPU* cpu){
    // The print statement to print logs
    if (print_pipeline != NULL){
        if(atoi(print_pipeline) == 1){
            printf("====================================================================");
            printf("\nClock Cycle #: %d\n",cpu->clock);
        }
    }
    load_the_memory(); // Load Memmory Map into Array
    for(;;){
        // if(writeback_unit(cpu)){
        //     break;
        // }
        reorder1_unit(cpu);
        reorder2_unit(cpu);
        writeback4_unit(cpu);
        writeback3_unit(cpu);
        writeback2_unit(cpu);
        writeback1_unit(cpu);
        adder_unit(cpu);
        multiplier2_unit(cpu);
        divider3_unit(cpu);
        memory4_unit(cpu);
        multiplier1_unit(cpu);
        divider2_unit(cpu);
        memory3_unit(cpu);
        divider1_unit(cpu);
        memory2_unit(cpu);
        memory1_unit(cpu);
        issue_unit(cpu);
        instruction_rename_unit(cpu);
        analysis_unit(cpu); 
        decode_unit(cpu); 
        fetch_unit(cpu);
        reserve_station_buff(cpu);
        print_rebuff(cpu);
        print_proj4_registers(cpu);
        // reorder_buff(cpu);
        clear_forwarding(cpu);
        if (print_pipeline != NULL){ // The print statement to print logs
            if(atoi(print_pipeline) == 1){
                // print_btb(cpu);
                // print_pt(cpu);
            }
        }
        cpu->clock++;
        cpu->fetch_latch.has_inst = 1;  
        // Safty Termination
        if(cpu->clock > 200){
            return;
        }
        if (print_pipeline != NULL){ // The print statement to print logs
            if(atoi(print_pipeline) == 1){
                printf("\n====================================================================");
                printf("\nClock Cycle #: %d\n",cpu->clock);
            }
        }
    }
    if (print_pipeline != NULL){ // The print statement to print logs
        if(atoi(print_pipeline) == 1){
            printf("\n================================\n");
        }
    }
    //TODO Loop Or recursion?
    // Create Struct or F!!!
    // printf("Hahaha");
}

void reorder1_unit(CPU* cpu){
    if (print_pipeline != NULL){ // The print statement to print logs
        if(atoi(print_pipeline) == 1){
            printf("| RE   :  \t\t\t\t\t\t\t\t");
        }
    }
    // printf();
    cpu->regs[atoi(cpu->reorder1_latch.renamed_rg1+3)].value = cpu->rebuff[atoi(cpu->reorder1_latch.renamed_rg1+3)].result;
    cpu->rebuff[atoi(cpu->reorder1_latch.renamed_rg1+3)].result = -1;
    cpu->rebuff[atoi(cpu->reorder1_latch.renamed_rg1+3)].completed = 0;
}

void reorder2_unit(CPU* cpu){
    if (print_pipeline != NULL){ // The print statement to print logs
        if(atoi(print_pipeline) == 1){
            printf("| RE   : \n");
            printf(" ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---\n");
        }
    }
}

void writeback4_unit(CPU* cpu){
    if(cpu->writeback4_latch.has_inst == 1 && cpu->writeback4_latch.halt_triggered==0){
        if(strcmp(cpu->instructions[cpu->writeback4_latch.pc],"") == 0){
            cpu->writeback4_latch = cpu->writeback4_latch;
            return;
        }
        if (print_pipeline != NULL){ // The print statement to print logs
            if(atoi(print_pipeline) == 1){
                printf("| WB   : %s\t\t\t\t\t\t\t\t",cpu->instructions[cpu->writeback4_latch.pc]);
            }
        }
        // Writeback
        cpu->rebuff[cpu->rebuff[0].head].result = cpu->writeback4_latch.buffer;
        cpu->rebuff[cpu->rebuff[0].head].completed = 1;
        increase_head++;
        if(cpu->regs[atoi(cpu->writeback4_latch.rg1+1)].is_issued >= 1){
            cpu->regs[atoi(cpu->writeback4_latch.rg1+1)].is_issued --;
        }
        cpu->reorder1_latch = cpu->writeback4_latch;
    }
    else if(cpu->writeback4_latch.has_inst == 0){
        if (print_pipeline != NULL){ // The print statement to print logs
            if(atoi(print_pipeline) == 1){
                printf("| WB   : \t\t\t\t\t\t\t\t");
            }
        }
    }
}

void writeback3_unit(CPU* cpu){
    if (print_pipeline != NULL){ // The print statement to print logs
        if(atoi(print_pipeline) == 1){
            printf("| WB   : \t\t\t\t\t\t\t\t");
        }
    }
}

void writeback2_unit(CPU* cpu){
    if (print_pipeline != NULL){ // The print statement to print logs
        if(atoi(print_pipeline) == 1){
            printf("| WB   : \t\t\t\t\t\t\t\t");
        }
    }
}


void writeback1_unit(CPU* cpu){
    if (print_pipeline != NULL){ // The print statement to print logs
        if(atoi(print_pipeline) == 1){
            printf("| WB   : \n");
        }
    }
}

void adder_unit(CPU* cpu){
    if(cpu->adder_latch.has_inst == 1 && cpu->adder_latch.halt_triggered==0){
        if(strcmp(cpu->instructions[cpu->adder_latch.pc],"") == 0){
            cpu->writeback4_latch = cpu->adder_latch;
            return;
        }
        if (print_pipeline != NULL){ // The print statement to print logs
            if(atoi(print_pipeline) == 1){
                printf("| Add  : %s\t\t\t\t\t\t\t\t",cpu->instructions[cpu->adder_latch.pc]);
            }
        }
        // Add Sub or Set
        if(strcmp(cpu->adder_latch.opcode,"add") == 0){
            //TODO Write Subtraction Logic
            if (cpu->adder_latch.or1[0] == 82 && cpu->adder_latch.or2[0] == 82){
                    cpu->adder_latch.buffer = cpu->adder_latch.rg2_val + cpu->adder_latch.rg3_val;
            }
            else if (cpu->adder_latch.or1[0] == 82){
                    cpu->adder_latch.buffer = cpu->adder_latch.rg2_val + atoi(cpu->adder_latch.or2+1);
            }
            else if (cpu->adder_latch.or2[0] == 82){
                    cpu->adder_latch.buffer = atoi(cpu->adder_latch.or1+1) + cpu->adder_latch.rg3_val;
            }
            else{
                cpu->adder_latch.buffer = atoi(cpu->adder_latch.or1+1) + atoi(cpu->adder_latch.or2+1);
            }
        }
        else if(strcmp(cpu->adder_latch.opcode,"sub") == 0){
            //TODO Write Subtraction Logic
            if (cpu->adder_latch.or1[0] == 82 && cpu->adder_latch.or2[0] == 82){
                    cpu->adder_latch.buffer = cpu->adder_latch.rg2_val - cpu->adder_latch.rg3_val;
            }
            else if (cpu->adder_latch.or1[0] == 82){
                    cpu->adder_latch.buffer = cpu->adder_latch.rg2_val - atoi(cpu->adder_latch.or2+1);
            }
            else if (cpu->adder_latch.or2[0] == 82){
                    cpu->adder_latch.buffer = atoi(cpu->adder_latch.or1+1) - cpu->adder_latch.rg3_val;
            }
            else{
                cpu->adder_latch.buffer = atoi(cpu->adder_latch.or1+1) - atoi(cpu->adder_latch.or2+1);
            }
        }
        else if(strcmp(cpu->adder_latch.opcode,"set") == 0){
            //TODO Write Set Logic
            if (cpu->adder_latch.or1[0] == 82){
                    cpu->adder_latch.buffer = cpu->adder_latch.rg2_val;
            }
            else{
                cpu->adder_latch.buffer = atoi(cpu->adder_latch.or1+1);
            }  
        }
        else if(strcmp(cpu->adder_latch.opcode,"ld") == 0){
            if (cpu->adder_latch.or1[0] == 82){
                    cpu->adder_latch.rg2_val = cpu->adder_latch.rg2_val;
            }
        }
        cpu->writeback4_latch = cpu->adder_latch;
    }
    else if(cpu->adder_latch.has_inst == 0){
        if (print_pipeline != NULL){ // The print statement to print logs
            if(atoi(print_pipeline) == 1){
                printf("| Add  : \t\t\t\t\t\t\t\t");
            }
        }
        cpu->writeback4_latch = cpu->adder_latch;
    }
}

void multiplier2_unit(CPU* cpu){
    if (print_pipeline != NULL){ // The print statement to print logs
        if(atoi(print_pipeline) == 1){
            printf("| Mul2 : \t\t\t\t\t\t\t\t");
        }
    }
}

void divider3_unit(CPU* cpu){
    if (print_pipeline != NULL){ // The print statement to print logs
        if(atoi(print_pipeline) == 1){
            printf("| DIV3 : \t\t\t\t\t\t\t\t");
        }
    }
}

void memory4_unit(CPU* cpu){
    if (print_pipeline != NULL){ // The print statement to print logs
        if(atoi(print_pipeline) == 1){
            printf("| MEM4 : \n");
        }
    }
}

void multiplier1_unit(CPU* cpu){
    if (print_pipeline != NULL){ // The print statement to print logs
        if(atoi(print_pipeline) == 1){
            printf("|        								| Mul1 : \t\t\t\t\t\t\t\t");
        }
    }
}

void divider2_unit(CPU* cpu){
    if (print_pipeline != NULL){ // The print statement to print logs
        if(atoi(print_pipeline) == 1){
            printf("| DIV2 : \t\t\t\t\t\t\t\t");
        }
    }
} 

void memory3_unit(CPU* cpu){
    if (print_pipeline != NULL){ // The print statement to print logs
        if(atoi(print_pipeline) == 1){
            printf("| MEM3 : \n");
        }
    }
}

void divider1_unit(CPU* cpu){
    if (print_pipeline != NULL){ // The print statement to print logs
        if(atoi(print_pipeline) == 1){
            printf("|        								|        								| DIV1 : \t\t\t\t\t\t\t\t");
        }
    }
} 

void memory2_unit(CPU* cpu){
    if (print_pipeline != NULL){ // The print statement to print logs
        if(atoi(print_pipeline) == 1){
            printf("| MEM2 : \n");
        }
    }
}

void memory1_unit(CPU* cpu){
    if (print_pipeline != NULL){ // The print statement to print logs
        if(atoi(print_pipeline) == 1){
            printf("|        								|        								|        								| MEM1 : \n");
            printf(" ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---\n");
        }
    }
}

void issue_unit(CPU* cpu){
    // Read from the Reserve Station
    if (print_pipeline != NULL){ // The print statement to print logs
        if(atoi(print_pipeline) == 1){
             printf("| IS   ");
            if(cpu->reserve_count == 0){
                printf("| %d %s %s %s %s ",cpu->reserve_station[0].instAddr,cpu->reserve_station[0].opcode,cpu->reserve_station[0].renamed_rg1,cpu->reserve_station[0].or1,cpu->reserve_station[0].or2);
            }
            else{
                for(int i=1;i<=cpu->reserve_count;i++){
                    printf("| %d %d %s %s %s %s ",i,cpu->reserve_station[i].instAddr,cpu->reserve_station[i].opcode,cpu->reserve_station[i].renamed_rg1,cpu->reserve_station[i].or1,cpu->reserve_station[i].or2);
                }
            }
        }
            printf("\n");
    }
    if(cpu->reserve_count < 0){
        cpu->adder_latch = cpu->issue_latch;
    }
    else if(cpu->reserve_count == 0){
        if(strcmp(cpu->reserve_station[0].opcode,"add") == 0){
            cpu->adder_latch = cpu->reserve_station[0];
            cpu->regs[atoi(cpu->reserve_station[0].rg1+1)].is_issued ++;
        }
        else if(strcmp(cpu->reserve_station[0].opcode,"sub") == 0){
            cpu->adder_latch = cpu->reserve_station[0];
            cpu->regs[atoi(cpu->reserve_station[0].rg1+1)].is_issued ++;
        }
        else if(strcmp(cpu->reserve_station[0].opcode,"set") == 0){
            cpu->adder_latch = cpu->reserve_station[0];
            cpu->regs[atoi(cpu->reserve_station[0].rg1+1)].is_issued ++;
        }
        cpu->reserve_count--;
    }
    else{
        for(int i=1;i<=cpu->reserve_count;i++){
            if(strcmp(cpu->reserve_station[i].opcode,"add") == 0){
                cpu->adder_latch = cpu->reserve_station[i];
                cpu->regs[atoi(cpu->reserve_station[i].rg1+1)].is_issued ++;
                cpu->reserve_count--;
                break;
            }
            else if(strcmp(cpu->reserve_station[i].opcode,"sub") == 0){
                cpu->adder_latch = cpu->reserve_station[i];
                cpu->regs[atoi(cpu->reserve_station[i].rg1+1)].is_issued ++;
                cpu->reserve_count--;
                break;
            }
            else if(strcmp(cpu->reserve_station[i].opcode,"set") == 0){
                cpu->adder_latch = cpu->reserve_station[i];
                cpu->regs[atoi(cpu->reserve_station[i].rg1+1)].is_issued ++;
                cpu->reserve_count--;
                break;
            }
        }
    }
}


void instruction_rename_unit(CPU* cpu){
    if(cpu->instruction_rename_latch.has_inst == 1 && cpu->instruction_rename_latch.halt_triggered==0){
        if(strcmp(cpu->instructions[cpu->instruction_rename_latch.pc],"")  == 0){
            // Nothing to store in Reverse Station
            if (print_pipeline != NULL){ // The print statement to print logs
                if(atoi(print_pipeline) == 1){
                    printf("| IR   : \n");
                }
            }            
            return;
        }
        if(strcmp(cpu->instruction_rename_latch.opcode,"ret") == 0){
            // Save ret in Reverse Station
            cpu->reserve_count++;
            cpu->reserve_station[cpu->reserve_count] = cpu->instruction_rename_latch;
            return;
        }
        cpu->instruction_rename_latch.buffer = -1;   //Initialize Buffer Value
        // Read the Register Values
        if (cpu->instruction_rename_latch.or1[0] == 82){   // Check if operand is register?
            strcpy(cpu->instruction_rename_latch.rg2,cpu->instruction_rename_latch.or1);
            cpu->instruction_rename_latch.rg2_val = cpu->regs[atoi(cpu->instruction_rename_latch.or1+1)].value;
            // printf("%s: %d\n",cpu->instruction_rename_latch.rg2,cpu->instruction_rename_latch.rg2_val);
        }
        if (cpu->instruction_rename_latch.or2[0] == 82){  // Check if operand is register?
            strcpy(cpu->instruction_rename_latch.rg3,cpu->instruction_rename_latch.or2);
            cpu->instruction_rename_latch.rg3_val = cpu->regs[atoi(cpu->instruction_rename_latch.or2+1)].value;
            // printf("%s: %d\n",cpu->instruction_rename_latch.rg3,cpu->instruction_rename_latch.rg3_val);
        }
        cpu->instruction_rename_latch.rg1_val = cpu->regs[atoi(cpu->instruction_rename_latch.rg1+1)].value;
        // Rename the Register for OOT Execution
        // cpu->rebuff[cpu->rebuff[0].tail].dest = atoi(cpu->instruction_rename_latch.rg1+1);
        // cpu->regs[cpu->rebuff[0].tail].tag = cpu->rebuff[cpu->rebuff[0].tail].dest;
        // cpu->rebuff[cpu->rebuff[0].tail].completed = 0;
        if(strcmp(cpu->instruction_rename_latch.opcode,"bez") == 0 || strcmp(cpu->instruction_rename_latch.opcode,"bgez") == 0 || strcmp(cpu->instruction_rename_latch.opcode,"blez") == 0 || strcmp(cpu->instruction_rename_latch.opcode,"bgtz") == 0 || strcmp(cpu->instruction_rename_latch.opcode,"bltz") == 0 ){
            // DO something I guess :PPPPPPP
            for(int i=cpu->rebuff[0].head;i<=cpu->rebuff[0].tail;i++){
                if(atoi(cpu->instruction_rename_latch.rg1+1) == cpu->rebuff[i].dest){
                    strcpy(cpu->instruction_rename_latch.renamed_rg1,cpu->rebuff[cpu->rebuff[0].tail].renamed_reg);
                }
            }
        }
        else{
            cpu->rebuff[cpu->rebuff[0].tail].dest = atoi(cpu->instruction_rename_latch.rg1+1);
            cpu->regs[cpu->rebuff[0].tail].tag = cpu->rebuff[cpu->rebuff[0].tail].dest;
            cpu->rebuff[cpu->rebuff[0].tail].completed = 0;
            strcpy(cpu->instruction_rename_latch.renamed_rg1,cpu->rebuff[cpu->rebuff[0].tail].renamed_reg);
            if(cpu->rebuff[0].tail == 7){
                cpu->rebuff[0].tail = 0;    
            }
            else{
                cpu->rebuff[0].tail++;
            }
        }
        if(strcmp(cpu->instruction_rename_latch.opcode,"bez") == 0){
            if(cpu->regs[atoi(cpu->instruction_rename_latch.rg1+1)].is_issued > 0){
                cpu->analysis_latch.branch_stall = 1;
                cpu->decode_latch.branch_stall = 1;
                cpu->fetch_latch.branch_stall = 1;
                return;
            }
            if(cpu->instruction_rename_latch.rg1_val == 0){
                if(cpu->instruction_rename_latch.branch_taken == 1){
                    // printf("Branch Already Taken");
                    if(cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern < 7){
                        cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern++;
                    }
                }
                else{
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].tag = get_tag(cpu->instruction_rename_latch.instAddr);
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].target = atoi(cpu->instruction_rename_latch.or1+1);
                    if(cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern < 7){
                        cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern++;
                    }
                    cpu->pc = cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].target/4;
                    cpu->fetch_latch.has_inst = 0;
                    cpu->decode_latch.has_inst = 0;
                    cpu->analysis_latch.has_inst = 0;
                    cpu->instruction_rename_latch.has_inst = 0;
                    if(cpu->instruction_rename_latch.halt_triggered == 1 || cpu->fetch_latch.halt_triggered == 1){
                        cpu->hazard++;
                        cpu->adder_latch.halt_triggered = 0;
                        cpu->instruction_rename_latch.halt_triggered = 0;
                        cpu->analysis_latch.halt_triggered = 0;
                        cpu->decode_latch.halt_triggered = 0;
                        cpu->fetch_latch.halt_triggered = 0;
                    }
                    for (int i=0; i<16; i++){
                        cpu->regs[i].is_writing = 0;
                    }
                    return;
                }
            }
            else{
                if(cpu->instruction_rename_latch.branch_taken == 1){
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].tag = get_tag(cpu->instruction_rename_latch.instAddr);
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].target = atoi(cpu->instruction_rename_latch.or1+1);
                    if(cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern > 0){
                        cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern--;
                    }
                    cpu->pc = (cpu->instruction_rename_latch.instAddr+4)/4;
                    cpu->reverse_branch = 1;
                    cpu->fetch_latch.has_inst = 0;
                    cpu->decode_latch.has_inst = 0;
                    cpu->analysis_latch.has_inst = 0;
                    cpu->instruction_rename_latch.has_inst = 0;
                    if(cpu->instruction_rename_latch.halt_triggered == 1 || cpu->fetch_latch.halt_triggered == 1){
                        cpu->hazard++;
                        cpu->adder_latch.halt_triggered = 0;
                        cpu->instruction_rename_latch.halt_triggered = 0;
                        cpu->analysis_latch.halt_triggered = 0;
                        cpu->decode_latch.halt_triggered = 0;
                        cpu->fetch_latch.halt_triggered = 0;
                    }
                    for (int i=0; i<16; i++){
                        cpu->regs[i].is_writing = 0;
                    }
                    // printf("bez not taken but should be Taken");
                    return;
                }
                else{
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].tag = get_tag(cpu->instruction_rename_latch.instAddr);
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].target = atoi(cpu->instruction_rename_latch.or1+1);
                    if(cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern > 0){
                        cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern--;
                    }
                    // printf("bez branch not taken");
                }
            }
        }
        else if(strcmp(cpu->instruction_rename_latch.opcode,"bgez") == 0){
            if(cpu->regs[atoi(cpu->instruction_rename_latch.rg1+1)].is_issued > 0){
                cpu->analysis_latch.branch_stall = 1;
                cpu->decode_latch.branch_stall = 1;
                cpu->fetch_latch.branch_stall = 1;
                return;
            }
            else if(cpu->instruction_rename_latch.rg1_val >= 0){
                cpu->pc = atoi(cpu->instruction_rename_latch.or1+1)/4;
                cpu->fetch_latch.has_inst = 0;
                cpu->decode_latch.has_inst = 0;
                cpu->analysis_latch.has_inst = 0;
                cpu->instruction_rename_latch.has_inst = 0;
                cpu->rebuff[cpu->rebuff[0].tail].dest = atoi(cpu->instruction_rename_latch.rg1+1);
                return;
            }
            else{
                // Do nothing!!!!!!!!!
            }
        }
        else if(strcmp(cpu->instruction_rename_latch.opcode,"blez") == 0){
            if(cpu->regs[atoi(cpu->instruction_rename_latch.rg1+1)].is_issued > 0){
                cpu->analysis_latch.branch_stall = 1;
                cpu->decode_latch.branch_stall = 1;
                cpu->fetch_latch.branch_stall = 1;
                return;
            }
            if(cpu->instruction_rename_latch.rg1_val <= 0){
                if(cpu->instruction_rename_latch.branch_taken == 1){
                    // printf("Branch Already Taken");
                    if(cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern < 7){
                        cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern++;
                    }
                }
                else{
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].tag = get_tag(cpu->instruction_rename_latch.instAddr);
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].target = atoi(cpu->instruction_rename_latch.or1+1);
                    if(cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern < 7){
                        cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern++;
                    }
                    cpu->pc = cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].target/4;
                    cpu->fetch_latch.has_inst = 0;
                    cpu->decode_latch.has_inst = 0;
                    cpu->analysis_latch.has_inst = 0;
                    cpu->instruction_rename_latch.has_inst = 0;
                    if(cpu->instruction_rename_latch.halt_triggered == 1 || cpu->fetch_latch.halt_triggered == 1){
                        cpu->hazard++;
                        cpu->adder_latch.halt_triggered = 0;
                        cpu->instruction_rename_latch.halt_triggered = 0;
                        cpu->analysis_latch.halt_triggered = 0;
                        cpu->decode_latch.halt_triggered = 0;
                        cpu->fetch_latch.halt_triggered = 0;
                    }
                    for (int i=0; i<16; i++){
                        cpu->regs[i].is_writing = 0;
                    }
                    return;
                }
            }
            else{
                if(cpu->instruction_rename_latch.branch_taken == 1){
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].tag = get_tag(cpu->instruction_rename_latch.instAddr);
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].target = atoi(cpu->instruction_rename_latch.or1+1);
                    if(cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern > 0){
                        cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern--;
                    }
                    cpu->pc = (cpu->instruction_rename_latch.instAddr+4)/4;
                    cpu->reverse_branch = 1;
                    cpu->fetch_latch.has_inst = 0;
                    cpu->decode_latch.has_inst = 0;
                    cpu->analysis_latch.has_inst = 0;
                    cpu->instruction_rename_latch.has_inst = 0;
                    if(cpu->instruction_rename_latch.halt_triggered == 1 || cpu->fetch_latch.halt_triggered == 1){
                        cpu->hazard++;
                        cpu->adder_latch.halt_triggered = 0;
                        cpu->instruction_rename_latch.halt_triggered = 0;
                        cpu->analysis_latch.halt_triggered = 0;
                        cpu->decode_latch.halt_triggered = 0;
                        cpu->fetch_latch.halt_triggered = 0;
                    }
                    for (int i=0; i<16; i++){
                        cpu->regs[i].is_writing = 0;
                    }
                    // printf("bez not taken but should be Taken");
                    return;
                }
                else{
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].tag = get_tag(cpu->instruction_rename_latch.instAddr);
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].target = atoi(cpu->instruction_rename_latch.or1+1);
                    if(cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern > 0){
                        cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern--;
                    }
                    // printf("bez branch not taken");
                }
            }
        }
        else if(strcmp(cpu->instruction_rename_latch.opcode,"bgtz") == 0){
            if(cpu->regs[atoi(cpu->instruction_rename_latch.rg1+1)].is_issued > 0){
                cpu->analysis_latch.branch_stall = 1;
                cpu->decode_latch.branch_stall = 1;
                cpu->fetch_latch.branch_stall = 1;
                return;
            }
            if(cpu->instruction_rename_latch.rg1_val > 0){
                if(cpu->instruction_rename_latch.branch_taken == 1){
                    // printf("Branch Already Taken");
                    if(cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern < 7){
                        cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern++;
                    }
                }
                else{
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].tag = get_tag(cpu->instruction_rename_latch.instAddr);
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].target = atoi(cpu->instruction_rename_latch.or1+1);
                    if(cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern < 7){
                        cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern++;
                    }
                    cpu->pc = cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].target/4;
                    cpu->fetch_latch.has_inst = 0;
                    cpu->decode_latch.has_inst = 0;
                    cpu->analysis_latch.has_inst = 0;
                    cpu->instruction_rename_latch.has_inst = 0;
                    if(cpu->instruction_rename_latch.halt_triggered == 1 || cpu->fetch_latch.halt_triggered == 1){
                        cpu->hazard++;
                        cpu->adder_latch.halt_triggered = 0;
                        cpu->instruction_rename_latch.halt_triggered = 0;
                        cpu->analysis_latch.halt_triggered = 0;
                        cpu->decode_latch.halt_triggered = 0;
                        cpu->fetch_latch.halt_triggered = 0;
                    }
                    for (int i=0; i<16; i++){
                        cpu->regs[i].is_writing = 0;
                    }
                    return;
                }
            }
            else{
                if(cpu->instruction_rename_latch.branch_taken == 1){
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].tag = get_tag(cpu->instruction_rename_latch.instAddr);
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].target = atoi(cpu->instruction_rename_latch.or1+1);
                    if(cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern > 0){
                        cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern--;
                    }
                    cpu->pc = (cpu->instruction_rename_latch.instAddr+4)/4;
                    cpu->reverse_branch = 1;
                    cpu->fetch_latch.has_inst = 0;
                    cpu->decode_latch.has_inst = 0;
                    cpu->analysis_latch.has_inst = 0;
                    cpu->instruction_rename_latch.has_inst = 0;
                    if(cpu->instruction_rename_latch.halt_triggered == 1 || cpu->fetch_latch.halt_triggered == 1){
                        cpu->hazard++;
                        cpu->adder_latch.halt_triggered = 0;
                        cpu->instruction_rename_latch.halt_triggered = 0;
                        cpu->analysis_latch.halt_triggered = 0;
                        cpu->decode_latch.halt_triggered = 0;
                        cpu->fetch_latch.halt_triggered = 0;
                    }
                    for (int i=0; i<16; i++){
                        cpu->regs[i].is_writing = 0;
                    }
                    // printf("bez not taken but should be Taken");
                    return;
                }
                else{
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].tag = get_tag(cpu->instruction_rename_latch.instAddr);
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].target = atoi(cpu->instruction_rename_latch.or1+1);
                    if(cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern > 0){
                        cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern--;
                    }
                    // printf("bez branch not taken");
                }
            }
        }
        else if(strcmp(cpu->instruction_rename_latch.opcode,"bltz") == 0){
            if(cpu->instruction_rename_latch.rg1_val < 0){
                if(cpu->instruction_rename_latch.branch_taken == 1){
                    // printf("Branch Already Taken");
                    if(cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern < 7){
                        cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern++;
                    }
                }
                else{
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].tag = get_tag(cpu->instruction_rename_latch.instAddr);
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].target = atoi(cpu->instruction_rename_latch.or1+1);
                    if(cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern < 7){
                        cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern++;
                    }
                    cpu->pc = cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].target/4;
                    cpu->fetch_latch.has_inst = 0;
                    cpu->decode_latch.has_inst = 0;
                    cpu->analysis_latch.has_inst = 0;
                    cpu->instruction_rename_latch.has_inst = 0;
                    if(cpu->instruction_rename_latch.halt_triggered == 1 || cpu->fetch_latch.halt_triggered == 1){
                        cpu->hazard++;
                        cpu->adder_latch.halt_triggered = 0;
                        cpu->instruction_rename_latch.halt_triggered = 0;
                        cpu->analysis_latch.halt_triggered = 0;
                        cpu->decode_latch.halt_triggered = 0;
                        cpu->fetch_latch.halt_triggered = 0;
                    }
                    for (int i=0; i<16; i++){
                        cpu->regs[i].is_writing = 0;
                    }
                    return;
                }
            }
            else{
                if(cpu->instruction_rename_latch.branch_taken == 1){
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].tag = get_tag(cpu->instruction_rename_latch.instAddr);
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].target = atoi(cpu->instruction_rename_latch.or1+1);
                    if(cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern > 0){
                        cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern--;
                    }
                    cpu->pc = (cpu->instruction_rename_latch.instAddr+4)/4;
                    cpu->reverse_branch = 1;
                    cpu->fetch_latch.has_inst = 0;
                    cpu->decode_latch.has_inst = 0;
                    cpu->analysis_latch.has_inst = 0;
                    cpu->instruction_rename_latch.has_inst = 0;
                    if(cpu->instruction_rename_latch.halt_triggered == 1 || cpu->fetch_latch.halt_triggered == 1){
                        cpu->hazard++;
                        cpu->adder_latch.halt_triggered = 0;
                        cpu->instruction_rename_latch.halt_triggered = 0;
                        cpu->analysis_latch.halt_triggered = 0;
                        cpu->decode_latch.halt_triggered = 0;
                        cpu->fetch_latch.halt_triggered = 0;
                    }
                    for (int i=0; i<16; i++){
                        cpu->regs[i].is_writing = 0;
                    }
                    // printf("bez not taken but should be Taken");
                    return;
                }
                else{
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].tag = get_tag(cpu->instruction_rename_latch.instAddr);
                    cpu->btb[(cpu->instruction_rename_latch.instAddr/4)%16].target = atoi(cpu->instruction_rename_latch.or1+1);
                    if(cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern > 0){
                        cpu->pt[(cpu->instruction_rename_latch.instAddr/4)%16].pattern--;
                    }
                    // printf("bez branch not taken");
                }
            }
        }
        if (print_pipeline != NULL){ // The print statement to print logs
            if(atoi(print_pipeline) == 1){
                printf("| IR   : %d %s %s %s %s\n",cpu->instruction_rename_latch.instAddr,cpu->instruction_rename_latch.opcode,cpu->instruction_rename_latch.renamed_rg1,cpu->instruction_rename_latch.or1,cpu->instruction_rename_latch.or2);
                // printf("| IR   : %s\n",cpu->instructions[cpu->instruction_rename_latch.pc]);
            }
        }
        cpu->reserve_count++;
        cpu->reserve_station[cpu->reserve_count] = cpu->instruction_rename_latch;
    }
    else if (cpu->instruction_rename_latch.has_inst == 1 && cpu->instruction_rename_latch.halt_triggered==1){
        if (print_pipeline != NULL){ // The print statement to print logs
            if(atoi(print_pipeline) == 1){
                printf("| IR   : %d %s %s %s %s ",cpu->instruction_rename_latch.instAddr,cpu->instruction_rename_latch.opcode,cpu->instruction_rename_latch.renamed_rg1,cpu->instruction_rename_latch.or1,cpu->instruction_rename_latch.or2);
                printf("| IR   : %s\n",cpu->instructions[cpu->instruction_rename_latch.pc]);
            }
        }
    }
    else if(cpu->instruction_rename_latch.has_inst == 0){
        if (print_pipeline != NULL){ // The print statement to print logs
            if(atoi(print_pipeline) == 1){
                printf("| IR   : \n");
            }
        }
    }
}

void analysis_unit(CPU* cpu){
    if(cpu->analysis_latch.has_inst == 1 && cpu->analysis_latch.branch_stall==1){
        if (print_pipeline != NULL){ // The print statement to print logs
            if(atoi(print_pipeline) == 1){
                printf("| IA   : %s\n",cpu->instructions[cpu->analysis_latch.pc]);
            }
        }
        cpu->analysis_latch.branch_stall=0;
        return;
    }
    else if(cpu->analysis_latch.has_inst == 1 && cpu->analysis_latch.halt_triggered==0){
        if(strcmp(cpu->instructions[cpu->analysis_latch.pc],"") == 0){
            if (print_pipeline != NULL){ // The print statement to print logs
               if(atoi(print_pipeline) == 1){
                    printf("| IA   : \n");
                }
            }
            cpu->instruction_rename_latch = cpu->analysis_latch;
            return;
        }
        if (print_pipeline != NULL){ // The print statement to print logs
            if(atoi(print_pipeline) == 1){
                printf("| IA   : %s\n",cpu->instructions[cpu->analysis_latch.pc]);
            }
        }
        if(strcmp(cpu->analysis_latch.opcode,"ret") == 0){
            cpu->instruction_rename_latch=cpu->analysis_latch;
            return;
        }
        cpu->instruction_rename_latch=cpu->analysis_latch;
    }
    else if(cpu->analysis_latch.has_inst == 1 && cpu->analysis_latch.halt_triggered==1){
        if (print_pipeline != NULL){ // The print statement to print logs
            if(atoi(print_pipeline) == 1){
                printf("| IA   : %s\n",cpu->instructions[cpu->analysis_latch.pc]);
            }
        }
    }
}

void decode_unit(CPU* cpu){
    if(cpu->decode_latch.has_inst == 1 && cpu->decode_latch.branch_stall==1){
        if (print_pipeline != NULL){ // The print statement to print logs
            if(atoi(print_pipeline) == 1){
                printf("| ID   : %s\n",cpu->instructions[cpu->decode_latch.pc]);
            }
        }
        cpu->decode_latch.branch_stall=0;
        return;
    }
    else if(cpu->decode_latch.has_inst == 1 && cpu->decode_latch.halt_triggered==0){
        if(strcmp(cpu->instructions[cpu->decode_latch.pc],"")  == 0){
            if (print_pipeline != NULL){ // The print statement to print logs
                if(atoi(print_pipeline) == 1){
                    printf("| ID   : %s\n",cpu->instructions[cpu->decode_latch.pc]);
                }
            }
            cpu->analysis_latch = cpu->decode_latch;
            return;
        }
        if (print_pipeline != NULL){ // The print statement to print logs
            if(atoi(print_pipeline) == 1){
                printf("| ID   : %s\n",cpu->instructions[cpu->decode_latch.pc]);
            }
        }
        // if(strcmp(cpu->decode_latch.opcode,"ret") == 0){
        //     cpu->analysis_latch = cpu->decode_latch;
        //     return;
        // }
        cpu->analysis_latch = cpu->decode_latch;
    }
    else if (cpu->decode_latch.has_inst == 1 && cpu->decode_latch.halt_triggered==1){
        if (print_pipeline != NULL){ // The print statement to print logs
            if(atoi(print_pipeline) == 1){
                printf("| ID   : %s\n",cpu->instructions[cpu->decode_latch.pc]);
            }
        }
    }
}

void fetch_unit(CPU* cpu){
    if(cpu->fetch_latch.has_inst == 1 && cpu->fetch_latch.branch_stall == 1){
        if (print_pipeline != NULL){ // The print statement to print logs
            if(atoi(print_pipeline) == 1){
                cpu->instructions[cpu->fetch_latch.pc][strcspn(cpu->instructions[cpu->fetch_latch.pc], "\r\t\n")] = 0;
                printf("| IF(s)   : %s\n",cpu->instructions[cpu->fetch_latch.pc]);
                printf(" ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---\n");
            }
        }
        cpu->fetch_latch.branch_stall = 0;
        return;
    }
    if(cpu->fetch_latch.has_inst == 1 && cpu->fetch_latch.halt_triggered == 0){

        cpu->fetch_latch.pc = cpu->pc;
        cpu->pc++;
        char str1[128];
        if(strcmp(cpu->instructions[cpu->fetch_latch.pc],"") == 0){
            cpu->decode_latch = cpu->fetch_latch;
            return;
        }
        if (print_pipeline != NULL){ // The print statement to print logs
            if(atoi(print_pipeline) == 1){
                cpu->instructions[cpu->fetch_latch.pc][strcspn(cpu->instructions[cpu->fetch_latch.pc], "\r\t\n")] = 0;
                printf("| IF   : %s\n",cpu->instructions[cpu->fetch_latch.pc]);
                printf(" ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---\n");
            }
        }
        strcpy(str1,cpu->instructions[cpu->fetch_latch.pc]);

        //-----------------------------Dynamic Spliting---------------------------------------------
        char **token= NULL;
        char *p = str1;
        char *sepa=" ";
        size_t  arr_len = 0,q;
        for (;;)
        {
            p += strspn(p, sepa);
            if (!(q = strcspn(p, sepa)))
                    break;
            if (q)
            {
                    token = realloc(token, (arr_len+1) * sizeof(char *));
                    token[arr_len] = malloc(q+1);
                    strncpy(token[arr_len], p, q);
                    token[arr_len][q] = 0;
                    arr_len++;
                    p += q;
            }
        }
        token = realloc(token, (arr_len+1) * sizeof(char *));
        token[arr_len] = NULL;
        cpu->fetch_latch.instLen = arr_len;
        //-------------------------------------Dynamic Spliting------------------------------------------
        if(arr_len > 4 ){
            cpu->fetch_latch.instAddr = (atoi)(token[0]);
            strcpy(cpu->fetch_latch.opcode,token[1]);
            cpu->fetch_latch.opcode[strcspn(cpu->fetch_latch.opcode, "\r\t\n")] = 0;
            strcpy(cpu->fetch_latch.rg1,token[2]);
            cpu->fetch_latch.rg1[strcspn(cpu->fetch_latch.rg1, "\r\t\n")] = 0;
            strcpy(cpu->fetch_latch.or1,token[3]);
            cpu->fetch_latch.or1[strcspn(cpu->fetch_latch.or1, "\r\t\n")] = 0;
            strcpy(cpu->fetch_latch.or2,token[4]);
            cpu->fetch_latch.or2[strcspn(cpu->fetch_latch.or2, "\r\t\n")] = 0;
        }
        else if(arr_len == 4){
            cpu->fetch_latch.instAddr = (atoi)(token[0]);
            strcpy(cpu->fetch_latch.opcode,token[1]);
            cpu->fetch_latch.opcode[strcspn(cpu->fetch_latch.opcode, "\r\t\n")] = 0;
            strcpy(cpu->fetch_latch.rg1,token[2]);
            cpu->fetch_latch.rg1[strcspn(cpu->fetch_latch.rg1, "\r\t\n")] = 0;
            strcpy(cpu->fetch_latch.or1,token[3]);
            cpu->fetch_latch.or1[strcspn(cpu->fetch_latch.or1, "\r\t\n")] = 0; 
        }
        else if(arr_len == 3){
            cpu->fetch_latch.instAddr = (atoi)(token[0]);
            strcpy(cpu->fetch_latch.opcode,token[1]);
            cpu->fetch_latch.opcode[strcspn(cpu->fetch_latch.opcode, "\r\t\n")] = 0;
            strcpy(cpu->fetch_latch.rg1,token[2]); 
            cpu->fetch_latch.rg1[strcspn(cpu->fetch_latch.rg1, "\r\t\n")] = 0;
        }
        else if(arr_len == 2){
            cpu->fetch_latch.instAddr = (atoi)(token[0]);
            strcpy(cpu->fetch_latch.opcode,token[1]);
            cpu->fetch_latch.opcode[strcspn(cpu->fetch_latch.opcode, "\r\t\n")] = 0;
        }
        cpu->instructions[cpu->fetch_latch.pc][strcspn(cpu->instructions[cpu->fetch_latch.pc], "\r\t\n")] = 0;
        // printf("Reverse: %d", cpu->reverse_branch);
        // Do not Branch using BTB for this cycle
        if(cpu->reverse_branch == 1){
            cpu->reverse_branch = 0;
            cpu->decode_latch = cpu->fetch_latch;
            return;
        }
        // implement BTB and prediction table
        if(strcmp(cpu->fetch_latch.opcode,"bez") == 0){
            if(cpu->btb[(cpu->fetch_latch.instAddr/4)%16].tag == get_tag(cpu->fetch_latch.instAddr)){
                if(cpu->pt[(cpu->fetch_latch.instAddr/4)%16].pattern > 3){
                    cpu->pc = (cpu->btb[(cpu->fetch_latch.instAddr/4)%16].target) / 4;
                    cpu->fetch_latch.branch_taken = 1;
                }
                else{
                    cpu->fetch_latch.branch_taken = 0;
                }
            }
            else{
                cpu->fetch_latch.branch_taken = 0;
            }
        }
        if(strcmp(cpu->fetch_latch.opcode,"bgez") == 0){
            if(cpu->btb[(cpu->fetch_latch.instAddr/4)%16].tag == get_tag(cpu->fetch_latch.instAddr)){
                if(cpu->pt[(cpu->fetch_latch.instAddr/4)%16].pattern > 3){
                    cpu->pc = (cpu->btb[(cpu->fetch_latch.instAddr/4)%16].target) / 4;
                    cpu->fetch_latch.branch_taken = 1;
                } 
                else{
                    cpu->fetch_latch.branch_taken = 0;
                }
            }
            else{
                cpu->fetch_latch.branch_taken = 0;
            }
        }
        else if(strcmp(cpu->fetch_latch.opcode,"blez") == 0){
            if(cpu->btb[(cpu->fetch_latch.instAddr/4)%16].tag == get_tag(cpu->fetch_latch.instAddr)){
                if(cpu->pt[(cpu->fetch_latch.instAddr/4)%16].pattern > 3){
                    cpu->pc = (cpu->btb[(cpu->fetch_latch.instAddr/4)%16].target) / 4;
                    cpu->fetch_latch.branch_taken = 1;
                }
                else{
                    cpu->fetch_latch.branch_taken = 0;
                }
            }
            else{
                cpu->fetch_latch.branch_taken = 0;
            }
        }
        else if(strcmp(cpu->fetch_latch.opcode,"bgtz") == 0){
            if(cpu->btb[(cpu->fetch_latch.instAddr/4)%16].tag == get_tag(cpu->fetch_latch.instAddr)){
                if(cpu->pt[(cpu->fetch_latch.instAddr/4)%16].pattern > 3){
                    cpu->pc = (cpu->btb[(cpu->fetch_latch.instAddr/4)%16].target) / 4;
                    cpu->fetch_latch.branch_taken = 1;
                }
                else{
                    cpu->fetch_latch.branch_taken = 0;
                }
            }
            else{
                cpu->fetch_latch.branch_taken = 0;
            }
        }
        else if(strcmp(cpu->fetch_latch.opcode,"bltz") == 0){
            if(cpu->btb[(cpu->fetch_latch.instAddr/4)%16].tag == get_tag(cpu->fetch_latch.instAddr)){
                if(cpu->pt[(cpu->fetch_latch.instAddr/4)%16].pattern > 3){
                    cpu->pc = (cpu->btb[(cpu->fetch_latch.instAddr/4)%16].target) / 4;
                    cpu->fetch_latch.branch_taken = 1;
                }
                else{
                    cpu->fetch_latch.branch_taken = 0;
                }
            }
            else{
                cpu->fetch_latch.branch_taken = 0;
            }
        }

        // BTB implementation ended
        cpu->decode_latch = cpu->fetch_latch;
    }
    else if (cpu->fetch_latch.has_inst == 1 && cpu->fetch_latch.halt_triggered == 1){
        if (print_pipeline != NULL){ // The print statement to print logs
            if(atoi(print_pipeline) == 1){
                printf("| IF(s): %s",cpu->instructions[cpu->fetch_latch.pc+1]);
            }
        }
    }
}

/*
 *  Clean the pIpelIne of forwarding at end of cycle :P
 */


void clear_forwarding(CPU* cpu){
    strcpy(cpu->add_reg,"NULL");
    strcpy(cpu->mul_reg,"NULL");
    strcpy(cpu->div_reg,"NULL");
    strcpy(cpu->br_reg,"NULL");
    strcpy(cpu->mem1_reg,"NULL");
    strcpy(cpu->freedit,"NULL");
    for (int i=0; i<16; i++){
        cpu->regs[i].freed_this_cycle = 0;
    }
    if(increase_head > 0){
        for(int j=0;j<increase_head;j++){
            cpu->rebuff[cpu->rebuff[0].head].dest = -1;
            if(cpu->rebuff[0].head == 7){
                cpu->rebuff[0].head = 0;    
            }
            else{
                cpu->rebuff[0].head++;
            }
        }
        increase_head = 0;
    }
}

void reserve_station_buff(CPU* cpu){
    if (print_pipeline != NULL){ // The print statement to print logs
        if(atoi(print_pipeline) == 1){
             printf("------------ Reserve Station ----------\n");
             if(cpu->reserve_count == 0){
                printf("|  %s %s %s %s ",cpu->reserve_station[0].opcode,cpu->reserve_station[0].rg1,cpu->reserve_station[0].or1,cpu->reserve_station[0].or2);
             }
             else{
                for(int i=1;i<=cpu->reserve_count;i++){;
                    if(strcmp(cpu->reserve_station[i].opcode,"none")!=0){
                        printf("| %d %s %s %s %s ",i,cpu->reserve_station[i].opcode,cpu->reserve_station[i].rg1,cpu->reserve_station[i].or1,cpu->reserve_station[i].or2);
                    }
                //     printf("%s %s | ",cpu->reserve_station[i].instAddr,cpu->reserve_station[i].opcode);
                }
             }
            printf("\n");
        }
    }
}