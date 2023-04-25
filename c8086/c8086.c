#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c8086.h"

static struct buffer
load_file(const char *path) {
    struct buffer result;

    FILE *file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Cannot load '%s'\n", path);
        abort();
    }

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
    fprintf(stderr, "Usage: %s PATH [-exec]\n", progname);
}

static void
print_op_address(struct op op) {
    printf("[");
    switch (op.rm) {
    case 0b000: printf("bx + si"); break;
    case 0b001: printf("bx + di"); break;
    case 0b010: printf("bp + si"); break;
    case 0b011: printf("bp + di"); break;
    case 0b100: printf("si"); break;
    case 0b101: printf("di"); break;
    case 0b110: {
        if (op.mod == 0b00) {
            printf("%d", op.addr);
        } else {
            printf("bp");
        }
    } break;
    case 0b111: printf("bx"); break;
    default: unreachable();
    }

    if ((op.mod == 0b01) || (op.mod == 0b10)) {
        i16 disp = (op.mod == 0b10) ? op.disp : op.disp_byte;
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
print_rm_to_reg(struct op op) {
    if (op.mod == 0b11) {
        enum reg dst = (op.d) ? op.reg : op.rm;
        enum reg src = (op.d) ? op.rm : op.reg;
        printf("%s, %s", reg_name(dst, op.w), reg_name(src, op.w));
    } else {
        if (op.d) {
            printf("%s", reg_name(op.reg, op.w));
            printf(", ");
            print_op_address(op);
        } else {
            print_op_address(op);
            printf(", ");
            printf("%s", reg_name(op.reg, op.w));
        }
    }
}

static void
print_im_to_reg(struct op op) {
    if (op.mod == 0b11) {
        printf("%s", reg_name(op.rm, op.w));
    } else {
        printf("%s ", (op.w) ? "word" : "byte");
        print_op_address(op);
    }
}

static void
print_op(struct op op) {
    // NOTE: d == 1 : dst in reg
    switch (op.type) {
    case op_UNKNOWN: {
        assert(!"Cannot print uknown op");
    } break;
    case op_MOV_RM_TO_REG: {
        printf("mov ");
        print_rm_to_reg(op);
    } break;
    case op_MOV_IMM_TO_RM: {
        printf("mov ");
        print_op_address(op);
        printf(", %s %d", (op.w) ? "word" : "byte", op.data);
    } break;
    case op_MOV_ACC_TO_MEM: {
        printf("mov [%d], ax", op.addr);
    } break;
    case op_MOV_MEM_TO_ACC: {
        printf("mov ax, [%d]", op.addr);
    } break;
    case op_MOV_IMM_TO_REG: {
        printf("mov %s, %d", reg_name(op.reg, op.w), (op.w) ? op.data : op.data_byte);
    } break;
    case op_ADD: {
        printf("add ");
        print_rm_to_reg(op);
    } break;
    case op_ADD_IMM_TO_RM: {
        printf("add ");
        print_im_to_reg(op);
        printf(", %d", op.data);
    } break;
    case op_ADD_IMM_TO_ACC: {
        printf("add %s, %d", reg_name(0, op.w), op.data);
    } break;
    case op_SUB: {
        printf("sub ");
        print_rm_to_reg(op);
    } break;
    case op_SUB_IMM_TO_RM: {
        printf("sub ");
        print_im_to_reg(op);
        printf(", %d", op.data);
    } break;
    case op_SUB_IMM_TO_ACC: {
        printf("sub %s, %d", reg_name(0, op.w), op.data);
    } break;
    case op_CMP: {
        printf("cmp ");
        print_rm_to_reg(op);
    } break;
    case op_CMP_IMM_TO_RM: {
        printf("cmp ");
        print_im_to_reg(op);
        printf(", %d", op.data);
    } break;
    case op_CMP_IMM_TO_ACC: {
        printf("cmp %s, %d", reg_name(0, op.w), op.data);
    } break;
    case op_JE: printf("je $%+d", op.ip_inc); break;
    case op_JL: printf("jl $%+d", op.ip_inc); break;
    case op_JLE: printf("jle $%+d", op.ip_inc); break;
    case op_JB: printf("jb $%+d", op.ip_inc); break;
    case op_JBE: printf("jbe $%+d", op.ip_inc); break;
    case op_JP: printf("jp $%+d", op.ip_inc); break;
    case op_JO: printf("jo $%+d", op.ip_inc); break;
    case op_JS: printf("js $%+d", op.ip_inc); break;
    case op_JNE: printf("jne $%+d", op.ip_inc); break;
    case op_JNL: printf("jnl $%+d", op.ip_inc); break;
    case op_JG: printf("jg $%+d", op.ip_inc); break;
    case op_JNB: printf("jnb $%+d", op.ip_inc); break;
    case op_JA: printf("ja $%+d", op.ip_inc); break;
    case op_JNP: printf("jnp $%+d", op.ip_inc); break;
    case op_JNO: printf("jno $%+d", op.ip_inc); break;
    case op_JNS: printf("jns $%+d", op.ip_inc); break;
    case op_LOOP: printf("loop $%+d", op.ip_inc); break;
    case op_LOOPZ: printf("loopz $%+d", op.ip_inc); break;
    case op_LOOPNZ: printf("loopnz $%+d", op.ip_inc); break;
    case op_JCXZ: printf("jcxz $%+d", op.ip_inc); break;
    }
}

static u8
decode_next(struct decoder *decoder) {
    assert(decoder->byte != decoder->end);
    u8 result = *decoder->byte++;
    return result;
}

static void
decode_disp(struct decoder *decoder, struct op *op) {
    if (op->mod == 0b00) {
        if (op->rm == 0b110) {
            u8 low = decode_next(decoder);
            u8 high = decode_next(decoder);
            op->disp = (high << 8) | low;
        }
    } else if (op->mod == 0b01) {
        op->disp = decode_next(decoder);
    } else if (op->mod == 0b10) {
        u8 low = decode_next(decoder);
        u8 high = decode_next(decoder);
        op->disp = (high << 8) | low;
    }
}

static void
decode_data(struct decoder *decoder, struct op *op) {
    op->data = decode_next(decoder);
    if (!op->s && op->w) {
        u8 high = decode_next(decoder);
        op->data |= (high << 8);
    }
}

static void
decode_mod_reg_rm(struct decoder *decoder, struct op *op) {
    u8 second_byte = decode_next(decoder);
    op->mod = (second_byte >> 6) & 0b11;
    op->reg = (second_byte >> 3) & 0b111;
    op->rm =  (second_byte >> 0) & 0b111;
}

static void
flags_print(enum flags flags) {
    if (flags & flags_CARRY) printf("C");
    if (flags & flags_PARITY) printf("P");
    if (flags & flags_AUX_CARRY) printf("A");
    if (flags & flags_ZERO) printf("Z");
    if (flags & flags_SIGN) printf("S");
    if (flags & flags_OVERFLOW) printf("O");
}

static void
cpu_advance_ip(struct cpu *cpu) {
    u16 old_ip = cpu->ip;
    cpu->ip = cpu->decoder->byte - cpu->memory;
    if (cpu->print_ip) {
        printf(" ip:0x%x->0x%x", old_ip, cpu->ip);
    }
}

static void
cpu_exec(struct cpu *cpu, struct op op) {
    printf(" ; ");

    enum flags old_flags = cpu->flags;

    switch (op.type) {
    case op_MOV_IMM_TO_REG: {
        u16 old = cpu->regs[op.reg];
        u16 new = cpu->regs[op.reg] = op.data;
        printf("%s:0x%x->0x%x", reg_name(op.reg, op.w), old, new);
    } break;
    case op_MOV_RM_TO_REG: {
        enum reg dst = (op.d) ? op.reg : op.rm;
        enum reg src = (op.d) ? op.rm : op.reg;
        u16 old = cpu->regs[dst];
        u16 new = cpu->regs[dst] = cpu->regs[src];
        printf("%s:0x%x->0x%x", reg_name(dst, op.w), old, new);
    } break;
    case op_ADD: {
        enum reg dst = (op.d) ? op.reg : op.rm;
        enum reg src = (op.d) ? op.rm : op.reg;
        u16 old = cpu->regs[dst];
        u16 new = old + cpu->regs[src];
        cpu->regs[op.reg] = new;
        printf("%s:0x%x->0x%x", reg_name(dst, op.w), old, new);
    } break;
    case op_ADD_IMM_TO_RM: {
        u16 old = cpu->regs[op.rm];
        u16 new = cpu->regs[op.rm] + op.data;
        cpu->regs[op.rm] = new;
        bool aux_carry = ((old & 0xf) + (op.data & 0xf)) > 0xf;
        if (aux_carry) {
            cpu->flags |= flags_AUX_CARRY;
        } else {
            cpu->flags &= ~flags_AUX_CARRY;
        }
        printf("%s:0x%x->0x%x", reg_name(op.rm, op.w), old, new);
    } break;
    case op_SUB: {
        enum reg dst = (op.d) ? op.reg : op.rm;
        enum reg src = (op.d) ? op.rm : op.reg;
        u16 old = cpu->regs[dst];
        u16 sub = cpu->regs[src];
        u16 new = old - sub;
        cpu->regs[dst] = new;
        if ((i16)new < 0) {
            cpu->flags |= flags_SIGN;
        }
        bool carry = (old & 0xff) < (sub & 0xff);
        if (carry) {
            cpu->flags |= flags_CARRY;
        } else {
            cpu->flags &= ~flags_CARRY;
        }
        bool aux_carry = ((old & 0xf) + (op.data & 0xf)) > 0xf;
        if (aux_carry) {
            cpu->flags |= flags_AUX_CARRY;
        } else {
            cpu->flags &= ~flags_AUX_CARRY;
        }
        printf("%s:0x%x->0x%x", reg_name(dst, op.w), old, new);

    } break;
    case op_SUB_IMM_TO_RM: {
        u16 old = cpu->regs[op.rm];
        u16 new = old - op.data;
        cpu->regs[op.rm] = new;
        if ((i16)new < 0) {
            cpu->flags |= flags_SIGN;
        }
        if (new == 0) {
            cpu->flags |= flags_ZERO;
            cpu->flags |= flags_PARITY;
        }
        printf("%s:0x%x->0x%x", reg_name(op.rm, op.w), old, new);
    } break;
    case op_CMP: {
        enum reg dst = (op.d) ? op.reg : op.rm;
        enum reg src = (op.d) ? op.rm : op.reg;
        i16 diff = cpu->regs[dst] - cpu->regs[src];

        cpu->flags &= ~flags_SIGN;
        if (diff == 0) {
            cpu->flags |= flags_ZERO;
            cpu->flags |= flags_PARITY;
        }
    } break;
    default:
        printf("skipping %d", op.type);
    }

    cpu_advance_ip(cpu);

    if (old_flags != cpu->flags) {
        printf(" flags:");
        flags_print(old_flags);
        printf("->");
        flags_print(cpu->flags);
    }
}

static void
cpu_add_and_exec(struct cpu *cpu, struct prog *prog, struct op op) {
#if DEBUG
    dbg(op);
#endif
    print_op(op);
    if (cpu->powered) {
        cpu_exec(cpu, op);
    }
    printf("\n");

    assert(prog->nops < len(prog->ops));
    prog->ops[prog->nops++] = op;
}


static struct prog
cpu_decode(struct cpu *cpu, struct buffer asm_data) {
    struct prog result = {};

    struct decoder decoder = {
        .byte = asm_data.data,
        .end = asm_data.data + asm_data.ndata,
    };

    cpu->memory = asm_data.data;
    cpu->decoder = &decoder;

    while (decoder.byte != decoder.end) {
        u8 first_byte = decode_next(&decoder);
        u8 opcode = first_byte >> 2;
        switch (opcode) {
        case 0b100010: {
            // NOTE: mov Register/memory to/from register
            struct op op = {
                .type = op_MOV_RM_TO_REG,
                .d = (first_byte >> 1) & 1,
                .w = first_byte & 1,
            };
            decode_mod_reg_rm(&decoder, &op);
            decode_disp(&decoder, &op);
            cpu_add_and_exec(cpu, &result, op);
        } break;
        case 0b110001: {
            // NOTE: mov Immediate to register/memory
            assert(((first_byte >> 1) & 0b1) == 0b1);

            struct op op = {
                .type = op_MOV_IMM_TO_RM,
                .w = first_byte & 1,
            };
            decode_mod_reg_rm(&decoder, &op);
            assert(op.reg == 0);
            decode_disp(&decoder, &op);
            decode_data(&decoder, &op);
            cpu_add_and_exec(cpu, &result, op);
        } break;
        case 0b101000: {
            // NOTE: mov Memory to accumulator
            // NOTE: mov Accumulator to memory
            struct op op = {
                .w = first_byte & 1,
                .reg = reg_AX,
            };

            switch ((first_byte & 0b1110)) {
            case 0b0000: op.type = op_MOV_MEM_TO_ACC; break;
            case 0b0010: op.type = op_MOV_ACC_TO_MEM; break;
            default: unreachable();
            }

            u8 addr_low = decode_next(&decoder);
            u8 addr_hi = decode_next(&decoder);
            op.addr = (addr_hi << 8) | addr_low;
            cpu_add_and_exec(cpu, &result, op);
        } break;
        case 0b000000: {
            // NOTE: add register/memory with register to either
            struct op op = {
                .type = op_ADD,
                .d = (first_byte >> 1) & 1,
                .w = first_byte & 1,
            };
            decode_mod_reg_rm(&decoder, &op);
            decode_disp(&decoder, &op);
            cpu_add_and_exec(cpu, &result, op);
        } break;
        case 0b100000: {
            struct op op = {
                .s = (first_byte >> 1) & 1,
                .w = first_byte & 1,
            };
            decode_mod_reg_rm(&decoder, &op);
            switch (op.reg) {
            case 0b000:
                // NOTE: add immediate to register/memory
                op.type = op_ADD_IMM_TO_RM;
                break;
            case 0b101:
                // NOTE: sub immediate to register/memory
                op.type = op_SUB_IMM_TO_RM;
                break;
            case 0b111:
                // NOTE: cmp immediate with register/memory
                op.type = op_CMP_IMM_TO_RM;
                break;
            default:
                unimplemented();
            }

            decode_disp(&decoder, &op);
            decode_data(&decoder, &op);
            cpu_add_and_exec(cpu, &result, op);
        } break;
        case 0b000001: {
            // NOTE: add immediate to accumulator
            struct op op = {
                .type = op_ADD_IMM_TO_ACC,
                .w = first_byte & 1,
            };
            decode_data(&decoder, &op);
            cpu_add_and_exec(cpu, &result, op);
        } break;
        case 0b001010: {
            // NOTE: sub register/memory with register to either
            struct op op = {
                .type = op_SUB,
                .d = (first_byte >> 1) & 1,
                .w = first_byte & 1,
            };
            decode_mod_reg_rm(&decoder, &op);
            decode_disp(&decoder, &op);
            cpu_add_and_exec(cpu, &result, op);
        } break;
        case 0b001011: {
            // NOTE: sub immediate to accumulator
            struct op op = {
                .type = op_SUB_IMM_TO_ACC,
                .w = first_byte & 1,
            };
            decode_data(&decoder, &op);
            cpu_add_and_exec(cpu, &result, op);
        } break;

        case 0b001110: {
            // NOTE: cmp register/memory and register
            struct op op = {
                .type = op_CMP,
                .d = (first_byte >> 1) & 1,
                .w = first_byte & 1,
            };
            decode_mod_reg_rm(&decoder, &op);
            decode_disp(&decoder, &op);
            cpu_add_and_exec(cpu, &result, op);
        } break;
        case 0b001111: {
            // NOTE: cmp immediate with accumulator
            struct op op = {
                .type = op_CMP_IMM_TO_ACC,
                .w = first_byte & 1,
            };
            decode_data(&decoder, &op);
            cpu_add_and_exec(cpu, &result, op);
        } break;
        default: {
            enum op_type jumps[255] = {
                [0b01110100] = op_JE,
                [0b01111100] = op_JL,
                [0b01111110] = op_JLE,
                [0b01110010] = op_JB,
                [0b01110110] = op_JBE,
                [0b01111010] = op_JP,
                [0b01110000] = op_JO,
                [0b01111000] = op_JS,
                [0b01110101] = op_JNE,
                [0b01111101] = op_JNL,
                [0b01111111] = op_JG,
                [0b01110011] = op_JNB,
                [0b01110111] = op_JA,
                [0b01111011] = op_JNP,
                [0b01110001] = op_JNO,
                [0b01111001] = op_JNS,
                [0b11100010] = op_LOOP,
                [0b11100001] = op_LOOPZ,
                [0b11100000] = op_LOOPNZ,
                [0b11100011] = op_JCXZ,
            };
            if (jumps[first_byte]) {
                struct op op = {
                    .type = jumps[first_byte],
                    .ip_inc = decode_next(&decoder) + 2, // NOTE: add op offset
                };
                cpu_add_and_exec(cpu, &result, op);
            } else if ((first_byte >> 4) == 0b1011) {
                // NOTE: mov Immediate to register
                struct op op = {
                    .type = op_MOV_IMM_TO_REG,
                    .w = first_byte & 0b1000,
                    .reg = first_byte & 0b111,
                };
                decode_data(&decoder, &op);
                cpu_add_and_exec(cpu, &result, op);
                break;
            } else {
                assert(!"Unsupported op");
            }
        } break;
        }
    }

    return result;
}

static void
cpu_print_regs(struct cpu *cpu) {
    printf("Final registers:\n");
    enum reg order[reg_num] = {
        reg_AX,
        reg_BX,
        reg_CX,
        reg_DX,
        reg_SP,
        reg_BP,
        reg_SI,
        reg_DI,
    };
    for (int i = 0; i < reg_num; i++) {
        enum reg reg = order[i];
        u16 value = cpu->regs[reg];
        if (value) {
            printf("\t%s: 0x%04x (%d)\n", reg_name(reg, 1), value, value);
        }
    }
    if (cpu->print_ip) {
        printf("\tip: 0x%04x (%d)\n", cpu->ip, cpu->ip);
    }
    if (cpu->flags) {
        printf("\tflags: ");
        flags_print(cpu->flags);
        printf("\n");
    }
}

int
main(int argc, char *argv[]) {
    if (argc < 2) {
        usage(argv[0]);
        exit(0);
    }

    struct cpu cpu = {};
    if (argc > 2) {
        if (strcmp(argv[2], "-exec")) {
            usage(argv[0]);
            exit(0);
        }

        cpu.powered = true;

        if (argc > 3 && strcmp(argv[3], "-print-ip") == 0) {
            cpu.print_ip = true;
        }
    }

    const char *bin_path = argv[1];
    struct buffer asm_data = load_file(bin_path);
    dbg(asm_data);

    if (cpu.powered) {
        printf("--- %s execution ---\n", bin_path);
    } else {
        printf("; %s\n", bin_path);
        printf("\n");
        printf("bits 16\n");
    }

    cpu_decode(&cpu, asm_data);
    printf("\n");

    if (cpu.powered) {
        cpu_print_regs(&cpu);
    }

    free_buffer(&asm_data);
}
