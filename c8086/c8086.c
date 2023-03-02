#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "c8086.h"

static struct buffer
load_file(const char *path) {
    struct buffer result;

    FILE *file = fopen(path, "rb");
    assert(file);

    fseek(file, 0, SEEK_END);
    result.ndata = ftell(file);
    fseek(file, 0, SEEK_SET);

    result.data = malloc(result.ndata);
    int nread = fread(result.data, 1, result.ndata, file);
    assert(nread == result.ndata);

    fclose(file);

    return result;
}

static void
free_buffer(struct buffer *buffer) {
    free(buffer->data);
    buffer->data = NULL;
    buffer->ndata = 0;
}

static void
usage(const char *progname) {
    fprintf(stderr, "Usage: %s PATH\n", progname);
}

static struct prog
decode(struct buffer asm_data) {
    struct prog result = {};

    for (int i = 0; i < asm_data.ndata; i += 2) {
        assert(i + 1 < asm_data.ndata);
        u8 first_byte = asm_data.data[i];
        u8 second_byte = asm_data.data[i + 1];

        u8 opcode = first_byte >> 2;
        assert(opcode == 0b100010);

        struct mov_op mov = {
            .op = op_MOV,
            .d = (first_byte >> 1) & 1,
            .w = first_byte & 1,
            .mod = (second_byte >> 5) & 0b111 ,
            .reg = (second_byte >> 3) & 0b111,
            .r_m = second_byte & 0b111,
        };
#if 0
        assert(mov.d == 0);
        assert(mov.w == 1);
        assert(mov.reg == reg_BX);
        assert(mov.r_m == reg_CX);
#endif

        dbg(mov);
        result.ops[result.nops++] = mov;
    }

    return result;
}

int
main(int argc, char *argv[]) {
    if (argc != 2) {
        usage(argv[0]);
        exit(0);
    }

    const char *bin_path = argv[1];
    struct buffer asm_data = load_file(bin_path);
    dbg(asm_data);
    struct prog prog = decode(asm_data);

    printf("; %s\n", bin_path);
    printf("\n");
    printf("bits 16\n");


    for (int i = 0; i < prog.nops; i++) {
        struct mov_op mov = prog.ops[i];
        // NOTE: d == 1 : dst in reg
        enum reg dst = (mov.d) ? mov.reg : mov.r_m;
        enum reg src = (mov.d) ? mov.r_m : mov.reg;
        printf("mov %s, %s\n", reg_name(dst, mov.w), reg_name(src, mov.w));
    }

    printf("\n");

    free_buffer(&asm_data);
}
