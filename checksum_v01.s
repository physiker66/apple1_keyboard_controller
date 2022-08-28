; checksum program for apple 1 replica
; entered via WozMon
; on execution reads out keyboard interface and stores data into RAM
; keyboard interface only uses 7bits, so data is transferred as 2 nibbles
; high nibble comes first
; program expects start address as pointer in F0/F1
; program expects end address as pointer in F2/F3



.segment "ZEROPAGE"


.segment "VARS"
StartAddress = $00F0       
StopAddress  = $00F2
KBDCR        = $D011
KBD          = $D010
DSP          = $D012
PRINTBYTE    = $FFDC
ECHO         = $FFEF

.segment "STARTUP"

; ##########################################################################################
start:
;brk
;nop
;nop
    ldy #0      ; set up y counters
    lda #0

@loop:
    tax
    lda StartAddress
    cmp StopAddress
    bne @loop1
    lda StartAddress + 1
    cmp StopAddress + 1
    bne @loop1
    jmp $FF00

@loop1:
    txa
    clc
    adc (StartAddress),y	; add next byte
    inc StartAddress
    bne @loop
    jsr PRINTBYTE
    lda #$A0
    jsr ECHO
    lda #0
    inc StartAddress + 1
    jmp @loop
