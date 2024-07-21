global ReadTest@@24

section .text

ReadTest@@24:
  mov rax, rdx
  add r8, rdx

  align 64
  .loop:
    vmovdqu ymm0, [rax +   0]
    vmovdqu ymm0, [rax +  32]
    vmovdqu ymm0, [rax +  64]
    vmovdqu ymm0, [rax +  96]
    vmovdqu ymm0, [rax + 128]
    vmovdqu ymm0, [rax + 160]
    vmovdqu ymm0, [rax + 192]
    vmovdqu ymm0, [rax + 224]
    vmovdqu ymm0, [rax + 256]
    vmovdqu ymm0, [rax + 288]
    vmovdqu ymm0, [rax + 320]
    vmovdqu ymm0, [rax + 352]
    vmovdqu ymm0, [rax + 384]
    vmovdqu ymm0, [rax + 416]
    vmovdqu ymm0, [rax + 448]
    vmovdqu ymm0, [rax + 480]

    add rax, 512
    cmp rax, r8
    cmovge rax, rdx

    sub rcx, 512
    jnz .loop
  ret
