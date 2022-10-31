BITS 64

%define NUM_LIGHTS 32
struc SirenBuffer
	.Seed: resd 1
    .AdjustedTime: resd 1
    .BeatNumber: resd 1
    .RotatorLastBeatMask: resd NUM_LIGHTS
    .RotatorLastBeatTime: resd NUM_LIGHTS
	.FlasherLastBeatMask: resd NUM_LIGHTS
    .FlasherLastBeatTime: resd NUM_LIGHTS
    .HeadlightLastBeatMask: resd 4
    .HeadlightLastBeatTime: resd 4
    .HeadlightIntensity: resd 4
    .HeadlightStatus: resd 1
    .RotatorStatus: resq 1
    .FlasherStatus: resq 1
endstruc

extern ExpandSirenSettings
extern MakeBoneArrays
extern CheckSirenBoneArray
extern CheckGlassBoneArray
extern FreeSirenSettingsAtArray
extern FreeSirenSettings
extern LogConflict

segment .data

GLOBAL SirenSettings_init_ret
SirenSettings_init_ret dq 0

GLOBAL ComputeSirenIndex_ret
ComputeSirenIndex_ret dq 0

GLOBAL ComputeSirenIndex_logic
ComputeSirenIndex_logic dq 0

GLOBAL LogConflict_nz_ret
LogConflict_nz_ret dq 0

GLOBAL LogConflict_z_ret
LogConflict_z_ret dq 0

GLOBAL GetSirenSetting_ret
GetSirenSetting_ret dq 0

GLOBAL CopyVarToModel_ret
CopyVarToModel_ret dq 0

GLOBAL CarCols_loc
CarCols_loc dq 0

GLOBAL RphOnGetSirenSettings
RphOnGetSirenSettings dq 0

GLOBAL SetFlags_ret
SetFlags_ret dq 0

GLOBAL SirenSettings_ReallocFree_ret
SirenSettings_ReallocFree_ret dq 0

GLOBAL SirenSettings_Free_ret
SirenSettings_Free_ret dq 0

GLOBAL DSL_RotateBeatTimeSet_ret
DSL_RotateBeatTimeSet_ret dq 0

GLOBAL DSL_RotateBeatTimeSub_ret
DSL_RotateBeatTimeSub_ret dq 0

GLOBAL DSL_PreCompute_ret
DSL_PreCompute_ret dq 0

GLOBAL DSL_BoneCheck_ret
DSL_BoneCheck_ret dq 0

GLOBAL free_logic
free_logic dq 0

GLOBAL DSL_Return_ret
DSL_Return_ret dq 0

GLOBAL CheckBroken_ret
CheckBroken_ret dq 0

GLOBAL CheckBrokenGlass_ret
CheckBrokenGlass_ret dq 0

GLOBAL CheckBrokenTwo_ret
CheckBrokenTwo_ret dq 0

GLOBAL CheckBrokenTwoGlass_ret
CheckBrokenTwoGlass_ret dq 0

GLOBAL InitThingy_ret
InitThingy_ret dq 0

segment .text

GLOBAL SirenSettings_init_patch

SirenSettings_init_patch:
	mov dword [rbx], 0xff
	jmp [rel SirenSettings_init_ret]

GLOBAL SirenSettings_ReallocFree_patch

SirenSettings_ReallocFree_patch:
	mov rcx, rbx
	call FreeSirenSettingsAtArray
	jmp [rel SirenSettings_ReallocFree_ret]

GLOBAL SirenSettings_Free_patch

SirenSettings_Free_patch:
	mov r9, FreeSirenSettings
	jmp [rel SirenSettings_Free_ret]


GLOBAL ComputeSirenIndex_patch

ComputeSirenIndex_patch:
	push rax
	push rcx
	push rdx
	push r8
	push r9
	push r10
	push r11
	sub rsp, 0x20 ; Note: original function is unaligned, so this 8-byte misalignment is actually correct.
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

GLOBAL GetSirenSetting_RPH_patch

GetSirenSetting_RPH_patch:
	mov ah, byte [rcx + 0x55f]
	movzx rdx, ax
	mov r8, rdi
	mov r9, r15
	sub rsp, 0x28
	call [rel RphOnGetSirenSettings]
	add rsp, 0x28
	ret

GLOBAL CopyVarToModel_patch

