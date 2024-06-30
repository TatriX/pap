global mov_all_bytes_asm

        .section text

        ;; TODO: Figure out ABI!
mov_all_bytes_asm:
        xor rax, rax
loop:
        mov [rdx + rax], al
        inc rax
        cmp rax, rcx,
        jb loop
        ret
