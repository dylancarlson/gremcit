; --------------------------------------------------------------------- ;
; auxmem.asm			Acit				92Apr21 ;
;		asm support for ems/xms/virtual memory			;
; ---------------------------------------------------------------------

; --------------------------------------------------------------------- ;
; these routines support msgtab.c. they implement the low-level link to ;
; xms/ems/virtual memory that would be goofy to do in c. written for	;
; borland's tasm 2.0.                                                   ;
;									;
; BiB/92Apr21								;
; ----------------------------------------------------------------------;

; --------------------------------------------------------------------- ;
;				contents				;
;									;
; setUpAuxMem() 	inits memory for our use			;
; addToEMS()		adds another chunk to our ems pool		;
; addToVirt()		adds another chunk to our virtual pool		;
; addToXMS()		adds another chunk to our xms pool		;
; disposeAuxMem()	releases any allocated memory			;
; HeapToXMS()		transfers memory from Heap to XMS		;
; HeapToEMS()		transfers memory from Heap to EMS		;
; HeapToVirt()		transfers memory from Heap to Virtual		;
; EMStoHeap()		transfers memory from EMS to Heap		;
; VirttoHeap()		transfers memory from Virtual to Heap		;
; XMStoHeap()		transfers memory from XMS to Heap		;
; --------------------------------------------------------------------- ;

ideal
model large, c

public setUpAuxMem, addToXMS, addToEMS, addToVirt, disposeAuxMem, XMStoHeap
public EMStoHeap, VirttoHeap, HeapToXMS, HeapToEMS, HeapToVirt

public xmsaddr, usexms, xmshand, xmssize, emsframe, emshand, emssize, vrthand
public vrtpags
dataseg

xmsaddr dd      0
usexms	db	0
xmshand dw	0
xmssize dw	0	; in Kbytes

xmsxfr	dd	16384	; for xms func 0bh
srchand dw	0
srcofs	dd	0
dsthand dw	0
dstofs	dd	0

emmname db	'EMMXXXX0'
emsframe dw	0
emshand dw	0
emssize dw	0	; in pages (16K)

vrthand dw	0
vrtpags dw	0

codeseg
; inits memory for our use
proc setUpAuxMem uses si di, vmofs:word, vmseg:word
	; verify existance of xms memory
	mov ax, 4300h
	int 2fh
	cmp al, 80h
	jne no_xms

	mov ax, 4310h
	int 2fh
	mov [word ptr xmsaddr], bx
	mov [word ptr xmsaddr+2], es
	mov [usexms], 1

no_xms: ; now, verify existance of ems memory
	mov ax, 3567h
	int 21h
	mov di, 10
	mov si, offset emmname
	mov cx, 8
	cld
	repz cmpsb
	jne no_ems

	mov ah, 40h	; test ems status
	int 67h
	or ah, ah
	jne no_ems

	mov ah, 46h
	int 67h
	or ah, ah
	jne no_ems
	cmp al, 40h
	jb no_ems	; need ems 4.0+

	mov ah, 41h
	int 67h
	or ah, ah
	jnz no_ems
	mov [emsframe], bx

no_ems: ; now, create virtual memory
	push ds
	mov dx, [vmofs]
	mov ax, [vmseg]
	mov ds, ax
	mov ah, 3ch
	xor cx, cx
	int 21h
	pop ds
	jc no_vrt

	mov [vrthand], ax

no_vrt: ; bad file name or something
	ret
endp


proc addToXMS
	cmp [usexms], 0
	je noAddXMS

	cmp [xmshand], 0
	jne reallocxms

	; allocate new handle
	mov ah, 9
	mov dx, 16
	call [xmsaddr]
	or ax, ax
	je noAddXMS
	mov [xmshand], dx
	mov [xmssize], 16
	mov ax, 1
	ret

reallocxms:
	mov ah, 0fh
	mov bx, [xmssize]
	add bx, 16
	mov dx, [xmshand]
	call [xmsaddr]
	or ax, ax
	je noAddXMS
	add [xmssize], 16
	mov ax, [xmssize]
	mov cl, 4
	shr ax, cl
	ret

noAddXMS:
	xor ax, ax
	ret
endp


proc addToEMS
	cmp [emsframe], 0
	je noAddEMS

	cmp [emshand], 0
	jne reallocems

	; allocate new handle
	mov ah, 43h
	mov bx, 1
	int 67h
	or ah, ah
	jne noAddEMS
	mov [emshand], dx
	mov [emssize], 1
	mov ax, 1
	ret

reallocems:
	mov ah, 51h
	mov bx, [emssize]
	inc bx
	mov dx, [emshand]
	int 67h
	or ah, ah
	jne noAddEMS
	inc [emssize]
	mov ax, [emssize]
	ret

noAddEMS:
	xor ax, ax
	ret
endp

