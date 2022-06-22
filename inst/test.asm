format binary
use64

macro pad num {
	rb num - ($ mod num)
}


rq 3
hello: world: dd 0xffffffff, 0xa0, 0x12121212

rb hex
;define hex 7 << 8

db 0x10

.united:
;rb hex - hex
hex = 7 shl 8; - world.united

pad 0x10
mov eax, eax
align 0x10
mov ecx, eax

; pad 0x10
; movss xmm0, xmm0
; pad 0x10
; movss xmm0, xmm8
; pad 0x10
; movss xmm8, xmm0
; pad 0x10
; movss xmm8, xmm8
; pad 0x10
; movss xmm8, xmm15
; pad 0x10
; movss xmm15, xmm8
; pad 0x10
; movss xmm15, xmm15

pad 0x10
cvtss2si rax, xmm0
pad 0x10
cvtss2si rcx, xmm0
pad 0x10
cvtss2si rax, xmm8
pad 0x10
cvtss2si rcx, xmm8
pad 0x10
cvtss2si r8, xmm0
pad 0x10
cvtss2si r9, xmm0
pad 0x10
cvtss2si r8, xmm8
pad 0x10
cvtss2si r9, xmm8


pad 0x10
cvtsi2sd xmm0, rax
pad 0x10
cvtsi2sd xmm8, rax
pad 0x10
cvtsi2sd xmm0, r9
pad 0x10
cvtsi2sd xmm8, r9


;mov r8d, ds
;pad 0x10
;mov eax, [ecx + 90]
