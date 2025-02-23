#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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


void print_binary8(uint8 n)
{
    uint32 mask = 0b10000000;
    for (int i = 0; i < 8; i++)
    {
        printf("%d", (mask & n) > 0);
        mask = (mask >> 1);
    }
}

void print_binary16(uint16 n)
{
    uint32 mask = 0b1000000000000000;
    for (int i = 0; i < 16; i++)
    {
        printf("%d", (mask & n) > 0);
        mask = (mask >> 1);
    }
}

void print_binary32(uint32 n)
{
    uint32 mask = 0b10000000000000000000000000000000;
    for (int i = 0; i < 32; i++)
    {
        printf("%d", (mask & n) > 0);
        mask = (mask >> 1);
    }
}


/*
    sp - stack pointer
    bp - base pointer
    si - source index
    di - destination index
*/

enum opcode
{
    OPCODE_MOV1 = 0b10001000, // mov (register/memory to/from register)
    OPCODE_MOV2 = 0b11000110, // mov (immediate to register/memory)
    OPCODE_MOV3 = 0b10110000, // mov (immediate to register)
    OPCODE_MOV4 = 0b10100000, // mov (memory to accumulator)
    OPCODE_MOV5 = 0b10100010, // mov (accumulator to memory)
    OPCODE_MOV6 = 0b10001110, // mov (register/memory to segment register)
    OPCODE_MOV7 = 0b10001100, // mov (segment register to register/memory)

    OPCODE_ADD1 = 0b00000000, // add (reg/memory with register to either)
    OPCODE_ADD3 = 0b00000100, // add (immediate to accumulator)

    OPCODE_SUB1 = 0b00101000, // sub (reg/memory and register to either)
    OPCODE_SUB3 = 0b00101100, // sub (immediate from accumulator)

    OPCODE_CMP1 = 0b00111000, // cmp (reg/memory and register)
    OPCODE_CMP3 = 0b00111100, // cmp (immediate with accumulator)

    OPCODE_JE   = 0b01110100, // jump on equal / zero
    OPCODE_JL   = 0b01111100, // jump on less / not greater or equal
    OPCODE_JLE  = 0b01111110, // jump on less or equal / not greater
    OPCODE_JB   = 0b01110010, // jump on below/not above or equal
    OPCODE_JBE  = 0b01110110, // jump on below or equal / not above
    OPCODE_JP   = 0b01111010, // jump on parity / parity even
    OPCODE_JO   = 0b01110000, // jump on overflow
    OPCODE_JS   = 0b01111000, // jump on sign
    OPCODE_JNE  = 0b01110101, // jump on not equal / not zero
    OPCODE_JNL  = 0b01111101, // jump on not less / greater or equal
    OPCODE_JNLE = 0b01111111, // jump on not less or equal / greater
    OPCODE_JNB  = 0b01110011, // jump on not below / above or equal
    OPCODE_JNBE = 0b01110111, // jump on not below or equal / above
    OPCODE_JNP  = 0b01111011, // jump on not par / par odd
    OPCODE_JNO  = 0b01110001, // jump on not overflow
    OPCODE_JNS  = 0b01111001, // jump on not sign
    OPCODE_LOOP = 0b11100010, // loop CX times
    OPCODE_LOOPZ = 0b11100001, // loop while zero
    OPCODE_LOOPNZ = 0b11100000, // loop while not zero
    OPCODE_JCXZ = 0b11100011, // jump on CX zero

    OPCODE_IMM_TO_REG_MEM = 0b10000000, // add (immediate to register/memory)
};

enum
{
    MOD_MM   = 0b00,
    MOD_MM8  = 0b01,
    MOD_MM16 = 0b10,
    MOD_RM   = 0b11,
};

typedef struct
{
    enum opcode opcode;
    int mask;
    char const *name;
} opcode_info;