proc addToVirt
	cmp [vrthand], 0
	je noAddVrt

	mov ax, [vrtpags]
	mov dx, 16384
	mul dx
	mov cx, dx
	mov dx, ax
	mov bx, [vrthand]
	mov ax, 4200h
	int 21h
	jc noAddVrt

	mov ah, 40h	; it doesn't really matter where we write from.
	mov cx, 16384	; actually, we may not need to write at all; i
	xor dx, dx	; forgot what happens if you seek beyond the end
	int 21h 	; of a file, and don't feel like checking it out.

	jc noAddVrt

	inc [vrtpags]
	mov ax, [vrtpags]
	ret

noAddVrt:
	xor ax, ax
	ret
endp

proc disposeAuxMem, vmofs:word, vmseg:word
	cmp [xmshand], 0
	je dsp_ems

	mov ah, 0ah
	mov dx, [xmshand]
	call [xmsaddr]
	mov [xmshand], 0
	mov [xmssize], 0
	mov [usexms], 0

dsp_ems:cmp [emshand], 0
	je dsp_vrt

	mov ah, 45h
	mov dx, [emshand]
	int 67h
	mov [emshand], 0
	mov [emssize], 0
	mov [emsframe], 0

dsp_vrt:mov bx, [vrthand]
	or bx, bx
	jz endDAM
	mov ah, 3eh
	int 21h 		; close the file

	push ds
	mov dx, [vmofs]
	mov ax, [vmseg]
	mov ds, ax
	mov ah, 41h
	int 21h 		; and delete it
	pop ds
	mov [vrthand], 0

endDAM: ret
endp

proc XMStoHeap uses si, tofs:word, tseg:word, fblk:word
	mov [dsthand], 0
	mov ax, [xmshand]
	mov [srchand], ax
	mov ax, [tofs]
	mov [word ptr dstofs], ax
	mov ax, [tseg]
	mov [word ptr dstofs+2], ax
	mov ax, [fblk]
	dec ax
	mov bx, 16384
	mul bx
	mov [word ptr srcofs], ax
	mov [word ptr srcofs+2], dx
	mov si, offset xmsxfr
	mov ah, 0bh
	call [xmsaddr]

	ret	; xms result is good for us, too
endp

proc EMStoHeap uses si di, tofs:word, tseg:word, fblk:word
	mov dx, [emshand]
	mov ax, 4400h
	mov bx, [fblk]
	dec bx
	int 67h
	or ah, ah
	jne no_eth

	push ds
	mov ax, [emsframe]
	mov bx, [tseg]
	mov di, [tofs]
	mov ds, ax
	mov es, bx
	xor si, si

	mov cx, 8192
	cld
	rep movsw
	pop ds
	mov ax, 1
	ret

no_eth: xor ax, ax
	ret
endp

proc VirttoHeap, tofs:word, tseg:word, fblk:word
	mov bx, [vrthand]
	or bx, bx
	je no_vth

	mov ax, [fblk]
	dec ax
	mov dx, 16384
	mul dx
	mov cx, dx
	mov dx, ax
	mov ax, 4200h
	int 21h 	; seek the block
	jc no_vth

	push ds
	mov dx, [tofs]
	mov ax, [tseg]
	mov ds, ax
	mov cx, 16384
	mov ah, 3fh
	int 21h 	; simple file read
	pop ds
	jc no_vth

	cmp ax, 16384
	jne no_vth

	mov ax, 1
	ret

no_vth: xor ax, ax
	ret
endp

proc HeapToEMS uses si di, tblk:word, fofs:word, fseg:word
	mov dx, [emshand]
	mov ax, 4400h
	mov bx, [tblk]
	dec bx
	int 67h
	or ah, ah
	jne no_hte

	push ds
	mov ax, [emsframe]
	mov bx, [fseg]
	mov si, [fofs]
	mov ds, bx
	mov es, ax
	xor di, di

	mov cx, 8192
	cld
	rep movsw
	pop ds
	xor ax, ax
	ret

no_hte: mov al, ah
	mov ah, 0
	ret
endp

proc HeapToXMS uses si, tblk:word, fofs:word, fseg:word
	mov [srchand], 0
	mov ax, [xmshand]
	mov [dsthand], ax
	mov ax, [fofs]
	mov [word ptr srcofs], ax
	mov ax, [fseg]
	mov [word ptr srcofs+2], ax
	mov ax, [tblk]
	dec ax
	mov bx, 16384
	mul bx
	mov [word ptr dstofs], ax
	mov [word ptr dstofs+2], dx
	mov si, offset xmsxfr
	mov ah, 0bh
	call [xmsaddr]

	ret	; xms result is good for us, too
endp

proc HeapToVirt, tblk:word, fofs:word, fseg:word
	mov bx, [vrthand]
	or bx, bx
	je no_htv

	mov ax, [tblk]
	dec ax
	mov dx, 16384
	mul dx
	mov cx, dx
	mov dx, ax
	mov ax, 4200h
	int 21h
	jc no_htv	; seek the block

	push ds
	mov dx, [fofs]
	mov ax, [fseg]
	mov ds, ax
	mov cx, 16384
	mov ah, 40h	
	int 21h 	; simple file write
	pop ds
	jc no_htv

	cmp ax, 16384
	jne no_htv

	mov ax, 1
	ret

no_htv: xor ax, ax
	ret
endp

end
