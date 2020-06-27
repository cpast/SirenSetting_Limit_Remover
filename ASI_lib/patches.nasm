BITS 64
segment .data

GLOBAL SirenSettings_init_ret
SirenSettings_init_ret dq 0

GLOBAL ComputeSirenIndex_ret
ComputeSirenIndex_ret dq 0

GLOBAL ComputeSirenIndex_logic
ComputeSirenIndex_logic dq 0

GLOBAL GetSirenSetting_ret
GetSirenSetting_ret dq 0

GLOBAL CopyVarToModel_ret
CopyVarToModel_ret dq 0

GLOBAL CarCols_loc
CarCols_loc dq 0

GLOBAL RphHookPrep_ret
RphHookPrep_ret dq 0

GLOBAL SetFlags_ret
SetFlags_ret dq 0

segment .text

GLOBAL SirenSettings_init_patch

SirenSettings_init_patch:
	mov dword [rbx], 0xff
	jmp [rel SirenSettings_init_ret]

GLOBAL ComputeSirenIndex_patch

ComputeSirenIndex_patch:
	push rax
	push rcx
	push rdx
	push r8
	push r9
	push r10
	push r11
	sub rsp, 0x20
	mov rcx, rax
	call [rel ComputeSirenIndex_logic]
	add rsp, 0x20
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdx
	pop rcx
	pop rax
	jmp [rel ComputeSirenIndex_ret]

GLOBAL GetSirenSetting_patch

GetSirenSetting_patch:
	mov ah, byte [rcx + 0x55f]
	movzx rax, ax
	test rax, rax
	jz .zero
	mov rcx, qword [rel CarCols_loc]
	mov rcx, qword [rcx]
	imul rax, rax, 0x5e8
	add rax, qword [rcx + 0x48]
	jmp .return
.zero:
	xor eax, eax
.return:
	jmp [rel GetSirenSetting_ret]

GLOBAL CopyVarToModel_patch

CopyVarToModel_patch:
	mov al, byte [rsi + 0x4d]
	mov byte [rdi + 0x55f], al
	jmp [rel CopyVarToModel_ret]

GLOBAL RphHookPrep_patch

RphHookPrep_patch:
	mov ah, byte [rcx + 0x55f]
	jmp [rel RphHookPrep_ret]

GLOBAL SetFlags_patch

SetFlags_patch:
	mov al, byte [r14 + 0x53b]
	or al, byte [r14 + 0x55f]
	cmp al, r12b
	jmp [rel SetFlags_ret]