opcode_info opcode_table[] =
{
    { OPCODE_MOV1, 0b11111100, "mov" },
    { OPCODE_MOV2, 0b11111110, "mov" },
    { OPCODE_MOV3, 0b11110000, "mov" },
    { OPCODE_MOV4, 0b11111110, "mov" },
    { OPCODE_MOV5, 0b11111110, "mov" },
    { OPCODE_MOV6, 0b11111111, "mov" },
    { OPCODE_MOV7, 0b11111111, "mov" },

    { OPCODE_ADD1, 0b11111100, "add" },
    { OPCODE_ADD3, 0b11111110, "add" },

    { OPCODE_SUB1, 0b11111100, "sub" },
    { OPCODE_SUB3, 0b11111110, "sub" },

    { OPCODE_CMP1, 0b11111100, "cmp" },
    { OPCODE_CMP3, 0b11111110, "cmp" },

    { OPCODE_JE,   0b11111111, "je" },
    { OPCODE_JL,   0b11111111, "jl" },
    { OPCODE_JLE,  0b11111111, "jle" },
    { OPCODE_JB,   0b11111111, "jb" },
    { OPCODE_JBE,  0b11111111, "jbe" },
    { OPCODE_JP,   0b11111111, "jp" },
    { OPCODE_JO,   0b11111111, "jo" },
    { OPCODE_JS,   0b11111111, "js" },
    { OPCODE_JNE,  0b11111111, "jne" },
    { OPCODE_JNL,  0b11111111, "jnl" },
    { OPCODE_JNLE, 0b11111111, "jnle" },
    { OPCODE_JNB,  0b11111111, "jnb" },
    { OPCODE_JNBE, 0b11111111, "jnbe" },
    { OPCODE_JNP,  0b11111111, "jnp" },
    { OPCODE_JNO,  0b11111111, "jno" },
    { OPCODE_JNS,  0b11111111, "jns" },
    { OPCODE_LOOP, 0b11111111, "loop" },
    { OPCODE_LOOPZ, 0b11111111, "loopz" },
    { OPCODE_LOOPNZ, 0b11111111, "loopnz" },
    { OPCODE_JCXZ, 0b11111111, "jcxz" },

    { OPCODE_IMM_TO_REG_MEM, 0b11111100, "???" },
};

enum
{
    REGISTER_AL = 0b0000000000000001,
    REGISTER_AH = 0b0000000000000010,
    REGISTER_AX = REGISTER_AH | REGISTER_AL,

    REGISTER_BL = 0b0000000000000100,
    REGISTER_BH = 0b0000000000001000,
    REGISTER_BX = REGISTER_BH | REGISTER_BL,

    REGISTER_CL = 0b0000000000010000,
    REGISTER_CH = 0b0000000000100000,
    REGISTER_CX = REGISTER_CH | REGISTER_CL,

    REGISTER_DL = 0b0000000001000000,
    REGISTER_DH = 0b0000000010000000,
    REGISTER_DX = REGISTER_DH | REGISTER_DL,

    REGISTER_SP = 0b0000000100000000,
    REGISTER_BP = 0b0000001000000000,
    REGISTER_SI = 0b0000010000000000,
    REGISTER_DI = 0b0000100000000000,
};

typedef struct
{
    union { uint16 ax; struct { uint8 al, ah; }; };
    union { uint16 bx; struct { uint8 bl, bh; }; };
    union { uint16 cx; struct { uint8 cl, ch; }; };
    union { uint16 dx; struct { uint8 dl, dh; }; };
    uint16 sp;
    uint16 bp;
    uint16 si;
    uint16 di;
} registers;

typedef enum
{
    I_NONE,

    I_MOV,
    I_ADD,
    I_SUB,
} instruction_tag;

typedef enum
{
    IOPERAND_NONE,

    IOPERAND_IMMEDIATE,
    IOPERAND_REGISTER,
    IOPERAND_ADDRESS,
} instruction_operand_tag;

typedef struct
{
    uint32 reg1, reg2;
    uint32 reg_count; // 0, 1, or 2
    uint32 displacement;
} effective_address;

typedef struct
{
    instruction_operand_tag tag;
    union
    {
        int32 imm;
        int32 reg;
        effective_address addr;
    };
} instruction_operand;

typedef struct
{
    instruction_tag tag;

    instruction_operand source, destination;
} instruction;

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


effective_address ea_table[8] =
{
    { .reg1 = 3, .reg2 = 6, .reg_count = 2 }, // (bx + si + displacement)
    { .reg1 = 3, .reg2 = 7, .reg_count = 2 }, // (bx + di + displacement)
    { .reg1 = 5, .reg2 = 6, .reg_count = 2 }, // (bp + si + displacement)
    { .reg1 = 5, .reg2 = 7, .reg_count = 2 }, // (bp + di + displacement)
    { .reg1 = 6, .reg_count = 1 },            // (si + displacement)
    { .reg1 = 7, .reg_count = 1 },            // (di + displacement)
    { .reg1 = 5, .reg_count = 1 },            // (bp + displacement) OR direct address
    { .reg1 = 3, .reg_count = 1 },            // (bx + displacement)
};


