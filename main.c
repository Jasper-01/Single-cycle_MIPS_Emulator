#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define instruction_size 32
#define opcode_size 6
#define opcode_start 0
#define register_size 5  // rs, rt, rd
#define rs_start 6
#define rt_start 11
#define rd_start 16
#define shamt_size 5  // used in R instructions
#define shamt_start 21
#define funct_size 6  // used in R instructions
#define funct_start 26
#define immediate_size 16  // used in I instructions
#define immediate_start 16
#define address_size 26  // used in J instructions
#define address_start 6

#define register_total 32

#define size_32bit 4
#define input_file_1 "input_1.bin"
#define input_file_2 "input_2.bin"

void read_file();
void read_instructions(FILE * fp, int total_instruction);
int file_len(FILE * fp);
int get_num_instructions(int file_size);
int decode_instructions(int* instructions, int* reg_arr, int PC);
int* get_stuff(int* instructions_bin, int length, int start);
int bin2dec(int* input, int size);
int regular_PC_update(int PC);
int branch_PC_update(int PC, int immediate);
void print_R_Instructions(int opcode, int rs, int rt, int* reg_arr, int shamt, int funct);
void print_I_Instructions(int opcode, int rs, int rt, int* reg_arr, int immediate);
void print_J_Instructions(int opcode, int address);
// R instructions -- always 0 in front, important info at funct
int get_R_instruction_information(int instructions, int* instructions_bin, int opcode, int* reg_arr, int PC);
// I instructions -- the majority
int get_I_instruction_information(int instructions, int* instructions_bin, int opcode, int* reg_arr, int PC);
// J instructions -- only j and jal
int get_J_instruction_information(int instructions, int* instructions_bin, int opcode, int PC);

int main() {
    read_file();
    return 0;
    // Single-cycle_MIPS_Emulator
}

void read_file() {
    FILE *input1;
    fopen_s(&input1, input_file_1, "r");
    FILE *input2;
    fopen_s(&input2, input_file_2, "r");

    if (input1 == NULL) {
        printf("ERROR has occurred in input file 1.\n");
        exit(2);
    } else if (input2 == NULL) {
        printf("ERROR has occurred in input file 2.\n");
        exit(2);
    }

    const int total_instructions = get_num_instructions(file_len(input1));
    read_instructions(input1, total_instructions);

    fclose(input1);
    fclose(input2);
}

void read_instructions(FILE * fp, const int total_instruction){
    int reg_arr[register_total] = {0};
    reg_arr[31] = 0xffffffff;
    reg_arr[29] = 0x10000000;

    int PC = 0;
    // bin_file[total_instruction][instruction_size]
    int** bin_file = (int**)malloc(total_instruction * sizeof(int*));
    for(int i = 0; i < total_instruction; i++){
        bin_file[i] = (int*)malloc(instruction_size * sizeof(int));
    }

    for(int i = 0; i < total_instruction; i++){
        int instructions_bin[instruction_size] = {0};
        for(int j = 0; j < 4; j++){
            char ch = getc(fp);
            int sample = (int) ch;
            if(sample < 0){
                sample += 256 ;
            }
            for(int k = 7; k >= 0; k--){
                int index = (j * 8) + k;
                instructions_bin[index] = sample % 2;
                if(instructions_bin[index] == 1){
                    sample -= 1;
                }
                sample = sample / 2;
            }
        }

        for(int j = 0; j < instruction_size; j++){
            bin_file[i][j] = instructions_bin[j];
        }

//        printf("Cycle[%d] (PC: 0x%0.2x)\n", i, PC);
//        PC = decode_instructions(instructions_bin, reg_arr, PC);
    }

//    while(PC != 0xFFFFFFFF){
//        PC = decode_instructions(bin_file, reg_arr, PC);
//        break;
//    }
}

int file_len(FILE * fp){
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    rewind(fp);

    return size;
}

int get_num_instructions(int file_size) { return (file_size / size_32bit); }

int decode_instructions(int* instructions_bin, int* reg_arr, int PC){
    int instructions = bin2dec(instructions_bin, instruction_size);
    int* opcode_bin = get_stuff(instructions_bin, opcode_size, opcode_start);
    int opcode = bin2dec(opcode_bin, opcode_size);

    printf("[Fetch Instruction] 0x%0.8x\n", instructions);

    switch(opcode){
        case 0:
            PC = get_R_instruction_information(instructions, instructions_bin, opcode, reg_arr, PC);
            break;
        case 2:
        case 3:
            PC = get_J_instruction_information(instructions, instructions_bin, opcode, PC);
            break;
        default:
            PC = get_I_instruction_information(instructions, instructions_bin, opcode, reg_arr, PC);
    }
    return PC;
}

int* get_stuff(int* instructions, int length, int start){
    int* temp = (int*)malloc(length* sizeof(int));
    for(int i = 0; i < length; i++){
        temp[i] = instructions[start+i];
    }
    return temp;
}

int regular_PC_update(int PC){
    printf("[PC Update]: PC <- 0x%0.8x + 4\n\n", PC);
    PC += 4;
    return PC;
}

int branch_PC_update(int PC, int immediate){
    printf("[PC Update]: PC <- 0x0.8x + 4 + 0x%0.2x\n\n", PC, immediate);
    PC = PC + 4 + immediate;
    return PC;
}

