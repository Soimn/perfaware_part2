global ReadRangeNTimes@@24
global ReadRangeNTimesSingle@@24

section .text

ReadRangeNTimesSingle@@24:
  xor r9, r9
  xor r10, r10
  align 64
  .loop:
    lea rax, [rdx + r9]

    vmovdqa ymm0, [rax +   0]
    vmovdqa ymm0, [rax +  32]
    vmovdqa ymm0, [rax +  64]
    vmovdqa ymm0, [rax +  96]
    vmovdqa ymm0, [rax + 128]
    vmovdqa ymm0, [rax + 160]
    vmovdqa ymm0, [rax + 192]
    vmovdqa ymm0, [rax + 224]
    vmovdqa ymm0, [rax + 256]
    vmovdqa ymm0, [rax + 288]
    vmovdqa ymm0, [rax + 320]
    vmovdqa ymm0, [rax + 352]
    vmovdqa ymm0, [rax + 384]
    vmovdqa ymm0, [rax + 416]
    vmovdqa ymm0, [rax + 448]
    vmovdqa ymm0, [rax + 480]

    add r9, 512
    cmp r9, r8
    cmovge r9, r10
    
    sub rcx, 512
    jnz .loop
  ret
