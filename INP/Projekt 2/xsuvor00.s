; DLX
; Artur Suvorkin xsuvor00

        .data 0x04          ; zacatek data segmentu v pameti
login:  .asciiz "xbidlo00"  ; <-- nahradte vasim loginem
cipher: .space 9 ; sem ukladejte sifrovane znaky (za posledni nezapomente dat 0)

        .align 2            ; dale zarovnavej na ctverice (2^2) bajtu
laddr:  .word login         ; 4B adresa vstupniho textu (pro vypis)
caddr:  .word cipher        ; 4B adresa sifrovaneho retezce (pro vypis)

        .text 0x40          ; adresa zacatku programu v pameti
        .global main        ; 

main:   ; r10-r13-r16-r19-r26-r0
addi r16, r0, 0     
addi r19, r0, 9

loop:
	lb r10, login(r16)

	sb cipher(r16), r10

	addi r16, r16, 1
	sgt  r26,r16,r19      ; if r13>r16 then r26=1 else r26 = 0
	bnez r26, next       ; branch if r26 not equals zero
	nop
j loop
	nop
next:
	addi r16, r0, 0     
	addi r16, r16,2	
	lb r10, cipher(r16); xbIdlo must minus meneni B D O
	subi r10, r10, 96	

	addi r16, r0, 0
     	addi r16, r16, 1
	addi r19, r0, 5	
second_loop:
	lb r13, cipher(r16)
	subi r13, r13, 97
	j check
	nop
check_succ:
	addi r13, r13, 97
	addi r19, r0, 5

	sb cipher(r16), r13
	addi r16, r16, 2
	sgt r26, r16 ,r19
	bnez r26, next_next
	nop
j second_loop
	nop

check:
	sgt r26, r10, r13
	beqz r26, bigger
	nop

	sub r26, r10, r13
	addi r19, r0, 26
	sub r13, r19, r26 
	addi r26, r0 , 0
j check_succ
nop
bigger:
	sub r13, r13, r10
j check_succ
nop


next_next:
	addi r16, r0, 0     
	addi r16, r0, 1	
	
	lb r10, login(r16); xBidlo must plus meneni X I L

	subi r10, r10, 96	

	addi r16, r0, 0
	addi r19, r0, 5	
third_loop:
	lb r13, cipher(r16)
	subi r13, r13, 96
	j check_third
	nop
check_succ_third:
	addi r13, r13, 96
	addi r19, r0, 5
	addi r26, r0, 0

	sb cipher(r16), r13
	addi r16, r16, 2
	sgt r26, r16 ,r19
	bnez r26, end
	nop
j third_loop
	nop

check_third:
	addi r19, r0, 26
	add r13, r13, r10
	sgt r26, r13, r19
	bnez r26, smaller_third
	nop
j check_succ_third
nop

smaller_third:
	addi r19, r0, 0
	subi r13, r13, 26
	add r13, r13, r19 
j check_succ_third
nop


	
end:    addi r14, r0, caddr ; <-- pro vypis sifry nahradte laddr adresou caddr
        trap 5  ; vypis textoveho retezce (jeho adresa se ocekava v r14)
        trap 0  ; ukonceni simulace