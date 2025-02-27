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

enum
{
/*
   0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15
  al   cl   dl   bl   ah   ch   dh   bh   ax   cx   dx   bx   sp   bp   si   di
 000  001  010  011  100  101  110  111 1000 1001 1010 1011 1100 1101 1110 1111
*/
    R_AL, R_CL, R_DL, R_BL, R_AH, R_CH, R_DH, R_BH,
    R_AX, R_CX, R_DX, R_BX, R_SP, R_BP, R_SI, R_DI,
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
    uint16 ip;
    // struct
    // {
    //     uint16 _u0 : 1;
    //     uint16 _u1 : 1;
    //     uint16 _u2 : 1;
    //     uint16 _u3 : 1;
    //     uint16 fo : 1;
    //     uint16 fd : 1;
    //     uint16 fi : 1;
    //     uint16 ft : 1;
    //     uint16 fs : 1;
    //     uint16 fz : 1;
    //     uint16 _u4 : 1;
    //     uint16 fa : 1;
    //     uint16 _u5 : 1;
    //     uint16 f : 1;
    //     uint16 f : 1;
    // };
    bool fz;
    bool fs;
} registers;

typedef enum
{
    I_NOOP,

    I_MOV,
    I_ADD,
    I_SUB,

    I_CMP,
    I_JE,  I_JL,  I_JLE,  I_JB,  I_JBE,  I_JP,  I_JO,  I_JS,
    I_JNE, I_JNL, I_JNLE, I_JNB, I_JNBE, I_JNP, I_JNO, I_JNS,
    I_LOOP,
    I_LOOPZ,
    I_LOOPNZ,
    I_JCXZ,
} instruction_tag;