effective_address read_ea(memory_buffer *buffer, int32 mod, int32 r_m)
{
    effective_address ea = ea_table[r_m];

    if (mod == MOD_MM)
    {
        // Direct address reading
        if (r_m == 0b110)
        {
            ea.reg_count = 0;
            ea.displacement = *(int16 *) (buffer->data + buffer->index);
            buffer->index += 2;
        }
    }
    else if (mod == MOD_MM8)
    {
        ea.displacement = *(int8 *) (buffer->data + buffer->index);
        buffer->index += 1;
    }
    else if (mod == MOD_MM16)
    {
        ea.displacement = *(int16 *) (buffer->data + buffer->index);
        buffer->index += 2;
    }

    return ea;
}


void print_eac(effective_address ea)
{
    if (ea.reg_count == 0)
    {
        printf("[%d]", ea.displacement);
    }
    else if (ea.reg_count == 1)
    {
        printf("[%s", register_names[ea.reg1][1]);
        if (ea.displacement == 0)
        {
            printf("]");
        }
        else
        {
            printf(" + %d]", ea.displacement);
        }
    }
    else if (ea.reg_count == 2)
    {
        printf("[%s + %s", register_names[ea.reg1][1],
            register_names[ea.reg2][1]);
        if (ea.displacement == 0)
        {
            printf("]");
        }
        else
        {
            printf(" + %d]", ea.displacement);
        }
    }
}


int32 read_data_bytes(memory_buffer *buffer, int32 w, int32 s)
{
    int32 data = (!s && w) ? *(int16 *) (buffer->data + buffer->index)
                           : *(int8 *)  (buffer->data + buffer->index);
    buffer->index += (!s && w) ? 2 : 1;
    return data;
}


void instruction_type1(memory_buffer *buffer, opcode_info *info)
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
        effective_address ea = read_ea(buffer, mod, r_m);
        (void) ea;

        printf("    %s ", info->name);

        if (d)
        {
            printf("%s, ", register_names[reg][w]);
        }

        if (ea.reg_count == 0)
        {
            printf("[%d]", ea.displacement);
        }
        else if (ea.reg_count == 1)
        {
            printf("[%s", register_names[ea.reg1][1]);
            if (ea.displacement == 0)
            {
                printf("]");
            }
            else
            {
                printf(" + %d]", ea.displacement);
            }
        }
        else if (ea.reg_count == 2)
        {
            printf("[%s + %s", register_names[ea.reg1][1],
                register_names[ea.reg2][1]);
            if (ea.displacement == 0)
            {
                printf("]");
            }
            else
            {
                printf(" + %d]", ea.displacement);
            }
        }

        if (!d)
        {
            printf(", %s", register_names[reg][w]);
        }

        printf("\n");
    }
}


void instruction_mov_imm_to_reg_mem(memory_buffer *buffer, opcode_info *info)
{
    uint8 byte1 = buffer->data[buffer->index++];
    uint8 byte2 = buffer->data[buffer->index++];

    int32 w = 0b00000001 & byte1;

    int32 mod = (0b11000000 & byte2) >> 6;
    int32 r_m = (0b00000111 & byte2);

    effective_address ea = read_ea(buffer, mod, r_m);
    (void) ea;

    int32 data = read_data_bytes(buffer, w, 0);
    (void) data;
}


void instruction_imm_to_reg(memory_buffer *buffer, opcode_info *info)
{
    uint8 byte1 = buffer->data[buffer->index++];

    int32 w = 0b00001000 & byte1;
    int32 reg = 0b00000111 & byte1;

    int32 data = read_data_bytes(buffer, w, 0);

    printf("    %s %s, %d\n", info->name, register_names[reg][w], (int) data);
}


void mov45(memory_buffer *buffer, opcode_info *info, bool reverse_order)
{
    uint8 byte1 = buffer->data[buffer->index++];

    int32 w = 0b00000001 & byte1;

    int32 addr = read_data_bytes(buffer, w, 0);

    if (reverse_order)
        printf("    %s [%d], ax\n", info->name, addr);
    else
        printf("    %s ax, [%d]\n", info->name, addr);
}


