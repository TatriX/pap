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

static void
print_mov_address(struct mov_op mov) {
    printf("[");
    assert(mov.mod != 0b00 || mov.r_m != 110);
    switch (mov.r_m) {
    case 0b000: printf("bx + si"); break;
    case 0b001: printf("bx + di"); break;
    case 0b010: printf("bp + si"); break;
    case 0b011: printf("bp + di"); break;
    case 0b100: printf("si"); break;
    case 0b101: printf("di"); break;
    case 0b110: printf("bp"); break;
    case 0b111: printf("bx"); break;
    default: unreachable();
    }
    if (mov.mod != 0b00 && mov.disp != 0) {
        printf(" + %d", mov.disp);
    }
    printf("]");
}

static void
print_mov(struct mov_op mov) {
    // NOTE: d == 1 : dst in reg
    switch (mov.op) {
    case op_MOV_RM_TO_REG: {
        printf("mov ");
        if (mov.mod == 0b11) {
            enum reg dst = (mov.d) ? mov.reg : mov.r_m;
            enum reg src = (mov.d) ? mov.r_m : mov.reg;
            printf("%s, %s", reg_name(dst, mov.w), reg_name(src, mov.w));
        } else {
            if (mov.d) {
                printf("%s", reg_name(mov.reg, mov.w));
                printf(", ");
                print_mov_address(mov);
            } else {
                print_mov_address(mov);
                printf(", ");
                printf("%s", reg_name(mov.reg, mov.w));
            }
        }
        printf("\n");
    } break;
    case op_MOV_IMM_TO_RM: {
        unimplemented();
    } break;
    case op_MOV_IMM_TO_REG: {
        printf("mov %s, %d\n", reg_name(mov.reg, mov.w), (mov.w) ? mov.data : mov.data_byte);
    } break;
    }
}

static struct prog
decode(struct buffer asm_data) {
    struct prog result = {};

    u8 *byte = asm_data.data;
    u8 *end = asm_data.data + asm_data.ndata;
    while (byte != end) {
        u8 first_byte = *byte++;
        assert(byte != end);
        u8 second_byte = *byte++;

        switch (first_byte >> 4) {
        case 0b1000: {
            // NOTE: Register/memory to/from register
            assert(((first_byte >> 2) & 0b11) == 0b10);
            struct mov_op mov = {
                .op = op_MOV_RM_TO_REG,
                .d = (first_byte >> 1) & 1,
                .w = first_byte & 1,
                .mod = (second_byte >> 6) & 0b11,
                .reg = (second_byte >> 3) & 0b111,
                .r_m = second_byte & 0b111,
            };
            if (mov.mod == 0b00) {
                assert(mov.r_m != 0b110);
            } else if (mov.mod == 0b01) {
                assert(byte != end);
                u8 third_byte = *byte++;
                mov.disp = third_byte;
            } else if (mov.mod == 0b10) {
                assert(byte != end);
                u8 third_byte = *byte++;
                assert(byte != end);
                u8 forth_byte = *byte++;
                mov.disp = (forth_byte << 8) | third_byte;
            }
#if DEBUG
            dbg(mov);
            print_mov(mov);
#endif
            assert(result.nops < len(result.ops));
            result.ops[result.nops++] = mov;
        } break;
        case 0b1100: {
            // NOTE: Immediate to register/memory
            assert(((first_byte >> 1) & 0b111) == 0b011);
            unimplemented();
        } break;
        case 0b1011: {
            // NOTE: Immediate to register
            struct mov_op mov = {
                .op = op_MOV_IMM_TO_REG,
                .w = first_byte & 0b1000,
                .reg = first_byte & 0b111,
            };
            if (mov.w) {
                assert(byte != end);
                u8 third_byte = *byte++;
                mov.data = (third_byte << 8) | second_byte;
            } else {
                mov.data = second_byte;
            }
#if DEBUG
            dbg(mov);
            print_mov(mov);
#endif
            assert(result.nops < len(result.ops));
            result.ops[result.nops++] = mov;
        } break;
        default:
            assert(!"Unsupported op");
        }
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
        print_mov(prog.ops[i]);
    }

    printf("\n");

    free_buffer(&asm_data);
}
