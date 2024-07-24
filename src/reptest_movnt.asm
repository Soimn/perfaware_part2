global Mov
global MovNT

section .text

Mov:
  movdqu [rcx], ymm0
  ret

MovNT@@:
  movntdq [rcx], ymm0
  ret
