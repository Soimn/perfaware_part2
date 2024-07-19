global Test_ReadFullSpeed@@16

section .text

Test_ReadFullSpeed@@16:
  xor rax, rax

  align 64
  .loop:
    vmovdqa ymm0, [rdx+rax]
    vmovdqa ymm0, [rdx+rax + 1024]
    add rax, 2048
    cmp rax, rcx
    jl .loop
  ret