typedef enum
{
    IOPERAND_NONE,

    IOP_IMM,
    IOP_REG,
    IOP_MEM,
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

typedef struct
{
    enum opcode opcode;
    int mask;
    instruction_tag instruction;
} opcode_info;

opcode_info opcode_table[] =
{
    { OPCODE_MOV1, 0b11111100, I_MOV },
    { OPCODE_MOV2, 0b11111110, I_MOV },
    { OPCODE_MOV3, 0b11110000, I_MOV },
    { OPCODE_MOV4, 0b11111110, I_MOV },
    { OPCODE_MOV5, 0b11111110, I_MOV },
    { OPCODE_MOV6, 0b11111111, I_MOV },
    { OPCODE_MOV7, 0b11111111, I_MOV },

    { OPCODE_ADD1, 0b11111100, I_ADD },
    { OPCODE_ADD3, 0b11111110, I_ADD },

    { OPCODE_SUB1, 0b11111100, I_SUB },
    { OPCODE_SUB3, 0b11111110, I_SUB },

    { OPCODE_CMP1, 0b11111100, I_CMP },
    { OPCODE_CMP3, 0b11111110, I_CMP },

    { OPCODE_JE,   0b11111111, I_JE },
    { OPCODE_JL,   0b11111111, I_JL },
    { OPCODE_JLE,  0b11111111, I_JLE },
    { OPCODE_JB,   0b11111111, I_JB },
    { OPCODE_JBE,  0b11111111, I_JBE },
    { OPCODE_JP,   0b11111111, I_JP },
    { OPCODE_JO,   0b11111111, I_JO },
    { OPCODE_JS,   0b11111111, I_JS },
    { OPCODE_JNE,  0b11111111, I_JNE },
    { OPCODE_JNL,  0b11111111, I_JNL },
    { OPCODE_JNLE, 0b11111111, I_JNLE },
    { OPCODE_JNB,  0b11111111, I_JNB },
    { OPCODE_JNBE, 0b11111111, I_JNBE },
    { OPCODE_JNP,  0b11111111, I_JNP },
    { OPCODE_JNO,  0b11111111, I_JNO },
    { OPCODE_JNS,  0b11111111, I_JNS },
    { OPCODE_LOOP, 0b11111111, I_LOOP },
    { OPCODE_LOOPZ, 0b11111111, I_LOOPZ },
    { OPCODE_LOOPNZ, 0b11111111, I_LOOPNZ },
    { OPCODE_JCXZ, 0b11111111, I_JCXZ },

    { OPCODE_IMM_TO_REG_MEM, 0b11111100, I_NOOP },
};

char const *register_names[] =
{
    "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh",
    "ax", "cx", "dx", "bx", "sp", "bp", "si", "di",
};

char const *instruction_names[] =
{
    "NOOP",
    "MOV", "ADD", "SUB",
    "CMP",
    "JE", "JL", "JLE", "JB",
    "JBE",
    "JP",
    "JO",
    "JS",
    "JNE",
    "JNL",
    "JNLE",
    "JNB",
    "JNBE",
    "JNP",
    "JNO",
    "JNS",
    "LOOP",
    "LOOPZ",
    "LOOPNZ",
    "JCXZ",
};

void print_ea(effective_address ea)
{
    if (ea.reg_count == 0)
    {
        printf("[%d]", ea.displacement);
    }
    else if (ea.reg_count == 1)
    {
        printf("[%s", register_names[ea.reg1 | 0b1000]);
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
        printf("[%s + %s", register_names[ea.reg1 | 0b1000],
            register_names[ea.reg2 | 0b1000]);
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

void print_instruction_operand(instruction_operand iop)
{
    switch (iop.tag)
    {
    case IOPERAND_NONE: break;
    case IOP_IMM: printf("%d", iop.imm); break;
    case IOP_REG: printf("%s", register_names[iop.reg]); break;
    case IOP_MEM: print_ea(iop.addr); break;
    }
}

void print_instruction(instruction i)
{
    printf("    %s ", instruction_names[i.tag]);
    print_instruction_operand(i.destination);
    if (i.source.tag != IOPERAND_NONE)
    {
        printf(", ");
        print_instruction_operand(i.source);
    }
    printf("\n");
}

typedef struct
{
    uint8 *memory;
    uint32 size;

    registers rs;
} sim8086;

effective_address ea_table[8] =
{
    { .reg1 = R_BX, .reg2 = R_SI, .reg_count = 2 }, // (bx + si + displacement)
    { .reg1 = R_BX, .reg2 = R_DI, .reg_count = 2 }, // (bx + di + displacement)
    { .reg1 = R_BP, .reg2 = R_SI, .reg_count = 2 }, // (bp + si + displacement)
    { .reg1 = R_BP, .reg2 = R_DI, .reg_count = 2 }, // (bp + di + displacement)
    { .reg1 = R_SI, .reg_count = 1 },            // (si + displacement)
    { .reg1 = R_DI, .reg_count = 1 },            // (di + displacement)
    { .reg1 = R_BP, .reg_count = 1 },            // (bp + displacement) OR direct address
    { .reg1 = R_BX, .reg_count = 1 },            // (bx + displacement)
};


effective_address read_ea(sim8086 *sim, int32 mod, int32 r_m)
{
    effective_address ea = ea_table[r_m];

    if (mod == MOD_MM)
    {
        // Direct address reading
        if (r_m == 0b110)
        {
            ea.reg_count = 0;
            ea.displacement = *(int16 *) (sim->memory + sim->rs.ip);
            sim->rs.ip += 2;
        }
    }
    else if (mod == MOD_MM8)
    {
        ea.displacement = *(int8 *) (sim->memory + sim->rs.ip);
        sim->rs.ip += 1;
    }
    else if (mod == MOD_MM16)
    {
        ea.displacement = *(int16 *) (sim->memory + sim->rs.ip);
        sim->rs.ip += 2;
    }

    return ea;
}

int32 read_data_bytes(sim8086 *sim, int32 w, int32 s)
{
    int32 data = (!s && w) ? *(int16 *) (sim->memory + sim->rs.ip)
                           : *(int8 *)  (sim->memory + sim->rs.ip);
    sim->rs.ip += (!s && w) ? 2 : 1;
    return data;
}


instruction instruction_type1(sim8086 *sim, opcode_info *info)
{
    uint8 byte1 = sim->memory[sim->rs.ip++];
    uint8 byte2 = sim->memory[sim->rs.ip++];

    int32 d = 0b00000010 & byte1;
    int32 w = 0b00000001 & byte1;

    int32 mod = (0b11000000 & byte2) >> 6;
    int32 reg = (0b00111000 & byte2) >> 3;
    int32 r_m = (0b00000111 & byte2);

    instruction result =
    {
        .tag = info->instruction,
        .source =
        {
            .tag = IOP_REG,
            .reg = reg | (w << 3),
        },
    };

    if (mod == MOD_RM)
    {
        result.destination = (instruction_operand){ .tag = IOP_REG, .reg = r_m | (w << 3) };
    }
    else
    {
        result.destination = (instruction_operand)
        {
            .tag = IOP_MEM,
            .addr = read_ea(sim, mod, r_m),
        };
    }

    if (d)
    {
        instruction_operand tmp = result.destination;
        result.destination = result.source;
        result.source = tmp;
    }

    return result;
}

instruction instruction_mov_imm_to_reg_mem(sim8086 *sim, opcode_info *info)
{
    uint8 byte1 = sim->memory[sim->rs.ip++];
    uint8 byte2 = sim->memory[sim->rs.ip++];

    int32 w = (0b00000001 & byte1);
    int32 mod = (0b11000000 & byte2) >> 6;
    int32 opc = (0b00111000 & byte2) >> 3;
    int32 r_m = (0b00000111 & byte2);

    if (opc != 0) return (instruction){};

    instruction result = { .tag = I_MOV };

    if (mod == MOD_RM)
    {
        result.destination = (instruction_operand)
        {
            .tag = IOP_REG,
            .reg = r_m | (w << 3),
        };
    }
    else
    {
        result.destination = (instruction_operand)
        {
            .tag = IOP_MEM,
            .addr = read_ea(sim, mod, r_m),
        };
    }

    result.source = (instruction_operand)
    {
        .tag = IOP_IMM,
        .imm = read_data_bytes(sim, w, 0),
    };

    return result;
}

instruction instruction_imm_to_reg(sim8086 *sim, opcode_info *info)
{
    uint8 byte1 = sim->memory[sim->rs.ip++];

    int32 w = (0b00001000 & byte1) >> 3;
    int32 reg = 0b00000111 & byte1;

    int32 data = read_data_bytes(sim, w, 0);

    instruction result =
    {
        .tag = info->instruction,
        .source =
        {
            .tag = IOP_IMM,
            .imm = data,
        },
        .destination =
        {
            .tag = IOP_REG,
            .reg = reg | (w << 3),
        }
    };

    return result;
}


// void mov_memory_and_accumulator(sim8086 *sim, opcode_info *info, bool reverse_order)
// {
//     uint8 byte1 = sim->memory[sim->rs.ip++];

//     int32 w = 0b00000001 & byte1;

//     int32 addr = read_data_bytes(sim, w, 0);
//     (void) addr;

    // @todo:
    // if (reverse_order)
    //     printf("    %s [%d], ax\n", instruction_names[info->instruction], addr);
    // else
    //     printf("    %s ax, [%d]\n", instruction_names[info->instruction], addr);
// }


instruction instruction_imm_to_reg_mem(sim8086 *sim, opcode_info *info)
{
    uint8 byte1 = sim->memory[sim->rs.ip++];
    uint8 byte2 = sim->memory[sim->rs.ip++];

    int32 s = 0b00000010 & byte1;
    int32 w = 0b00000001 & byte1;

    int32 mod = (0b11000000 & byte2) >> 6;
    int32 opc = (0b00111000 & byte2) >> 3;
    int32 r_m = (0b00000111 & byte2);

    instruction result = {};

    switch (opc)
    {
    case 0b000: result.tag = I_ADD; break;
    case 0b101: result.tag = I_SUB; break;
    case 0b111: result.tag = I_CMP; break;
    default:
        printf("unknown sub_opcode\n");
        exit(1);
    }

    if (mod == MOD_RM)
    {
        result.destination = (instruction_operand)
        {
            .tag = IOP_REG,
            .reg = r_m | (w << 3),
        };
    }
    else
    {
        result.destination = (instruction_operand)
        {
            .tag = IOP_MEM,
            .addr = read_ea(sim, mod, r_m),
        };
    }

    result.source = (instruction_operand)
    {
        .tag = IOP_IMM,
        .imm = read_data_bytes(sim, w, s),
    };

    return result;
}


instruction instruction_imm_to_acc(sim8086 *sim, opcode_info *info)
{
    uint8 byte1 = sim->memory[sim->rs.ip++];

    int32 w = 0b00000001 & byte1;
    int32 data = read_data_bytes(sim, w, 0);

    instruction result =
    {
        .tag = info->instruction,
        .source =
        {
            .tag = IOP_IMM,
            .imm = data,
        },
        .destination =
        {
            .tag = IOP_REG,
            .reg = w << 3,
        }
    };
    return result;
}

instruction instruction_jumps(sim8086 *sim, opcode_info *info)
{
    sim->rs.ip++; // first byte is fully opcode
    int8 ip_inc8 = sim->memory[sim->rs.ip++];

    instruction result =
    {
        .tag = info->instruction,
        .destination =
        {
            .tag = IOP_IMM,
            .imm = ip_inc8,
        },
    };
    return result;
}

instruction decode_next_instruction(sim8086 *sim)
{
    instruction result = { .tag = I_NOOP };

    uint8 byte = sim->memory[sim->rs.ip];

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
        printf("Can't find opcode for byte: 0b");
        print_binary8(byte);
        printf("\n");
        exit(1);
    }

    switch (info.opcode)
    {
    case OPCODE_MOV1: result = instruction_type1(sim, &info); break;
    case OPCODE_MOV2: result = instruction_mov_imm_to_reg_mem(sim, &info); break;
    case OPCODE_MOV3: result = instruction_imm_to_reg(sim, &info); break;
    // case OPCODE_MOV4: mov_memory_and_accumulator(sim, &info, false); break;
    // case OPCODE_MOV5: mov_memory_and_accumulator(sim, &info, true); break;

    case OPCODE_ADD1: result = instruction_type1(sim, &info); break;
    case OPCODE_ADD3: result = instruction_imm_to_acc(sim, &info); break;

    case OPCODE_SUB1: result = instruction_type1(sim, &info); break;
    case OPCODE_SUB3: result = instruction_imm_to_acc(sim, &info); break;

    case OPCODE_CMP1: result = instruction_type1(sim, &info); break;
    case OPCODE_CMP3: result = instruction_imm_to_acc(sim, &info); break;

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
        result = instruction_jumps(sim, &info);
        break;

    case OPCODE_IMM_TO_REG_MEM:
        result = instruction_imm_to_reg_mem(sim, &info); break;

    default:
        printf("Don't know what to do!\n");
        exit(1);
    }

    return result;
}

void choose_register(sim8086 *sim, int32 reg, void **d, int32 *w)
{
    switch (reg)
    {
    case R_AL: *d = &sim->rs.al; break;
    case R_AH: *d = &sim->rs.ah; break;
    case R_AX: *d = &sim->rs.ax; *w = 1; break;

    case R_BL: *d = &sim->rs.bl; break;
    case R_BH: *d = &sim->rs.bh; break;
    case R_BX: *d = &sim->rs.bx; *w = 1; break;

    case R_CL: *d = &sim->rs.cl; break;
    case R_CH: *d = &sim->rs.ch; break;
    case R_CX: *d = &sim->rs.cx; *w = 1; break;

    case R_DL: *d = &sim->rs.dl; break;
    case R_DH: *d = &sim->rs.dh; break;
    case R_DX: *d = &sim->rs.dx; *w = 1; break;

    case R_BP: *d = &sim->rs.bp; *w = 1; break;
    case R_SP: *d = &sim->rs.sp; *w = 1; break;
    case R_DI: *d = &sim->rs.di; *w = 1; break;
    case R_SI: *d = &sim->rs.si; *w = 1; break;
    }
}

#define EXECUTE_INSTRUCTION(INSTR) do { \
    if (w) { \
        UPDATE_FLAGS(uint16); \
        ASSIGN(INSTR, uint16); \
    } else { \
        UPDATE_FLAGS(uint8); \
        ASSIGN(INSTR, uint8); \
    }} while (false)

void execute_mov(sim8086 *sim, void *d, void *s, int32 w)
{
    if (w) *(uint16 *) d = *(uint16 *) s;
    else   *(uint8  *) d = *(uint8  *) s;
}

#define execute_add_sub(INSTR) \
    if (w) { \
        *(uint16 *) d INSTR##= *(uint16 *) s; \
        sim->rs.fs = ((*(uint16 *) d) >> 15); \
        sim->rs.fz = ((*(uint16 *) d) == 0); \
    } else { \
        *(uint8 *) d INSTR##= *(uint8 *) s; \
        sim->rs.fs = ((*(uint8 *) d) >> 7); \
        sim->rs.fz = ((*(uint8 *) d) == 0); \
    }

void execute_cmp(sim8086 *sim, void *d, void *s, int32 w)
{
    if (w)
    {
        uint16 r = *(uint16 *) d - *(uint16 *) s;
        sim->rs.fs = (r >> 15);
        sim->rs.fz = (r == 0);
    }
    else
    {
        uint8 r = *(uint8 *) d - *(uint8 *) s;
        sim->rs.fs = (r >> 7);
        sim->rs.fz = (r == 0);
    }
}

// #define execute_zjump(CMP, OFFSET) \
//     do { if (sim->rs.fs CMP 0) } while (false)

void execute_instruction(sim8086 *sim, instruction i)
{
    void *s = 0;
    void *d = 0;
    int32 w = 0;

    if (i.destination.tag == IOP_IMM) d = &i.destination.imm;
    else if (i.destination.tag == IOP_REG) choose_register(sim, i.destination.reg, &d, &w);
    else if (i.destination.tag == IOP_MEM)
    {
        d = sim->memory + i.destination.addr.displacement;

        int32 r1 = 0;
        int32 r2 = 0;
        if (i.destination.addr.reg_count > 0)
        {
            void *reg = 0;
            int32 w_ = 0;
            choose_register(sim, i.destination.addr.reg1, &reg, &w_);

            if (w_) r1 = *(uint16 *) reg;
            else r1 = *(uint8 *) reg;
        }
        if (i.destination.addr.reg_count > 1)
        {
            void *reg = 0;
            int32 w_ = 0;
            choose_register(sim, i.destination.addr.reg2, &reg, &w_);

            if (w_) r2 = *(uint16 *) reg;
            else r2 = *(uint8 *) reg;
        }

        d += (r1 + r2);
    }
    else { printf("Error while executing instruction! (d)\n"); exit(1); }
    if (i.source.tag == IOP_IMM) s = &i.source.imm;
    else if (i.source.tag == IOP_REG) choose_register(sim, i.source.reg, &s, &w);
    else if (i.source.tag == IOP_MEM)
    {
        s = sim->memory + i.source.addr.displacement;

        int32 r1 = 0;
        int32 r2 = 0;
        if (i.source.addr.reg_count > 0)
        {
            void *reg = 0;
            int32 w_ = 0;
            choose_register(sim, i.source.addr.reg1, &reg, &w_);

            if (w_) r1 = *(uint16 *) reg;
            else r1 = *(uint8 *) reg;
        }
        if (i.source.addr.reg_count > 1)
        {
            void *reg = 0;
            int32 w_ = 0;
            choose_register(sim, i.source.addr.reg2, &reg, &w_);

            if (w_) r2 = *(uint16 *) reg;
            else r2 = *(uint8 *) reg;
        }

        s += (r1 + r2);
    }
    // else { printf("Error while executing instruction! (%d)\n", i.source.tag); exit(1); }

    switch (i.tag)
    {
    case I_MOV: execute_mov(sim, d, s, w);  break;
    case I_ADD: execute_add_sub(+); break;
    case I_SUB: execute_add_sub(-); break;
    case I_CMP: execute_cmp(sim, d, s, w); break;
    case I_JE:   // zf == 1
        if (sim->rs.fz == 1) sim->rs.ip += i.destination.imm;
        break;
    // case I_JL:   // (sf xor of) == 1
    // case I_JLE:  // ((sf xor of) or zf) == 1
    // case I_JB:   // cf == 1
    // case I_JBE:  // (cf or zf) == 1
    // case I_JP:   // pf == 1
    // case I_JO:   // of == 1
    //     break;
    case I_JS:   // sf == 1
        if (sim->rs.fs == 1) sim->rs.ip += i.destination.imm;
        break;

    case I_JNE:  // zf == 0
        if (sim->rs.fz == 0) sim->rs.ip += i.destination.imm;
        break;
    // case I_JNL:  // (sf xor of) == 0
    // case I_JNLE: // ((sf xor of) or zf) == 0
    // case I_JNB:  // cf == 0
    // case I_JNBE: // (cf or zf) == 0
    // case I_JNP:  // pf == 0
    // case I_JNO:  // of == 0
    case I_JNS:  // sf == 0
        if (sim->rs.fs == 0) sim->rs.ip += i.destination.imm;
        break;

    default: printf("Cannot execute given instruction!\n");
    }
}

void print_out_compound_register_state(uint16 rx)
{
    uint8 rl = rx >> 8;
    uint8 rh = rx & (0xff);
    print_binary8(rl);
    printf(" ");
    print_binary8(rh);
    printf(" (%d|%d; %d)", rl, rh, rx);
}

void print_out_registers_state(registers *rs)
{
    printf("Registers:\n"
           "    AX: ");
    print_out_compound_register_state(rs->ax);
    printf("\n");
    printf("    BX: ");
    print_out_compound_register_state(rs->bx);
    printf("\n");
    printf("    CX: ");
    print_out_compound_register_state(rs->cx);
    printf("\n");
    printf("    DX: ");
    print_out_compound_register_state(rs->dx);
    printf("\n");
    printf("    SP: ");
    print_binary16(rs->sp);
    printf(" (%d)\n", rs->sp);
    printf("    BP: ");
    print_binary16(rs->bp);
    printf(" (%d)\n", rs->bp);
    printf("    SI: ");
    print_binary16(rs->si);
    printf(" (%d)\n", rs->si);
    printf("    DI: ");
    print_binary16(rs->di);
    printf(" (%d)\n", rs->di);
    printf("    IP: ");
    print_binary16(rs->ip);
    printf(" (%d)\n", rs->ip);
    printf("Flags:\n"
           "       _ _ _ _ O D I T S Z _ A _ P _ C\n"
           "                       %d %d\n",
           rs->fs, rs->fz);
}

void print_out_memory_state(sim8086 *sim, int32 low_addr, int32 high_addr)
{
    while (low_addr < high_addr)
    {
        int32 reminder = low_addr % 16;
        int32 print_address = low_addr - reminder;

        printf("0x%016x | ", print_address);

        // print reminder of bytes

        for (int i = 0; i < 16; i++)
        {
            printf("%02x ", sim->memory[print_address + i]);
        }

        printf(" | ");

        for (int i = 0; i < 16; i++)
        {
            char c = sim->memory[print_address + i];
            bool is_ascii = c > 31 && c < 127;
            printf("%c", is_ascii ? c : '.');
        }

        printf("\n");
        low_addr = print_address + 16;
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("e8086 <binary_input> \n");
        return 1;
    }
    char const *filename = argv[1];

    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("Could not open file \'%s\'\n", filename);
        return 1;
    }

    sim8086 sim =
    {
        .size = 1 << 16,
        .memory = malloc(1 << 16),
    };
    memset(sim.memory, 0, sim.size);

    size_t n = fread(sim.memory, 1, sim.size, f);
    fclose(f);

    // decoding

    fprintf(stdout, "; read %zu bytes\nbits 16\n", n);

    while (sim.rs.ip < n)
    {
        instruction instr = decode_next_instruction(&sim);
        print_instruction(instr);
        execute_instruction(&sim, instr);
    }

    print_out_registers_state(&sim.rs);
    print_out_memory_state(&sim, 999, 1024);

    return 0;
}

