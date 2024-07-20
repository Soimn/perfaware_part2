global ReadAmountMaskedToRange@@24

section .text

; Assumes mask is of the form (1 << i) - 1 where 7 <= i <= 30
ReadAmountMaskedToRange@@24:
  xor rax, rax
  align 64
  .loop:
    mov r9, rax
    and r9, r8
    add r9, rdx

    vmovdqa ymm0, [r9 +  0]
    vmovdqa ymm0, [r9 + 32]
    vmovdqa ymm0, [r9 + 64]
    vmovdqa ymm0, [r9 + 96]
    
    add rax, 128
    cmp rax, rcx
    jl .loop
  ret