void instruction_imm_to_reg_mem(memory_buffer *buffer, opcode_info *info)
{
    uint8 byte1 = buffer->data[buffer->index++];
    uint8 byte2 = buffer->data[buffer->index++];

    int32 s = 0b00000010 & byte1;
    int32 w = 0b00000001 & byte1;

    int32 mod = (0b11000000 & byte2) >> 6;
    int32 opc = (0b00111000 & byte2) >> 3;
    int32 r_m = (0b00000111 & byte2);

    switch (opc)
    {
    case 0b000: printf("    add"); break;
    case 0b101: printf("    sub"); break;
    case 0b111: printf("    cmp"); break;
    default:
        printf("unknown sub_opcode\n");
        exit(1);
    }

    if (mod == MOD_RM)
    {
        printf(" %s", register_names[r_m][w]);
    }
    else
    {
        printf(" ");
        effective_address ea = read_ea(buffer, mod, r_m);
        print_eac(ea);
    }

    int32 data = read_data_bytes(buffer, w, s);
    printf(", %d\n", data);
}


void instruction_imm_to_acc(memory_buffer *buffer, opcode_info *info)
{
    uint8 byte1 = buffer->data[buffer->index++];

    int32 w = 0b00000001 & byte1;
    int32 data = read_data_bytes(buffer, w, 0);

    printf("    %s %s, %d\n", info->name, w ? "ax" : "al", (int) data);
}


void instruction_jumps(memory_buffer *buffer, opcode_info *info)
{
    buffer->index++; // first byte is fully opcode
    int8 ip_inc8 = buffer->data[buffer->index++];

    printf("    %s %d\n", info->name, (int) ip_inc8);
}


int main()
{
    char const *filename = "computer_enhance/perfaware/part1/listing_0041_add_sub_cmp_jnz";

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

    // print_binary8(0b10000000);
    // printf("\n\n");

    fprintf(stdout, "; read %zu bytes\nbits 16\n", n);

    while (buffer.index < n)
    {
        uint8 byte = buffer.data[buffer.index];
        opcode_info info = {};
        bool found = false;
        for (int opcode_index = 0; opcode_index < ARRAY_COUNT(opcode_table); opcode_index++)
        {
            info = opcode_table[opcode_index];
            int32 opcode = (byte & info.mask);

            // printf("BYTE:          0b");
            // print_binary8(byte);
            // printf("\n");

            // printf("mask:          0b");
            // print_binary8(info.mask);
            // printf("\n");

            // printf("%03d: opcode =  0b", opcode_index);
            // print_binary8(opcode);
            // printf("\n");

            // printf("info.opcode =  0b");
            // print_binary8(info.opcode);
            // printf("\n");

            // printf("\n");

            if (opcode == info.opcode)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            printf("Can't find opcode for byte: 0b");
            print_binary8(byte);
            printf("\n");
            exit(1);
        }

        // printf("Found opcode: 0b");
        // print_binary8(info.opcode);
        // printf("\n");

        switch (info.opcode)
        {
        case OPCODE_MOV1: instruction_type1(&buffer, &info); break;
        case OPCODE_MOV2: instruction_mov_imm_to_reg_mem(&buffer, &info); break;
        case OPCODE_MOV3: instruction_imm_to_reg(&buffer, &info); break;
        case OPCODE_MOV4: mov45(&buffer, &info, false); break;
        case OPCODE_MOV5: mov45(&buffer, &info, true); break;

        case OPCODE_ADD1: instruction_type1(&buffer, &info); break;
        case OPCODE_ADD3: instruction_imm_to_acc(&buffer, &info); break;

        case OPCODE_SUB1: instruction_type1(&buffer, &info); break;
        case OPCODE_SUB3: instruction_imm_to_acc(&buffer, &info); break;

        case OPCODE_CMP1: instruction_type1(&buffer, &info); break;
        case OPCODE_CMP3: instruction_imm_to_acc(&buffer, &info); break;

        case OPCODE_JE:
        case OPCODE_JL:
        case OPCODE_JLE:
        case OPCODE_JB:
        case OPCODE_JBE:
        case OPCODE_JP:
        case OPCODE_JO:
        case OPCODE_JS:
        case OPCODE_JNE:
        case OPCODE_JNL:
        case OPCODE_JNLE:
        case OPCODE_JNB:
        case OPCODE_JNBE:
        case OPCODE_JNP:
        case OPCODE_JNO:
        case OPCODE_JNS:
        case OPCODE_LOOP:
        case OPCODE_LOOPZ:
        case OPCODE_LOOPNZ:
        case OPCODE_JCXZ:
            instruction_jumps(&buffer, &info);
            break;

        case OPCODE_IMM_TO_REG_MEM:
            instruction_imm_to_reg_mem(&buffer, &info); break;

        default:
            printf("Don't know what to do!\n");
            exit(1);
        }
    }


    return 0;
}