int bin2dec(int* input, int size){
    int temp = 0;
    for(int i = 0; i < size; i++) {
        if (input[i] == 1) {
            float power = (float) ((size - i - 1) * input[i]);
            temp += (int) pow(2.0, power);
        }
    }
    return temp;
}

void print_R_Instructions(int opcode, int rs, int rt, int* reg_arr, int shamt, int funct){
    printf("opcode: 0x%x, rs: 0x%x (R[%d]=0x%x), rt: 0x%x (R[%d]=0x%x), shamt: 0x%x, funct: 0x%x\n", opcode, rs, rs, reg_arr[rs], rt, rt, reg_arr[rt], shamt, funct);
}

void print_I_Instructions(int opcode, int rs, int rt, int* reg_arr, int immediate){
    printf("opcode: 0x%x, rs: 0x%x (R[%d]=0x%x), rt: 0x%x (R[%d]=0x%x), imm: 0x%x\n", opcode, rs, rs, reg_arr[rs], rt, rt, reg_arr[rt], immediate);
}

void print_J_Instructions(int opcode, int address){
    printf("opcode: 0x%x, address: 0x%x\n", opcode, address);
}

// R functions
int get_R_instruction_information(int instructions, int* instructions_bin, int opcode, int* reg_arr, int PC){
    printf("[Decode Instruction] type: R\n");
    int rs = bin2dec(get_stuff(instructions_bin, register_size, rs_start), register_size);
    int rt = bin2dec(get_stuff(instructions_bin, register_size, rt_start), register_size);
    int rd = bin2dec(get_stuff(instructions_bin, register_size, rd_start), register_size);
    int shamt = bin2dec(get_stuff(instructions_bin, shamt_size, shamt_start), shamt_size);
    int funct = bin2dec(get_stuff(instructions_bin, funct_size, funct_start), funct_size);

    switch(funct){
        case 8:     // jr
            PC = reg_arr[rs];
            print_R_Instructions(opcode, rs, rt, reg_arr, shamt, funct);
            printf("[Jump register] r[%d]\n", rs);
            printf("[PC Update]: PC <- 0x%0x\n\n", PC);
            break;
        default:
            PC = regular_PC_update(PC);
    }
    return PC;
}

// I instructions
int get_I_instruction_information(int instructions, int* instructions_bin, int opcode, int* reg_arr, int PC){
    printf("[Decode Instruction] type: I\n");
//    for(int i = 0; i < instruction_size; i+=4){
//        printf("%d%d%d%d ", instructions_bin[i],instructions_bin[i+1],instructions_bin[i+2], instructions_bin[i+3]);
//    }

    int rs = bin2dec(get_stuff(instructions_bin, register_size, rs_start), register_size);
    int rt = bin2dec(get_stuff(instructions_bin, register_size, rt_start), register_size);
    int immediate = bin2dec(get_stuff(instructions_bin, immediate_size, immediate_start), immediate_size);


    switch(opcode){
        case 9:     // addiu
            reg_arr[rt] = reg_arr[rs] + immediate;
            print_I_Instructions(opcode, rs, rt, reg_arr, immediate);
            printf("[Add] r[%d] <- r[%d] + 0x%x\n", rt, rs, immediate);
            PC = regular_PC_update(PC);
            break;

        case 4:     // beq
            printf("[Branch on equal] : if(R[%d]==R[%d]) PC <- PC+4+0x%x\n", rs, rt, immediate);
            if(reg_arr[rs] == reg_arr[rt]){
                PC = branch_PC_update(PC, immediate);
            } else{
                PC = regular_PC_update(PC);
            }
            break;

        case 5:
            printf("[Branch on not equal] : if(R[%d]!=R[%d]) PC <- PC+4+0x%x\n", rs, rt, immediate);
            if(reg_arr[rs] != reg_arr[rt]){
                PC = branch_PC_update(PC, immediate);
            } else{
                PC = regular_PC_update(PC);
            }
            break;

        case 35:
            print_I_Instructions(opcode, rs, rt, reg_arr, immediate);
            printf("[Store]: r[%d] <- Mem[0x%0.8x] = 0x%x\n", rt, rs + immediate, reg_arr[rs]);
            PC = regular_PC_update(PC);
            break;

        case 43:    // sw
            reg_arr[rt] = reg_arr[rs + immediate];
            print_I_Instructions(opcode, rs, rt, reg_arr, immediate);
            printf("[Store]: Mem[0x%0.8x] <- r[%d] = 0x%x\n", rs+immediate, rt , reg_arr[rt]);
            PC = regular_PC_update(PC);
            break;
        default:
            PC = regular_PC_update(PC);
    }
    return PC;
}

// J instruction
int get_J_instruction_information(int instructions, int* instructions_bin, int opcode, int PC){
    printf("[Decode Instruction] type: J\n");
    int address = bin2dec(get_stuff(instructions_bin, address_size, address_start), address_size);
    print_J_Instructions(opcode, address);
    PC = regular_PC_update(PC);
    return PC;
}

// 000000 11 1100 0000 1110 1000 00 100101  <- line 30/58
// 000000 11 1010 0000 1111 0000 00 100101  <- line 10/8