CopyVarToModel_patch:
	mov al, byte [rsi + 0x4d]
	mov byte [rdi + 0x55f], al
	jmp [rel CopyVarToModel_ret]

GLOBAL SetFlags_patch

SetFlags_patch:
	mov al, byte [r14 + 0x53b]
	or al, byte [r14 + 0x55f]
	cmp al, r12b
	jmp [rel SetFlags_ret]

GLOBAL LogConflict_patch

LogConflict_patch:
	jnz .nonzero
	push rax
	push rcx
	push rdx
	push r8
	push r9
	push r10
	push r11
	sub rsp, 0x28
	mov rcx, rax
	mov rdx, rbp
	call LogConflict
	add rsp, 0x28
	pop r11
	pop r10
	pop r9
	pop r8
	pop rdx
	pop rcx
	pop rax
	jmp [rel LogConflict_z_ret]
.nonzero:
	jmp [rel LogConflict_nz_ret]

GLOBAL DSL_PreCompute_patch

DSL_PreCompute_patch:
	push rax
	sub rsp, 0x28
	mov rcx, r15
	call MakeBoneArrays
	add rsp, 0x28
	pop rcx
	sub rsp, 0x20
	call ExpandSirenSettings
	add rsp, 0x20
	jmp [rel DSL_PreCompute_ret]

GLOBAL DSL_BoneCheck_patch

DSL_BoneCheck_patch:
	push rax
	push rcx
	push rdx
	push r8
	push r9
	push r11
	sub rsp, 0x20
	mov rdx, r15
	call CheckSirenBoneArray
	add rsp, 0x20
	pop r11
	pop r9
	pop r8
	pop rdx
	pop rcx
	mov r10d, eax
	pop rax
	jmp [rel DSL_BoneCheck_ret]



GLOBAL DSL_RotateBeatTimeSet_patch

DSL_RotateBeatTimeSet_patch:
	mov dword [rax + r11*4 + SirenBuffer.RotatorLastBeatTime], r9d
	jmp [rel DSL_RotateBeatTimeSet_ret]

GLOBAL DSL_RotateBeatTimeSub_patch

DSL_RotateBeatTimeSub_patch:
	sub eax, dword [rcx + r11*4 + SirenBuffer.RotatorLastBeatTime]
	jmp [rel DSL_RotateBeatTimeSub_ret]

GLOBAL DSL_Return_patch

DSL_Return_patch:
	sub rsp, 0x20
	mov rcx, rsi
	call [rel free_logic]
	add rsp, 0x20
	jmp [rel DSL_Return_ret]

GLOBAL CheckBroken_patch

CheckBroken_patch:
	push r9
	sub rsp, 0x28
	mov rcx, r9
	call MakeBoneArrays
	mov rdx, [rsp + 0x28]
	mov ecx, r14d
	call CheckSirenBoneArray
	mov edx, eax
	add rsp, 0x28
	pop r9
	jmp [rel CheckBroken_ret]

GLOBAL CheckBrokenGlass_patch

CheckBrokenGlass_patch:
	push rcx
	sub rsp, 0x28
	mov rdx, r8
	mov ecx, r14d
	call CheckGlassBoneArray
	mov edx, eax
	add rsp, 0x28
	pop rcx
	jmp [rel CheckBrokenGlass_ret]

GLOBAL CheckBrokenTwo_patch

CheckBrokenTwo_patch:
	push r9
	sub rsp, 0x28
	mov rcx, r9
	call MakeBoneArrays
	mov rdx, [rsp + 0x28]
	mov ecx, r15d
	call CheckSirenBoneArray
	mov edx, eax
	add rsp, 0x28
	pop r9
	jmp [rel CheckBrokenTwo_ret]

GLOBAL CheckBrokenTwoGlass_patch

CheckBrokenTwoGlass_patch:
	push rcx
	sub rsp, 0x28
	mov rdx, r8
	mov ecx, r15d
	call CheckGlassBoneArray
	mov edx, eax
	add rsp, 0x28
	pop rcx
	jmp [rel CheckBrokenTwoGlass_ret]

GLOBAL InitThingy_patch

InitThingy_patch:
	push rcx
	sub rsp, 0x28
	call MakeBoneArrays
	mov rdx, [rsp + 0x28]
	mov ecx, r14d
	call CheckSirenBoneArray
	mov edx, eax
	add rsp, 0x28
	pop rcx
	jmp [rel InitThingy_ret]