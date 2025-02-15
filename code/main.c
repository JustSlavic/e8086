#include <stdio.h>
#include <stdlib.h>

#define ARRAY_COUNT(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

typedef int bool;
#define true 1
#define false 0

typedef   signed char   int8;
typedef unsigned char  uint8;
typedef          short  int16;
typedef unsigned short uint16;
typedef   signed int    int32;
typedef unsigned int   uint32;


// @note: only first 6 bits are meaningful
int opcode_mask = 0b11111100;

enum opcode
{
    OPCODE_MOV1 = 0b10001000, // mov (register/memory to/from register)
    OPCODE_MOV2 = 0b11000110, // mov (immediate to register/memory)
    OPCODE_MOV3 = 0b10110000, // mov (immediate to register)
    OPCODE_MOV4 = 0b10100000, // mov (memory to accumulator)
    OPCODE_MOV5 = 0b10100010, // mov (accumulator to memory)
    OPCODE_MOV6 = 0b10001110, // mov (register/memory to segment register)
    OPCODE_MOV7 = 0b10001100, // mov (segment register to register/memory)
};

enum
{
    MOD_MM   = 0,
    MOD_MM8  = 1,
    MOD_MM16 = 2,
    MOD_RM   = 3,
};

typedef struct
{
    enum opcode opcode;
    int mask;
    int nbits;
    char const *name;
} opcode_info;

opcode_info opcode_table[] =
{
    { OPCODE_MOV1, 0b11111100, 6, "mov" },
    { OPCODE_MOV2, 0b11111110, 7, "mov" },
    { OPCODE_MOV3, 0b11110000, 4, "mov" },
    { OPCODE_MOV4, 0b11111110, 7, "mov" },
    { OPCODE_MOV5, 0b11111110, 7, "mov" },
    { OPCODE_MOV6, 0b11111111, 8, "mov" },
    { OPCODE_MOV7, 0b11111111, 8, "mov" },
};

char const *register_names[8][2] =
{
    { "al", "ax" }, // 0
    { "cl", "cx" }, // 1
    { "dl", "dx" }, // 2
    { "bl", "bx" }, // 3
    { "ah", "sp" }, // 4
    { "ch", "bp" }, // 5
    { "dh", "si" }, // 6
    { "bh", "di" }, // 7
};

typedef struct
{
    uint8 *data;
    uint32 size;
    uint32 index;
} memory_buffer;

struct displacement
{
    uint32 registers[2];
    uint32 register_count; // 0, 1, or 2
    uint32 additive;
};


struct displacement make_displacement(memory_buffer *buffer, int32 mod, int32 r_m)
{
    struct displacement displacement = {};

    switch (r_m)
    {
        case 0b000: // (bx + si + displacement)
        {
            displacement.registers[0] = 3; // bx
            displacement.registers[1] = 6; // si
            displacement.register_count = 2;
        }
        break;

        case 0b001: // (bx + di + displacement)
        {
            displacement.registers[0] = 3; // bx
            displacement.registers[1] = 7; // di
            displacement.register_count = 2;
        }
        break;

        case 0b010: // (bp + si + displacement)
        {
            displacement.registers[0] = 5; // bp
            displacement.registers[1] = 6; // si
            displacement.register_count = 2;
        }
        break;

        case 0b011: // (bp + di + displacement)
        {
            displacement.registers[0] = 5; // bp
            displacement.registers[1] = 7; // di
            displacement.register_count = 2;
        }
        break;

        case 0b100: // (si + displacement)
        {
            displacement.registers[0] = 6; // si
            displacement.register_count = 1;
        }
        break;

        case 0b101: // (di + displacement)
        {
            displacement.registers[0] = 7; // di
            displacement.register_count = 1;
        }
        break;

        case 0b110: // direct address or (bp + displacement)
        {
            if (mod == MOD_MM)
            {} // direct address, no registers involved
            else
            {
                displacement.registers[0] = 5; // bp
                displacement.register_count = 1;
            }
        }
        break;

        case 0b111: // (bx + displacement)
        {
            displacement.registers[0] = 3; // bx
            displacement.register_count = 1;
        }
        break;
    }

    if (mod == MOD_MM)
    {
        // no additive
    }
    else if (mod == MOD_MM8)
    {
        displacement.additive = *(int8 *) (buffer->data + buffer->index);
        buffer->index += 1;
    }
    else if (mod == MOD_MM16)
    {
        displacement.additive = *(int16 *) (buffer->data + buffer->index);
        buffer->index += 2;
    }

