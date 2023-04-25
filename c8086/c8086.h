typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char i8;
typedef signed short i16;
typedef unsigned short u16;

#define len(a) (sizeof(a)/sizeof(0[a]))

#define DEBUG 0

#if DEBUG == 2
#define dbg(x) __builtin_dump_struct(&(x), fprintf, stderr)
#define debugf(fmt, ...) fprintf(stderr, (fmt), __VA_ARGS__)
#else
#define dbg(...)
#define debugf(...)
#endif

#define unimplemented() assert(!"unimplemented")
#define unreachable() assert(!"unreachable")

struct buffer {
    u8 *data;
    int ndata;
};

enum reg {
    reg_AX,
    reg_CX,
    reg_DX,
    reg_BX,
    reg_SP,
    reg_BP,
    reg_SI,
    reg_DI,

    reg_num,
};

enum op_type {
    op_UNKNOWN,

    op_MOV_RM_TO_REG,
    op_MOV_IMM_TO_RM,
    op_MOV_IMM_TO_REG,
    op_MOV_MEM_TO_ACC,
    op_MOV_ACC_TO_MEM,

    op_ADD,
    op_ADD_IMM_TO_RM,
    op_ADD_IMM_TO_ACC,

    op_SUB,
    op_SUB_IMM_TO_RM,
    op_SUB_IMM_TO_ACC,

    op_CMP,
    op_CMP_IMM_TO_RM,
    op_CMP_IMM_TO_ACC,

    op_JE,
    op_JL,
    op_JLE,
    op_JB,
    op_JBE,
    op_JP,
    op_JO,
    op_JS,
    op_JNE, // jnz
    op_JNL,
    op_JG,
    op_JNB,
    op_JA,
    op_JNP,
    op_JNO,
    op_JNS,
    op_LOOP,
    op_LOOPZ,
    op_LOOPNZ,
    op_JCXZ,
};

struct op {
    enum op_type type;
    u8 d; // 0 - src in reg, 1 - dst in reg
    u8 s; // sign extension
    u8 w; // byte/word
    u8 mod;

    enum reg reg;
    enum reg rm;
    union {
        i8 disp_byte;
        i16 disp;
        i16 addr;
        i8 ip_inc;
    };
    union {
        u8 data_byte;
        u16 data;
    };
};

struct prog {
    struct op ops[128];
    int nops;
};

static inline const char *
reg_name(enum reg reg, u8 word) {
    static const char *word_names[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
    static const char *byte_names[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
    assert(reg < reg_num);
    const char *result = (word ? word_names : byte_names)[reg];
    return result;
}

enum flags {
    flags_CARRY = 0x1,
    flags_PARITY = 0x2,
    flags_AUX_CARRY = 0x4,
    flags_ZERO = 0x8,
    flags_SIGN = 0x10,
    flags_OVERFLOW = 0x20,
};

struct cpu {
    bool powered;
    enum reg regs[reg_num];
    enum flags flags;

    u16 last_ip; // NOTE: beginning of current instruction
    u16 ip;
    u8 *memory;
    u16 nbytes; // NOTE: number of bytes actually read into memory

    bool print_ip;
};
