#include <stdio.h>
#include <stdlib.h>


// @note: only first 6 bits are meaningful
int opcode_mask = 0xfc;

enum opcode_table
{
    OPCODE_MOV1 = 0x88, // mov (register/memory to/from register)
};

char const *reg_table[8][2] =
{
    { "al", "ax" },
    { "cl", "cx" },
    { "dl", "dx" },
    { "bl", "bx" },
    { "ah", "sp" },
    { "ch", "bp" },
    { "dh", "si" },
    { "bh", "di" },
};



int main()
{
    char const *filename = "t";

    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("Could not open file \'%s\'\n", filename);
        return 1;
    }

    size_t buffer_size = 1024;
    char *buffer = malloc(buffer_size);
    memset(buffer, 0, buffer_size);
    size_t n = fread(buffer, 1, buffer_size, f);
    fclose(f);

    // decoding

    fprintf(stdout, "; read %llu bytes\nbits 16\n", n);

    int opcode = buffer[0] & opcode_mask;
    switch (opcode)
    {
    case OPCODE_MOV1:
        {
            fprintf(stdout, "mov ");
        }
    }

    int d = buffer[0] & 0x02;
    int w = buffer[0] & 0x01;

    int mode = buffer[1] & 0xc0;
    int reg = (buffer[1] & 0x38) >> 3;
    int rm = buffer[1] & 0x07;

    if (mode == 0xc0) // mode = 0b1100'0000
    {
        char const *reg_name = reg_table[reg][w];
        char const *rm_name = reg_table[rm][w];

        char const *dest_register = d > 0 ? reg_name : rm_name;
        char const *source_register = d > 0 ? rm_name : reg_name;

        fprintf(stdout, "%s, %s\n", dest_register, source_register);
    }
    else
    {
        fprintf(stdout, "Unsopported mode (0x%x)\n", mode);
    }

    return 0;
}
