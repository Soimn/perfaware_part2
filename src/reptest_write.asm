extern Reptest_RoundIsNotDone
extern Reptest_BeginTestSection
extern Reptest_EndTestSection
extern Reptest_AddBytesProcessed

global Test_MovBytewise
global Test_NopBytewise
global Test_CmpBytewise
global Test_DecBytewise

section .text

; void Test_MovBytewise(Reptest* test, Write_Params params)
Test_MovBytewise:
  enter 32, 0

  mov [rsp], rcx

  mov r8, [rdx+0]
  mov [rsp+8], r8
  mov r8, [rdx+8]
  mov [rsp+16], r8

  .round_not_done:
    mov rcx, [rsp]
    call Reptest_RoundIsNotDone
    test al, al
    jz .round_done

    mov rcx, [rsp]
    call Reptest_BeginTestSection
    
    xor rax, rax
    mov rcx, [rsp+8]
    mov rdx, [rsp+16]
    .loop:
      mov byte [rcx + rax], al
      inc rax
      cmp rax, rdx
      jb .loop

    mov rcx, [rsp]
    call Reptest_EndTestSection

    mov rcx, [rsp]
    mov rdx, [rsp+16]
    call Reptest_AddBytesProcessed

    jmp .round_not_done
  .round_done:

  leave
  ret

; void Test_NopBytewise(Reptest* test, Write_Params params)
Test_NopBytewise:
  enter 32, 0

  mov [rsp], rcx

  mov r8, [rdx+0]
  mov [rsp+8], r8
  mov r8, [rdx+8]
  mov [rsp+16], r8

  .round_not_done:
    mov rcx, [rsp]
    call Reptest_RoundIsNotDone
    test al, al
    jz .round_done

    mov rcx, [rsp]
    call Reptest_BeginTestSection
    
    xor rax, rax
    mov rdx, [rsp+8]
    mov rcx, [rsp+16]
    .loop:
      db 0x0F, 0x1F, 0x00 ; 3 Byte NOP
      inc rax
      cmp rax, rcx
      jb .loop

    mov rcx, [rsp]
    call Reptest_EndTestSection

    mov rcx, [rsp]
    mov rdx, [rsp+16]
    call Reptest_AddBytesProcessed

    jmp .round_not_done
  .round_done:

  leave
  ret

; void Test_CmpBytewise(Reptest* test, Write_Params params)
Test_CmpBytewise:
  enter 32, 0

  mov [rsp], rcx

  mov r8, [rdx+0]
  mov [rsp+8], r8
  mov r8, [rdx+8]
  mov [rsp+16], r8

  .round_not_done:
    mov rcx, [rsp]
    call Reptest_RoundIsNotDone
    test al, al
    jz .round_done

    mov rcx, [rsp]
    call Reptest_BeginTestSection
    
    xor rax, rax
    mov rdx, [rsp+8]
    mov rcx, [rsp+16]
    .loop:
      inc rax
      cmp rax, rcx
      jb .loop

    mov rcx, [rsp]
    call Reptest_EndTestSection

    mov rcx, [rsp]
    mov rdx, [rsp+16]
    call Reptest_AddBytesProcessed

    jmp .round_not_done
  .round_done:

  leave
  ret

; void Test_DecBytewise(Reptest* test, Write_Params params)
Test_DecBytewise:
  enter 32, 0

  mov [rsp], rcx

  mov r8, [rdx+0]
  mov [rsp+8], r8
  mov r8, [rdx+8]
  mov [rsp+16], r8

  .round_not_done:
    mov rcx, [rsp]
    call Reptest_RoundIsNotDone
    test al, al
    jz .round_done

    mov rcx, [rsp]
    call Reptest_BeginTestSection
    
    mov rdx, [rsp+8]
    mov rcx, [rsp+16]
    lea rax, [rcx-1]
    .loop:
      dec rax
      cmp rax, rcx
      jb .loop

    mov rcx, [rsp]
    call Reptest_EndTestSection

    mov rcx, [rsp]
    mov rdx, [rsp+16]
    call Reptest_AddBytesProcessed

    jmp .round_not_done
  .round_done:

  leave
  ret
