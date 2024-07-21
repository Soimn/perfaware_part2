global AssTest64K@@24

section .text

AssTest64K@@24:
  mov rax, rdx

  align 64
  .loop:
    vmovdqa ymm0, [rax +   0]
    vmovdqa ymm0, [rax +  32]
    vmovdqa ymm0, [rax +  64]
    vmovdqa ymm0, [rax +  96]
    vmovdqa ymm0, [rax + 128]
    vmovdqa ymm0, [rax + 160]
    vmovdqa ymm0, [rax + 192]
    vmovdqa ymm0, [rax + 224]

    add rax, 0x10000
    cmp rax, r8
    cmovge rax, rdx

    dec rcx
    jnz .loop
  ret
