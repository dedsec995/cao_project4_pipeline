//
//  main.c
//  Pipeline
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"

int binary_flag;

void run_cpu_fun(char* filename, char* option){

    CPU *cpu = CPU_init(filename,option);
    CPU_run(cpu);
    CPU_stop(cpu);
}

int main(int argc, const char * argv[]) {
    if (argc<=1) {
        fprintf(stderr, "Error : missing required args\n");
        return -1;
    }
    char* filename = (char*)argv[1];
    char* option = (char*)argv[2];
    run_cpu_fun(filename,option);
    
    return 0;
}
