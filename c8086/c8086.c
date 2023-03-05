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
    switch (mov.r_m) {
    case 0b000: printf("bx + si"); break;
    case 0b001: printf("bx + di"); break;
    case 0b010: printf("bp + si"); break;
    case 0b011: printf("bp + di"); break;
    case 0b100: printf("si"); break;
    case 0b101: printf("di"); break;
    case 0b110: {
        if (mov.mod == 0b00) {
            printf("%d", mov.addr);
        } else {
            printf("bp");
        }
    } break;
    case 0b111: printf("bx"); break;
    default: unreachable();
    }

    if ((mov.mod == 0b01) || (mov.mod == 0b10)) {
        i16 disp = (mov.mod == 0b10) ? mov.disp : mov.disp_byte;
        if (disp == 0) {
        } else if (disp > 0) {
            printf(" + %d",  disp);
        } else {
            printf(" - %d",  -disp);
        }
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
        printf("mov ");
        print_mov_address(mov);
        printf(", %s %d", (mov.w) ? "word" : "byte", mov.data);
        printf("\n");
    } break;
    case op_MOV_ACC_TO_MEM: {
        printf("mov [%d], ax\n", mov.addr);
    } break;
    case op_MOV_MEM_TO_ACC: {
        printf("mov ax, [%d]\n", mov.addr);
    } break;
    case op_MOV_IMM_TO_REG: {
        printf("mov %s, %d\n", reg_name(mov.reg, mov.w), (mov.w) ? mov.data : mov.data_byte);
    } break;
    }
}

static void
prog_add_mov_op(struct prog *prog, struct mov_op mov) {
#if DEBUG
    dbg(mov);
    print_mov(mov);
#endif
    assert(prog->nops < len(prog->ops));
    prog->ops[prog->nops++] = mov;
}

static u8
decode_next(struct decoder *decoder) {
    assert(decoder->byte != decoder->end);
    u8 result = *decoder->byte++;
    return result;
}

static void
decode_disp(struct decoder *decoder, struct mov_op *mov) {
    if (mov->mod == 0b00) {
        if (mov->r_m == 0b110) {
            u8 low = decode_next(decoder);
            u8 high = decode_next(decoder);
            mov->disp = (high << 8) | low;
        }
    } else if (mov->mod == 0b01) {
        mov->disp = decode_next(decoder);
    } else if (mov->mod == 0b10) {
        u8 low = decode_next(decoder);
        u8 high = decode_next(decoder);
        mov->disp = (high << 8) | low;
    }
}

static void
decode_data(struct decoder *decoder, struct mov_op *mov) {
    mov->data = decode_next(decoder);
    if (mov->w) {
        u8 high = decode_next(decoder);
        mov->data |= (high << 8);
    }
}

static struct prog
decode(struct buffer asm_data) {
    struct prog result = {};

    struct decoder decoder = {
        .byte = asm_data.data,
        .end = asm_data.data + asm_data.ndata,
    };

    while (decoder.byte != decoder.end) {
        u8 first_byte = decode_next(&decoder);

        switch (first_byte >> 4) {
        case 0b1000: {
            // NOTE: Register/memory to/from register
            assert(((first_byte >> 2) & 0b11) == 0b10);

            u8 second_byte = decode_next(&decoder);
            struct mov_op mov = {
                .op = op_MOV_RM_TO_REG,
                .d = (first_byte >> 1) & 1,
                .w = first_byte & 1,
                .mod = (second_byte >> 6) & 0b11,
                .reg = (second_byte >> 3) & 0b111,
                .r_m = second_byte & 0b111,
            };
            decode_disp(&decoder, &mov);
            prog_add_mov_op(&result, mov);
        } break;
        case 0b1100: {
            // NOTE: Immediate to register/memory
            assert(((first_byte >> 1) & 0b111) == 0b011);

            u8 second_byte = decode_next(&decoder);
            struct mov_op mov = {
                .op = op_MOV_IMM_TO_RM,
                .w = first_byte & 0b1,
                .mod = (second_byte >> 6) & 0b11,
                .r_m = second_byte & 0b111,
            };
            decode_disp(&decoder, &mov);
            decode_data(&decoder, &mov);
            prog_add_mov_op(&result, mov);
        } break;
        case 0b1011: {
            // NOTE: Immediate to register
            struct mov_op mov = {
                .op = op_MOV_IMM_TO_REG,
                .w = first_byte & 0b1000,
                .reg = first_byte & 0b111,
            };
            decode_data(&decoder, &mov);
            prog_add_mov_op(&result, mov);
        } break;
        case 0b1010: {
            // NOTE: Memory to accumulator
            // NOTE: Accumulator to memory
            struct mov_op mov = {
                .w = first_byte & 0b1,
                .reg = reg_AX,
            };

            switch ((first_byte & 0b1110)) {
            case 0b0000: mov.op = op_MOV_MEM_TO_ACC; break;
            case 0b0010: mov.op = op_MOV_ACC_TO_MEM; break;
            default: unreachable();
            }

            u8 addr_low = decode_next(&decoder);
            u8 addr_hi = decode_next(&decoder);
            mov.addr = (addr_hi << 8) | addr_low;
            prog_add_mov_op(&result, mov);
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
