typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char i8;
typedef signed short i16;

#define DEBUG 0

#if DEBUG
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

enum op {
    op_MOV_RM_TO_REG,
    op_MOV_IMM_TO_RM,
    op_MOV_IMM_TO_REG,
};

struct mov_op {
    enum op op;
    u8 d;
    u8 w;
    u8 mod;
    enum reg reg;
    enum reg r_m;
    union {
        i8 disp_byte;
        i16 disp;
        i8 data_byte;
        i16 data;
    };
};

struct prog {
    struct mov_op ops[64];
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
