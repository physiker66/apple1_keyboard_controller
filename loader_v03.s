; loader program for apple 1 replica
; entered via WozMon
; on execution reads out keyboard interface and stores data into RAM
; keyboard interface only uses 7bits, so data is transferred as 2 nibbles
; high nibble comes first
; program expects start address as pointer in F0/F1
; program expects end address as pointer in F2/F3



.segment "ZEROPAGE"


.segment "VARS"
TargetAdress = $00F0        ; start address
FinalAdress  = $00F2
KBDCR        = $D011
KBD          = $D010

.segment "STARTUP"

; ##########################################################################################
start:
;brk
;nop
;nop
    ldy     #0      ; set up y counters

loop:
    lda TargetAdress
    cmp FinalAdress
    bne firstchar
    lda TargetAdress + 1
    cmp FinalAdress + 1
    bne firstchar
    jmp $FF00
firstchar:
    lda KBDCR
    bpl firstchar
    lda KBD			; get high nibble
    clc
    asl
    asl
    asl
    asl
    sta (TargetAdress),y	; store into target
nextchar:
    lda KBDCR
    bpl nextchar
    lda KBD			; get low nibble
    and #%00001111
    clc
    adc (TargetAdress),y	; add high nibble
    sta (TargetAdress),y	; store into target
    inc TargetAdress
    bne loop
    inc TargetAdress + 1
    jmp loop
