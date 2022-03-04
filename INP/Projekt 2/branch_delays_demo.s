; -------------------------------------------------------------------
; Example program with jumps and "branch delay slots"
;
; Given two values in r1 and r2, find the maximum and writes it to r4
; -------------------------------------------------------------------

.global main

main:     addi r1,r0,10     ; r1 = 10 (0xA)
          addi r2,r0,11     ; r2 = 11 (0xB)
          sgt  r3,r1,r2     ; if r1>r2 then r3=1 else r3 = 0
          bnez r3, true     ; branch if r3 not equals zero
          add  r4,r0,r2
          j end
          ; branch delay slots
          nop
          nop
          ; withou the delay slots, the following instruction
          ; would take effect BEFORE the jump finishes and the
          ; content of r4 would be rewritten by incorrect value
true:     add  r4,r0,r1
end:      trap 0