    return displacement;
}


void mov1(memory_buffer *buffer, opcode_info *info)
{
    uint8 byte1 = buffer->data[buffer->index++];
    uint8 byte2 = buffer->data[buffer->index++];

    int32 d = 0b00000010 & byte1;
    int32 w = 0b00000001 & byte1;

    int32 mod = (0b11000000 & byte2) >> 6;
    int32 reg = (0b00111000 & byte2) >> 3;
    int32 r_m = (0b00000111 & byte2);

    if (mod == MOD_RM)
    {
        if (d)
        {
            printf("    %s %s, %s\n", info->name, register_names[reg][w], register_names[r_m][w]);
        }
        else
        {
            printf("    %s %s, %s\n", info->name, register_names[r_m][w], register_names[reg][w]);
        }
    }
    else
    {
        struct displacement displacement = make_displacement(buffer, mod, r_m);
        (void) displacement;

        printf("    %s ", info->name);

        if (d)
        {
            printf("%s, ", register_names[reg][w]);
        }

        if (displacement.register_count == 0)
        {
            printf("[%d]", displacement.additive);
        }
        else if (displacement.register_count == 1)
        {
            printf("[%s", register_names[displacement.registers[0]][1]);
            if (displacement.additive == 0)
            {
                printf("]");
            }
            else
            {
                printf(" + %d]", displacement.additive);
            }
        }
        else if (displacement.register_count == 2)
        {
            printf("[%s + %s", register_names[displacement.registers[0]][1],
                register_names[displacement.registers[1]][1]);
            if (displacement.additive == 0)
            {
                printf("]");
            }
            else
            {
                printf(" + %d]", displacement.additive);
            }
        }

        if (!d)
        {
            printf(", %s", register_names[reg][w]);
        }

        printf("\n");
    }
}


void mov2(memory_buffer *buffer, opcode_info *info)
{
    uint8 byte1 = buffer->data[buffer->index++];
    uint8 byte2 = buffer->data[buffer->index++];

    int32 w = 0b00000001 & byte1;

    int32 mod = (0b11000000 & byte2) >> 6;
    int32 r_m = (0b00000111 & byte2);

    struct displacement displacement = make_displacement(buffer, mod, r_m);
    (void) displacement;

    int32 data = 0;
    if (w)
    {
        data = *(int16 *) (buffer->data + buffer->index);
        buffer->index += 2;
    }
    else
    {
        data = buffer->data[buffer->index];
        buffer->index += 1;
    }
}


void mov3(memory_buffer *buffer, opcode_info *info)
{
    uint8 byte1 = buffer->data[buffer->index++];

    int32 w = 0b00001000 & byte1;
    int32 reg = 0b00000111 & byte1;

    int32 data = 0;
    if (w)
    {
        data = *(int16 *) (buffer->data + buffer->index);
        buffer->index += 2;
    }
    else
    {
        data = *(int8 *) (buffer->data + buffer->index);
        buffer->index += 1;
    }

    printf("    %s %s, %d\n", info->name, register_names[reg][w], (int) data);
}


int main()
{
    char const *filename = "assignment2";

    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("Could not open file \'%s\'\n", filename);
        return 1;
    }

    memory_buffer buffer = {};
    buffer.size = 1024;
    buffer.data = malloc(buffer.size);
    memset(buffer.data, 0, buffer.size);

    size_t n = fread(buffer.data, 1, buffer.size, f);
    fclose(f);

    // decoding

    fprintf(stdout, "; read %llu bytes\nbits 16\n", n);

    while (buffer.index < n)
    {
        uint8 byte = buffer.data[buffer.index];
        opcode_info info = {};
        bool found = false;
        for (int opcode_index = 0; opcode_index < ARRAY_COUNT(opcode_table); opcode_index++)
        {
            info = opcode_table[opcode_index];
            int32 opcode = (byte & info.mask);
            if (opcode == info.opcode)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            printf("Unsupported opcode '0x%x'!\n", info.opcode);
            exit(1);
        }

        // printf("Found opcode '0x%x'\n", info.opcode);

        if (info.opcode == OPCODE_MOV1)      mov1(&buffer, &info);
        else if (info.opcode == OPCODE_MOV2) mov2(&buffer, &info);
        else if (info.opcode == OPCODE_MOV3) mov3(&buffer, &info);
        // else if (info.opcode == OPCODE_MOV4) ;
        else
        {
            printf("Don't know what to do!\n");
            break;
        }
    }

    return 0;
}

