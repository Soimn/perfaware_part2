extern Reptest_RoundIsNotDone
extern Reptest_BeginTestSection
extern Reptest_EndTestSection
extern Reptest_AddBytesProcessed

global Test_MovBytewiseLoop
global Test_MovUnrolledBytewiseLoop
global Test_NopBytewiseLoop
global Test_Nop3BytewiseLoop
global Test_Nop9BytewiseLoop
global Test_CmpBytewiseLoop
global Test_DecBytewiseLoop
global Test_MovSameQWordx1Loop
global Test_MovSameQWordx2Loop
global Test_MovSameQWordx3Loop

section .text

Test_MovSameQWordx1Loop:
  align 64
  .loop:
    mov [rdx], rax
    sub rcx, 1
    cmp rcx, 0
    jg .loop
  ret

Test_MovSameQWordx2Loop:
  align 64
  .loop:
    mov [rdx], rax
    mov [rdx], rax
    sub rcx, 2
    cmp rcx, 0
    jg .loop
  ret

Test_MovSameQWordx3Loop:
  align 64
  .loop:
    mov [rdx], rax
    mov [rdx], rax
    mov [rdx], rax
    sub rcx, 3
    cmp rcx, 0
    jg .loop
  ret

Test_MovBytewiseLoop:
  xor rax, rax
  .loop:
    mov [rdx + rax], al
    inc rax
    cmp rax, rcx
    jb .loop
  ret

Test_MovUnrolledBytewiseLoop:
  xor rax, rax
  test rcx, 1
  jz .loop
  mov [rdx + rax], al
  inc rax
  cmp rax, rcx
  jb .loop

  .loop:
    mov [rdx + rax], al
    inc rax
    mov [rdx + rax], al
    inc rax
    cmp rax, rcx
    jb .loop
  ret

Test_NopBytewiseLoop:
  xor rax, rax
  .loop:
    db 0x0F, 0x1F, 0x00 ; 3 byte nop
    inc rax
    cmp rax, rcx
    jb .loop
  ret

Test_Nop3BytewiseLoop:
  xor rax, rax
  .loop:
    nop
    nop
    nop
    inc rax
    cmp rax, rcx
    jb .loop
  ret

Test_Nop9BytewiseLoop:
  xor rax, rax
  .loop:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    inc rax
    cmp rax, rcx
    jb .loop
  ret

Test_CmpBytewiseLoop:
  xor rax, rax
  .loop:
    inc rax
    cmp rax, rcx
    jb .loop
  ret

Test_DecBytewiseLoop:
  xor rax, rax
  .loop:
    dec rcx
    test rcx, rcx
    jnz .loop
  ret
