typedef unsigned char u8;

#define DEBUG 0

#if DEBUG
#define dbg(x) __builtin_dump_struct(&(x), fprintf, stderr)
#define debugf(fmt, ...) fprintf(stderr, (fmt), __VA_ARGS__)
#else
#define dbg(...)
#define debugf(...)
#endif

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
    op_MOV,
};

struct mov_op {
    enum op op;
    u8 d;
    u8 w;
    u8 mod;
    enum reg reg;
    enum reg r_m;
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
