; aWarpSharp package 2016.06.23 for Avisynth+ and Avisynth 2.6
; based on Firesledge's 2015.12.30 for Avisynth 2.5
; aWarpSharp package 2012.03.28 for Avisynth 2.5
; Copyright (C) 2003 MarcFD, 2012 Skakov Pavel
; 2015 Firesledge
; 2016 pinterf
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.

.586
.model flat,c

.data

align 16

ul_0 dword 4 dup(0)
ul_1 dword 4 dup(1)
uw_8000 word 8 dup(32768)

.code
.686
.xmm


JPSDR_Warp0_8_SSE2 proc psrc:dword,pedg:dword,pdst:dword,src_pitch:dword,edg_pitchp:dword,edg_pitchn:dword,
	y_limit_min:dword,y_limit_max:dword,x_limit_min:dword,x_limit_max:dword,i_:dword,depthH:dword,depthV:dword
	
	public JPSDR_Warp0_8_SSE2

xmm8_ equ XMMWORD ptr[esp+8h]
xmm9_ equ XMMWORD ptr[esp+18h] ; depthH
xmm10_ equ XMMWORD ptr[esp+28h]
xmm11_ equ XMMWORD ptr[esp+38h]
xmm12_ equ XMMWORD ptr[esp+48h]
xmm13_ equ XMMWORD ptr[esp+58h]
xmm14_ equ XMMWORD ptr[esp+68h] ; depthV
	
	push esi
	push edi
	push ebx
	push ebp
	
	mov esi,x_limit_min
	movdqu xmm3,XMMWORD ptr[esi]
	mov esi,x_limit_max
	movdqu xmm4,XMMWORD ptr[esi]
	mov	esi,psrc
	mov	ecx,pedg
	mov	eax,pdst
	mov edx,src_pitch
	mov	ebx,edg_pitchp
	mov edi,i_ 					;signed!
    sub	eax,8
    add	edx,esi
    add	ebx,ecx
    movd xmm1,y_limit_min
    movd xmm2,y_limit_max
    movd xmm6,depthH
    movd xmm0,src_pitch
    pcmpeqw xmm7,xmm7
    psrlw xmm7,15
    punpcklwd xmm0,xmm7
	movd xmm7,depthV
    pshufd xmm1,xmm1,0
    pshufd xmm2,xmm2,0
    pshufd xmm6,xmm6,0
    pshufd xmm0,xmm0,0
	pshufd xmm7,xmm7,0
    packssdw xmm1,xmm1
    packssdw xmm2,xmm2
    packssdw xmm6,xmm6
	packssdw xmm7,xmm7
    pcmpeqw xmm5,xmm5
    psllw xmm5,15
	
	push edg_pitchn
	lea ebp,[esp-6Ch]
	and ebp,0FFFFFFF0h
	xchg esp,ebp
	push eax
	push ebp
	mov ebp,dword ptr[ebp]
	add	ebp,ecx
	
    movdqa xmm8_,xmm0
    movdqa xmm9_,xmm6
    movdqa xmm10_,xmm1
    movdqa xmm11_,xmm2
    movdqa xmm12_,xmm3
    movdqa xmm13_,xmm4
	movdqa xmm14_,xmm7
	movdqa xmm7,XMMWORD ptr[edi+ecx]
	
    movdqa xmm1,xmm7
    pslldq xmm7,7
    punpcklqdq xmm7,xmm1
    psrldq xmm1,1
    psrldq xmm7,7
	psrlw xmm5,8
	
JPSDR_Warp0_8_SSE2_1:
	movq xmm4,qword ptr[edi+ebx]
	movq xmm2,qword ptr[edi+ebp]
    pxor xmm3,xmm3
    punpcklbw xmm7,xmm3
    punpcklbw xmm1,xmm3
    punpcklbw xmm4,xmm3
    punpcklbw xmm2,xmm3
    psubw xmm7,xmm1
    psubw xmm4,xmm2
    psllw xmm7,7
    psllw xmm4,7
    pmulhw xmm7,xmm9_ ;depthH
    pmulhw xmm4,xmm14_ ;depthV
    movdqa xmm6,xmm8_	;preload 1 src_pitch
    pmaxsw xmm4,xmm10_	;y_limit_min
    pminsw xmm4,xmm11_	;y_limit_max
	
    pcmpeqw xmm3,xmm3
    psrlw xmm3,9
    movdqa xmm1,xmm7
    movdqa xmm2,xmm4
	
    pand xmm7,xmm3 ;007F
    pand xmm4,xmm3 ;007F
    psraw xmm1,7
    psraw xmm2,7

    movd xmm3,edi
    pshufd xmm3,xmm3,0
	
    packssdw xmm3,xmm3
    paddsw xmm1,xmm3
	
    movdqa xmm3,xmm13_	;x_limit_max
    movdqa xmm0,xmm12_	;x_limit_min
	
    pcmpgtw xmm3,xmm1
    pcmpgtw xmm0,xmm1
	
    pminsw xmm1,xmm13_	;x_limit_max
    pmaxsw xmm1,xmm12_	;x_limit_min
		  
    pand xmm7,xmm3
    pandn xmm0,xmm7

    movdqa xmm7,xmm2
    punpcklwd xmm2,xmm1
    punpckhwd xmm7,xmm1
    pmaddwd xmm2,xmm6 ;1 src_pitch
    pmaddwd xmm7,xmm6 ;1 src_pitch

    movd eax,xmm2
    pinsrw xmm3,word ptr[eax+esi],0
    psrldq xmm2,4
    pinsrw xmm1,word ptr[eax+edx],0
    movd eax,xmm2
    pinsrw xmm3,word ptr[eax+esi+1],1
    psrldq xmm2,4
    pinsrw xmm1,word ptr[eax+edx+1],1
    movd eax,xmm2
    pinsrw xmm3,word ptr[eax+esi+2],2
    psrldq xmm2,4
    pinsrw xmm1,word ptr[eax+edx+2],2
    movd eax,xmm2
    pinsrw xmm3,word ptr[eax+esi+3],3
    pinsrw xmm1,word ptr[eax+edx+3],3
    movd eax,xmm7
    pinsrw xmm3,word ptr[eax+esi+4],4
    psrldq xmm7,4
    pinsrw xmm1,word ptr[eax+edx+4],4
    movd eax,xmm7
    pinsrw xmm3,word ptr[eax+esi+5],5
    psrldq xmm7,4
    pinsrw xmm1,word ptr[eax+edx+5],5
    movd eax,xmm7
    pinsrw xmm3,word ptr[eax+esi+6],6
    psrldq xmm7,4
    pinsrw xmm1,word ptr[eax+edx+6],6
    movd eax,xmm7
    pinsrw xmm3,word ptr[eax+esi+7],7
    pinsrw xmm1,word ptr[eax+edx+7],7
	mov	eax,dword ptr[esp+4]
	
    pcmpeqw xmm6,xmm6
    movdqa xmm2,xmm3
    psrlw xmm6,8
    movdqa xmm7,xmm1
    pand xmm3,xmm6 ;00FF
    pand xmm1,xmm6 ;00FF
    movdqa xmm6,xmm5
    psubw xmm6,xmm0
    pmullw xmm3,xmm6
    pmullw xmm1,xmm6
    movdqa xmm6,xmm5 ;0080
    psrlw xmm5,1
    psrlw xmm2,8
    psrlw xmm7,8
    pmullw xmm2,xmm0
    pmullw xmm7,xmm0
    paddw xmm3,xmm2
    paddw xmm1,xmm7
    paddw xmm3,xmm5 ;0040
    paddw xmm1,xmm5 ;0040
    psraw xmm3,7
    psraw xmm1,7

    psubw xmm6,xmm4
    movdqu xmm7,XMMWORD ptr[edi+ecx+7]
    pmullw xmm1,xmm4
    pmullw xmm3,xmm6
    paddw xmm3,xmm1
    movdqa xmm1,xmm7
	
    paddw xmm3,xmm5  ;0040
    psrldq xmm1,2
    psraw xmm3,7
    paddw xmm5,xmm5
    packuswb xmm3,xmm3
    add edi,8
    jg short JPSDR_Warp0_8_SSE2_2
    movq qword ptr[edi+eax],xmm3
    jnz JPSDR_Warp0_8_SSE2_1
    jmp short JPSDR_Warp0_8_SSE2_Fin
	
JPSDR_Warp0_8_SSE2_2:
	movd dword ptr[edi+eax],xmm3
	
JPSDR_Warp0_8_SSE2_Fin:
	pop esp
	pop ecx

	pop ebp
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_Warp0_8_SSE2 endp


JPSDR_Warp0_8_AVX proc psrc:dword,pedg:dword,pdst:dword,src_pitch:dword,edg_pitchp:dword,edg_pitchn:dword,
	y_limit_min:dword,y_limit_max:dword,x_limit_min:dword,x_limit_max:dword,i_:dword,depthH:dword,depthV:dword

	public JPSDR_Warp0_8_AVX
	
xmm8_ equ XMMWORD ptr[esp+8h]
xmm9_ equ XMMWORD ptr[esp+18h] ;depthH
xmm10_ equ XMMWORD ptr[esp+28h]
xmm11_ equ XMMWORD ptr[esp+38h]
xmm12_ equ XMMWORD ptr[esp+48h]
xmm13_ equ XMMWORD ptr[esp+58h]
xmm14_ equ XMMWORD ptr[esp+68h] ;depthV
	
	push esi
	push edi
	push ebx
	push ebp
	
	mov esi,x_limit_min
	vmovdqu xmm3,XMMWORD ptr[esi]
	mov esi,x_limit_max
	vmovdqu xmm4,XMMWORD ptr[esi]
	mov	esi,psrc
	mov	ecx,pedg
	mov	eax,pdst
	mov edx,src_pitch
	mov	ebx,edg_pitchp
	mov edi,i_ 					;signed!
    sub	eax,8
    add	edx,esi
    add	ebx,ecx
    vmovd xmm1,y_limit_min
    vmovd xmm2,y_limit_max
    vmovd xmm6,depthH
    vmovd xmm0,src_pitch
    vpcmpeqw xmm7,xmm7,xmm7
    vpsrlw xmm7,xmm7,15
    vpunpcklwd xmm0,xmm0,xmm7
	vmovd xmm7,depthV
    vpshufd xmm1,xmm1,0
    vpshufd xmm2,xmm2,0
    vpshufd xmm6,xmm6,0
    vpshufd xmm0,xmm0,0
	vpshufd xmm7,xmm7,0
    vpackssdw xmm1,xmm1,xmm1
    vpackssdw xmm2,xmm2,xmm2
    vpackssdw xmm6,xmm6,xmm6
	vpackssdw xmm7,xmm7,xmm7
    vpcmpeqw xmm5,xmm5,xmm5
    vpsllw xmm5,xmm5,15
	
	push edg_pitchn
	lea ebp,[esp-6Ch]
	and ebp,0FFFFFFF0h
	xchg esp,ebp
	push eax
	push ebp
	mov ebp,dword ptr[ebp]
	add	ebp,ecx
	
    vmovdqa xmm8_,xmm0
    vmovdqa xmm9_,xmm6
    vmovdqa xmm10_,xmm1
    vmovdqa xmm11_,xmm2
    vmovdqa xmm12_,xmm3
    vmovdqa xmm13_,xmm4
	vmovdqa xmm14_,xmm7
	vmovdqa xmm7,XMMWORD ptr[edi+ecx]
	
    vmovdqa xmm1,xmm7
    vpslldq xmm7,xmm7,7
    vpunpcklqdq xmm7,xmm7,xmm1
    vpsrldq xmm1,xmm1,1
    vpsrldq xmm7,xmm7,7
	
JPSDR_Warp0_8_AVX_1:
    vmovq xmm4,qword ptr[edi+ebx]
    vmovq xmm2,qword ptr[edi+ebp]
    vpxor xmm0,xmm0,xmm0
    vpunpcklbw xmm7,xmm7,xmm0
    vpunpcklbw xmm1,xmm1,xmm0
    vpunpcklbw xmm4,xmm4,xmm0
    vpunpcklbw xmm2,xmm2,xmm0
    vpsubw xmm7,xmm7,xmm1
    vpsubw xmm4,xmm4,xmm2
    vpsllw xmm7,xmm7,7
    vpsllw xmm4,xmm4,7
    vpmulhw xmm7,xmm7,xmm9_ ;depthH
    vpmulhw xmm4,xmm4,xmm14_ ;depthV
    vmovd xmm6,edi ;preload

    vpmaxsw xmm4,xmm4,xmm10_	;y_limit_min
    vpminsw xmm4,xmm4,xmm11_	;y_limit_max
	
	vpshufd xmm6,xmm6,0
	
	vpcmpeqw xmm0,xmm0,xmm0
    vpsrlw xmm0,xmm0,9
	vpsraw xmm2,xmm4,7
	vpsraw xmm1,xmm7,7
	
    vpand xmm4,xmm4,xmm0 ;007F
    vpand xmm7,xmm7,xmm0 ;007F

    vpackssdw xmm6,xmm6,xmm6
    vpaddsw xmm1,xmm1,xmm6
	
    vmovdqa xmm6,xmm8_	;preload 1 src_pitch

    vmovdqa xmm0,xmm13_	;x_limit_max
    vmovdqa xmm3,xmm12_	;x_limit_min

    vpcmpgtw xmm0,xmm0,xmm1
    vpcmpgtw xmm3,xmm3,xmm1
	
    vpminsw xmm1,xmm1,xmm13_	;x_limit_max
    vpmaxsw xmm1,xmm1,xmm12_	;x_limit_min

    vpand xmm7,xmm7,xmm0
    vpandn xmm3,xmm3,xmm7

	vpunpckhwd xmm7,xmm2,xmm1
    vpunpcklwd xmm2,xmm2,xmm1
    vpmaddwd xmm2,xmm2,xmm6 ;1 src_pitch
    vpmaddwd xmm7,xmm7,xmm6 ;1 src_pitch
	
    vpsignw xmm3,xmm3,xmm5 ;8000
    vpsignw xmm4,xmm4,xmm5 ;8000
    vpacksswb xmm5,xmm5,xmm5
    vpacksswb xmm3,xmm3,xmm3
    vpacksswb xmm4,xmm4,xmm4

    vpsubb xmm0,xmm5,xmm3
    vpsubb xmm1,xmm5,xmm4
    vpsrlw xmm5,xmm5,9
    vpunpcklbw xmm0,xmm0,xmm3
    vpunpcklbw xmm1,xmm1,xmm4

    vmovd eax,xmm2
    vpinsrw xmm3,xmm3,word ptr[eax+esi],0
	vpsrldq xmm2,xmm2,4
    vpinsrw xmm4,xmm4,word ptr[eax+edx],0
    vmovd eax,xmm2
    vpinsrw xmm3,xmm3,word ptr[eax+esi+1],1
	vpsrldq xmm2,xmm2,4
    vpinsrw xmm4,xmm4,word ptr[eax+edx+1],1
    vmovd eax,xmm2
    vpinsrw xmm3,xmm3,word ptr[eax+esi+2],2
	vpsrldq xmm2,xmm2,4
    vpinsrw xmm4,xmm4,word ptr[eax+edx+2],2
    vmovd eax,xmm2
    vpinsrw xmm3,xmm3,word ptr[eax+esi+3],3
    vpinsrw xmm4,xmm4,word ptr[eax+edx+3],3
    vmovd eax,xmm7
    vpinsrw xmm3,xmm3,word ptr[eax+esi+4],4
	vpsrldq xmm7,xmm7,4
    vpinsrw xmm4,xmm4,word ptr[eax+edx+4],4
    vmovd eax,xmm7
    vpinsrw xmm3,xmm3,word ptr[eax+esi+5],5
	vpsrldq xmm7,xmm7,4
    vpinsrw xmm4,xmm4,word ptr[eax+edx+5],5
    vmovd eax,xmm7
    vpinsrw xmm3,xmm3,word ptr[eax+esi+6],6
	vpsrldq xmm7,xmm7,4
    vpinsrw xmm4,xmm4,word ptr[eax+edx+6],6
    vmovd eax,xmm7
    vpinsrw xmm3,xmm3,word ptr[eax+esi+7],7
    vpinsrw xmm4,xmm4,word ptr[eax+edx+7],7

    vpcmpeqw xmm2,xmm2,xmm2
    vmovdqu xmm7,XMMWORD ptr[edi+ecx+7]
    vpmaddubsw xmm3,xmm3,xmm0
    vpmaddubsw xmm4,xmm4,xmm0
	
	mov	eax,dword ptr[esp+4]
	
    vpsignw xmm3,xmm3,xmm2
    vpsignw xmm4,xmm4,xmm2
    vpaddw xmm3,xmm3,xmm5 ; 0040
    vpaddw xmm4,xmm4,xmm5 ; 0040
    vpsraw xmm3,xmm3,7
    vpsraw xmm4,xmm4,7
    vpackuswb xmm3,xmm3,xmm3
    vpackuswb xmm4,xmm4,xmm4
    vpunpcklbw xmm3,xmm3,xmm4
    vpmaddubsw xmm3,xmm3,xmm1
    vpalignr xmm1,xmm1,xmm7,2
    vpsignw xmm3,xmm3,xmm2

    vpaddw xmm3,xmm3,xmm5 ; 0040
    vpsraw xmm3,xmm3,7
    vpsllw xmm5,xmm5,9
    vpackuswb xmm3,xmm3,xmm3
		  
    add edi,8	
    jg short JPSDR_Warp0_8_AVX_2
    vmovq qword ptr[edi+eax],xmm3
    jnz JPSDR_Warp0_8_AVX_1
    jmp short JPSDR_Warp0_8_AVX_Fin
	
JPSDR_Warp0_8_AVX_2:
	vmovd dword ptr[edi+eax],xmm3
	
JPSDR_Warp0_8_AVX_Fin:
	pop esp
	pop ecx

	pop ebp
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_Warp0_8_AVX endp


JPSDR_Warp2_8_SSE2 proc psrc:dword,pedg:dword,pdst:dword,src_pitch:dword,edg_pitchp:dword,edg_pitchn:dword,
	y_limit_min:dword,y_limit_max:dword,x_limit_min:dword,x_limit_max:dword,i_:dword,depthH:dword,depthV:dword

	public JPSDR_Warp2_8_SSE2
	
xmm8_ equ XMMWORD ptr[esp+8h]
xmm9_ equ XMMWORD ptr[esp+18h] ;depthH
xmm10_ equ XMMWORD ptr[esp+28h]
xmm11_ equ XMMWORD ptr[esp+38h]
xmm12_ equ XMMWORD ptr[esp+48h]
xmm13_ equ XMMWORD ptr[esp+58h]
xmm14_ equ XMMWORD ptr[esp+68h] ;depthV
	
	push esi
	push edi
	push ebx
	push ebp
	
	mov esi,x_limit_min
	movdqu xmm3,XMMWORD ptr[esi]
	mov esi,x_limit_max
	movdqu xmm4,XMMWORD ptr[esi]
	mov	esi,psrc
	mov	ecx,pedg
	mov	eax,pdst
	mov edx,src_pitch
	mov	ebx,edg_pitchp
	mov edi,i_ 					;signed!
    sub	eax,8
    add	edx,esi
    add	ebx,ecx
    movd xmm1,y_limit_min
    movd xmm2,y_limit_max
    movd xmm6,depthH
    movd xmm0,src_pitch
    pcmpeqw xmm7,xmm7
    psrlw xmm7,15
    punpcklwd xmm0,xmm7
	movd xmm7,depthV
    pshufd xmm1,xmm1,0
    pshufd xmm2,xmm2,0
    pshufd xmm6,xmm6,0
    pshufd xmm0,xmm0,0
	pshufd xmm7,xmm7,0
    packssdw xmm1,xmm1
    packssdw xmm2,xmm2
    packssdw xmm6,xmm6
	packssdw xmm7,xmm7
    pcmpeqw xmm5,xmm5
    psllw xmm5,15
	
	push edg_pitchn
	lea ebp,[esp-6Ch]
	and ebp,0FFFFFFF0h
	xchg esp,ebp
	push eax
	push ebp
	mov ebp,dword ptr[ebp]
	add	ebp,ecx
	
    movdqa xmm8_,xmm0
    movdqa xmm9_,xmm6
    movdqa xmm10_,xmm1
    movdqa xmm11_,xmm2
    movdqa xmm12_,xmm3
    movdqa xmm13_,xmm4
	movdqa xmm14_,xmm7
	movdqa xmm7,XMMWORD ptr[edi+ecx]
	
    movdqa xmm1,xmm7
    pslldq xmm7,7
    punpcklqdq xmm7,xmm1
    psrldq xmm1,1
    psrldq xmm7,7
	psrlw xmm5,8
	
JPSDR_Warp2_8_SSE2_1:
	movq xmm4,qword ptr[edi+ebx]
	movq xmm2,qword ptr[edi+ebp]
    pxor xmm3,xmm3
    punpcklbw xmm7,xmm3
    punpcklbw xmm1,xmm3
    punpcklbw xmm4,xmm3
    punpcklbw xmm2,xmm3
    psubw xmm7,xmm1
    psubw xmm4,xmm2
    psllw xmm7,7
    psllw xmm4,7
    pmulhw xmm7,xmm9_ ;depthH
    pmulhw xmm4,xmm14_ ;depthV
    movdqa xmm6,xmm8_	;preload 1 src_pitch
    pmaxsw xmm4,xmm10_	;y_limit_min
    pminsw xmm4,xmm11_	;y_limit_max
	
    pcmpeqw xmm3,xmm3
    psrlw xmm3,9
    movdqa xmm1,xmm7
    movdqa xmm2,xmm4
	
    psllw xmm7,2 ;SMAGL=2
    psllw xmm4,2 ;SMAGL=2
	
    pand xmm7,xmm3 ;007F
    pand xmm4,xmm3 ;007F
    psraw xmm1,5	;7-SMAGL
    psraw xmm2,5	;7-SMAGL

    movd xmm3,edi
    pshufd xmm3,xmm3,0
	
	pslld xmm3,2	;SMAGL=2
	
    packssdw xmm3,xmm3
    paddsw xmm1,xmm3
	
    movdqa xmm3,xmm13_	;x_limit_max
    movdqa xmm0,xmm12_	;x_limit_min
	
    pcmpgtw xmm3,xmm1
    pcmpgtw xmm0,xmm1
	
    pminsw xmm1,xmm13_	;x_limit_max
    pmaxsw xmm1,xmm12_	;x_limit_min
		  
    pand xmm7,xmm3
    pandn xmm0,xmm7

    movdqa xmm7,xmm2
    punpcklwd xmm2,xmm1
    punpckhwd xmm7,xmm1
    pmaddwd xmm2,xmm6 ;1 src_pitch
    pmaddwd xmm7,xmm6 ;1 src_pitch

    movd eax,xmm2
    pinsrw xmm3,word ptr[eax+esi],0
    psrldq xmm2,4
    pinsrw xmm1,word ptr[eax+edx],0
    movd eax,xmm2
    pinsrw xmm3,word ptr[eax+esi+1*4],1
    psrldq xmm2,4
    pinsrw xmm1,word ptr[eax+edx+1*4],1
    movd eax,xmm2
    pinsrw xmm3,word ptr[eax+esi+2*4],2
    psrldq xmm2,4
    pinsrw xmm1,word ptr[eax+edx+2*4],2
    movd eax,xmm2
    pinsrw xmm3,word ptr[eax+esi+3*4],3
    pinsrw xmm1,word ptr[eax+edx+3*4],3
    movd eax,xmm7
    pinsrw xmm3,word ptr[eax+esi+4*4],4
    psrldq xmm7,4
    pinsrw xmm1,word ptr[eax+edx+4*4],4
    movd eax,xmm7
    pinsrw xmm3,word ptr[eax+esi+5*4],5
    psrldq xmm7,4
    pinsrw xmm1,word ptr[eax+edx+5*4],5
    movd eax,xmm7
    pinsrw xmm3,word ptr[eax+esi+6*4],6
    psrldq xmm7,4
    pinsrw xmm1,word ptr[eax+edx+6*4],6
    movd eax,xmm7
    pinsrw xmm3,word ptr[eax+esi+7*4],7
    pinsrw xmm1,word ptr[eax+edx+7*4],7
	mov	eax,dword ptr[esp+4]
	
    pcmpeqw xmm6,xmm6
    movdqa xmm2,xmm3
    psrlw xmm6,8
    movdqa xmm7,xmm1
    pand xmm3,xmm6 ;00FF
    pand xmm1,xmm6 ;00FF
    movdqa xmm6,xmm5
    psubw xmm6,xmm0
    pmullw xmm3,xmm6
    pmullw xmm1,xmm6
    movdqa xmm6,xmm5 ;0080
    psrlw xmm5,1
    psrlw xmm2,8
    psrlw xmm7,8
    pmullw xmm2,xmm0
    pmullw xmm7,xmm0
    paddw xmm3,xmm2
    paddw xmm1,xmm7
    paddw xmm3,xmm5 ;0040
    paddw xmm1,xmm5 ;0040
    psraw xmm3,7
    psraw xmm1,7

    psubw xmm6,xmm4
    movdqu xmm7,XMMWORD ptr[edi+ecx+7]
    pmullw xmm1,xmm4
    pmullw xmm3,xmm6
    paddw xmm3,xmm1
    movdqa xmm1,xmm7
	
    paddw xmm3,xmm5  ;0040
    psrldq xmm1,2
    psraw xmm3,7
    paddw xmm5,xmm5
    packuswb xmm3,xmm3
    add edi,8
    jg short JPSDR_Warp2_8_SSE2_2
    movq qword ptr[edi+eax],xmm3
    jnz JPSDR_Warp2_8_SSE2_1
    jmp short JPSDR_Warp2_8_SSE2_Fin
	
JPSDR_Warp2_8_SSE2_2:
	movd dword ptr[edi+eax],xmm3
	
JPSDR_Warp2_8_SSE2_Fin:
	pop esp
	pop ecx

	pop ebp
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_Warp2_8_SSE2 endp


JPSDR_Warp2_8_AVX proc psrc:dword,pedg:dword,pdst:dword,src_pitch:dword,edg_pitchp:dword,edg_pitchn:dword,
	y_limit_min:dword,y_limit_max:dword,x_limit_min:dword,x_limit_max:dword,i_:dword,depthH:dword,depthV:dword

	public JPSDR_Warp2_8_AVX
	
xmm8_ equ XMMWORD ptr[esp+8h]
xmm9_ equ XMMWORD ptr[esp+18h] ;depthH
xmm10_ equ XMMWORD ptr[esp+28h]
xmm11_ equ XMMWORD ptr[esp+38h]
xmm12_ equ XMMWORD ptr[esp+48h]
xmm13_ equ XMMWORD ptr[esp+58h]
xmm14_ equ XMMWORD ptr[esp+68h] ;depthV
	
	push esi
	push edi
	push ebx
	push ebp
	
	mov esi,x_limit_min
	vmovdqu xmm3,XMMWORD ptr[esi]
	mov esi,x_limit_max
	vmovdqu xmm4,XMMWORD ptr[esi]
	mov	esi,psrc
	mov	ecx,pedg
	mov	eax,pdst
	mov edx,src_pitch
	mov	ebx,edg_pitchp
	mov edi,i_ 					;signed!
    sub	eax,8
    add	edx,esi
    add	ebx,ecx
    vmovd xmm1,y_limit_min
    vmovd xmm2,y_limit_max
    vmovd xmm6,depthH
    vmovd xmm0,src_pitch
    vpcmpeqw xmm7,xmm7,xmm7
    vpsrlw xmm7,xmm7,15
    vpunpcklwd xmm0,xmm0,xmm7
	vmovd xmm7,depthV
    vpshufd xmm1,xmm1,0
    vpshufd xmm2,xmm2,0
    vpshufd xmm6,xmm6,0
    vpshufd xmm0,xmm0,0
	vpshufd xmm7,xmm7,0
    vpackssdw xmm1,xmm1,xmm1
    vpackssdw xmm2,xmm2,xmm2
    vpackssdw xmm6,xmm6,xmm6
	vpackssdw xmm7,xmm7,xmm7
    vpcmpeqw xmm5,xmm5,xmm5
    vpsllw xmm5,xmm5,15
	
	push edg_pitchn
	lea ebp,[esp-6Ch]
	and ebp,0FFFFFFF0h
	xchg esp,ebp
	push eax
	push ebp
	mov ebp,dword ptr[ebp]
	add	ebp,ecx
	
    vmovdqa xmm8_,xmm0
    vmovdqa xmm9_,xmm6
    vmovdqa xmm10_,xmm1
    vmovdqa xmm11_,xmm2
    vmovdqa xmm12_,xmm3
    vmovdqa xmm13_,xmm4
	vmovdqa xmm14_,xmm7
	vmovdqa xmm7,XMMWORD ptr[edi+ecx]
	
    vmovdqa xmm1,xmm7
    vpslldq xmm7,xmm7,7
    vpunpcklqdq xmm7,xmm7,xmm1
    vpsrldq xmm1,xmm1,1
    vpsrldq xmm7,xmm7,7
	
JPSDR_Warp2_8_AVX_1:
    vmovq xmm4,qword ptr[edi+ebx]
    vmovq xmm2,qword ptr[edi+ebp]
    vpxor xmm0,xmm0,xmm0
    vpunpcklbw xmm7,xmm7,xmm0
    vpunpcklbw xmm1,xmm1,xmm0
    vpunpcklbw xmm4,xmm4,xmm0
    vpunpcklbw xmm2,xmm2,xmm0
    vpsubw xmm7,xmm7,xmm1
    vpsubw xmm4,xmm4,xmm2
    vpsllw xmm7,xmm7,7
    vpsllw xmm4,xmm4,7
    vpmulhw xmm7,xmm7,xmm9_ ;depthH
    vpmulhw xmm4,xmm4,xmm14_ ;depthV
    vmovd xmm6,edi ;preload

    vpmaxsw xmm4,xmm4,xmm10_	;y_limit_min
    vpminsw xmm4,xmm4,xmm11_	;y_limit_max
	
	vpshufd xmm6,xmm6,0
	
	vpslld xmm6,xmm6,2	;SMAGL=2
	
	vpcmpeqw xmm0,xmm0,xmm0
    vpsrlw xmm0,xmm0,9
	vpsraw xmm2,xmm4,5	;7-SMAGL
	vpsraw xmm1,xmm7,5	;7-SMAGL
	
    vpsllw xmm4,xmm4,2	;SMAGL=2
    vpsllw xmm7,xmm7,2	;SMAGL=2
	
    vpand xmm4,xmm4,xmm0 ;007F
    vpand xmm7,xmm7,xmm0 ;007F

    vpackssdw xmm6,xmm6,xmm6
    vpaddsw xmm1,xmm1,xmm6
	
    vmovdqa xmm6,xmm8_	;preload 1 src_pitch

    vmovdqa xmm0,xmm13_	;x_limit_max
    vmovdqa xmm3,xmm12_	;x_limit_min

    vpcmpgtw xmm0,xmm0,xmm1
    vpcmpgtw xmm3,xmm3,xmm1
	
    vpminsw xmm1,xmm1,xmm13_	;x_limit_max
    vpmaxsw xmm1,xmm1,xmm12_	;x_limit_min

    vpand xmm7,xmm7,xmm0
    vpandn xmm3,xmm3,xmm7

	vpunpckhwd xmm7,xmm2,xmm1
    vpunpcklwd xmm2,xmm2,xmm1
    vpmaddwd xmm2,xmm2,xmm6 ;1 src_pitch
    vpmaddwd xmm7,xmm7,xmm6 ;1 src_pitch
	
    vpsignw xmm3,xmm3,xmm5 ;8000
    vpsignw xmm4,xmm4,xmm5 ;8000
    vpacksswb xmm5,xmm5,xmm5
    vpacksswb xmm3,xmm3,xmm3
    vpacksswb xmm4,xmm4,xmm4

    vpsubb xmm0,xmm5,xmm3
    vpsubb xmm1,xmm5,xmm4
    vpsrlw xmm5,xmm5,9
    vpunpcklbw xmm0,xmm0,xmm3
    vpunpcklbw xmm1,xmm1,xmm4

    vmovd eax,xmm2
    vpinsrw xmm3,xmm3,word ptr[eax+esi],0
	vpsrldq xmm2,xmm2,4
    vpinsrw xmm4,xmm4,word ptr[eax+edx],0
    vmovd eax,xmm2
    vpinsrw xmm3,xmm3,word ptr[eax+esi+1*4],1
	vpsrldq xmm2,xmm2,4
    vpinsrw xmm4,xmm4,word ptr[eax+edx+1*4],1
    vmovd eax,xmm2
    vpinsrw xmm3,xmm3,word ptr[eax+esi+2*4],2
	vpsrldq xmm2,xmm2,4
    vpinsrw xmm4,xmm4,word ptr[eax+edx+2*4],2
    vmovd eax,xmm2
    vpinsrw xmm3,xmm3,word ptr[eax+esi+3*4],3
    vpinsrw xmm4,xmm4,word ptr[eax+edx+3*4],3
    vmovd eax,xmm7
    vpinsrw xmm3,xmm3,word ptr[eax+esi+4*4],4
	vpsrldq xmm7,xmm7,4
    vpinsrw xmm4,xmm4,word ptr[eax+edx+4*4],4
    vmovd eax,xmm7
    vpinsrw xmm3,xmm3,word ptr[eax+esi+5*4],5
	vpsrldq xmm7,xmm7,4
    vpinsrw xmm4,xmm4,word ptr[eax+edx+5*4],5
    vmovd eax,xmm7
    vpinsrw xmm3,xmm3,word ptr[eax+esi+6*4],6
	vpsrldq xmm7,xmm7,4
    vpinsrw xmm4,xmm4,word ptr[eax+edx+6*4],6
    vmovd eax,xmm7
    vpinsrw xmm3,xmm3,word ptr[eax+esi+7*4],7
    vpinsrw xmm4,xmm4,word ptr[eax+edx+7*4],7

    vpcmpeqw xmm2,xmm2,xmm2
    vmovdqu xmm7,XMMWORD ptr[edi+ecx+7]
    vpmaddubsw xmm3,xmm3,xmm0
    vpmaddubsw xmm4,xmm4,xmm0
	
	mov	eax,dword ptr[esp+4]
	
    vpsignw xmm3,xmm3,xmm2
    vpsignw xmm4,xmm4,xmm2
    vpaddw xmm3,xmm3,xmm5 ; 0040
    vpaddw xmm4,xmm4,xmm5 ; 0040
    vpsraw xmm3,xmm3,7
    vpsraw xmm4,xmm4,7
    vpackuswb xmm3,xmm3,xmm3
    vpackuswb xmm4,xmm4,xmm4
    vpunpcklbw xmm3,xmm3,xmm4
    vpmaddubsw xmm3,xmm3,xmm1
    vpalignr xmm1,xmm1,xmm7,2
    vpsignw xmm3,xmm3,xmm2

    vpaddw xmm3,xmm3,xmm5 ; 0040
    vpsraw xmm3,xmm3,7
    vpsllw xmm5,xmm5,9
    vpackuswb xmm3,xmm3,xmm3
		  
    add edi,8	
    jg short JPSDR_Warp2_8_AVX_2
    vmovq qword ptr[edi+eax],xmm3
    jnz JPSDR_Warp2_8_AVX_1
    jmp short JPSDR_Warp2_8_AVX_Fin
	
JPSDR_Warp2_8_AVX_2:
	vmovd dword ptr[edi+eax],xmm3
	
JPSDR_Warp2_8_AVX_Fin:
	pop esp
	pop ecx

	pop ebp
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_Warp2_8_AVX endp


JPSDR_Sobel_8_SSE2 proc psrc:dword,pdst:dword,src_pitch:dword,y_:dword,height:dword,i_:dword,thresh:dword

	public JPSDR_Sobel_8_SSE2
	
	push esi
	push edi
	push ebx

	mov ebx,16
    mov esi,psrc
    mov edi,pdst
    mov edx,src_pitch
    xor	eax,eax
    mov ecx,y_
    test ecx,ecx
    cmovnz eax,edx
    inc	ecx
    add	edx,eax
    cmp	ecx,height     ;32 bit O.K.
    cmovz edx,eax
    sub esi,eax
	mov ecx,i_
    sub	edi,ebx
    sub	edi,esi
    movd xmm0,thresh
    pshufd xmm0,xmm0,0
    packssdw xmm0,xmm0
    packuswb xmm0,xmm0
	
JPSDR_Sobel_8_SSE2_1:
    movdqu xmm2,XMMWORD ptr[esi-1]
    movdqu xmm3,XMMWORD ptr[esi]
    movdqu xmm4,XMMWORD ptr[esi+1]
    movdqu xmm5,XMMWORD ptr[esi+edx-1]
    movdqu xmm6,XMMWORD ptr[esi+edx]
    movdqu xmm7,XMMWORD ptr[esi+edx+1]

    movdqa xmm1,xmm2
    pavgb xmm1,xmm4
    pavgb xmm3,xmm1

    movdqa xmm1,xmm5
    pavgb xmm1,xmm7
    pavgb xmm6,xmm1

    movdqa xmm1,xmm3
    psubusb xmm3,xmm6
    psubusb xmm6,xmm1
    por xmm6,xmm3

    movdqu xmm1,XMMWORD ptr[esi+eax-1]
    movdqu xmm3,XMMWORD ptr[esi+eax+1]
    pavgb xmm5,xmm2
    pavgb xmm7,xmm4
    pavgb xmm1,xmm5
    pavgb xmm3,xmm7
    movdqa xmm5,xmm1
    psubusb xmm1,xmm3
    psubusb xmm3,xmm5
    por xmm1,xmm3

    movdqa xmm2,xmm6
    paddusb xmm2,xmm1
    pmaxub xmm1,xmm6
    paddusb xmm2,xmm1

    movdqa xmm3,xmm2
    paddusb xmm2,xmm2
    paddusb xmm2,xmm3
    paddusb xmm2,xmm2
    pminub xmm2,xmm0  ;thresh
    add	esi,ebx
    sub	ecx,4
    jb short JPSDR_Sobel_8_SSE2_2
    movntdq XMMWORD ptr[esi+edi],xmm2
	jz short JPSDR_Sobel_8_SSE2_Fin
    jmp JPSDR_Sobel_8_SSE2_1

JPSDR_Sobel_8_SSE2_2:
    test ecx,2
    jz short JPSDR_Sobel_8_SSE2_3
    movq qword ptr[esi+edi],xmm2
    test ecx,1
    jz short JPSDR_Sobel_8_SSE2_Fin
    add esi,8
    psrldq xmm2,8

JPSDR_Sobel_8_SSE2_3:
    movd dword ptr[esi+edi],xmm2

JPSDR_Sobel_8_SSE2_Fin:
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_Sobel_8_SSE2 endp


JPSDR_Sobel_8_SSE2_a proc psrc:dword,pdst:dword,src_pitch:dword,y_:dword,height:dword,i_:dword,thresh:dword

	public JPSDR_Sobel_8_SSE2_a
	
	push esi
	push edi
	push ebx

	mov ebx,16
    mov esi,psrc
    mov edi,pdst
    mov edx,src_pitch
    xor	eax,eax
    mov ecx,y_
    test ecx,ecx
    cmovnz eax,edx
    inc	ecx
    add	edx,eax
    cmp	ecx,height     ;32 bit O.K.
    cmovz edx,eax
    sub esi,eax
	mov ecx,i_
    sub	edi,ebx
    sub	edi,esi
    movd xmm0,thresh
    pshufd xmm0,xmm0,0
    packssdw xmm0,xmm0
    packuswb xmm0,xmm0
	
	; First pixel
    movdqu xmm2,XMMWORD ptr[esi]
    movdqu xmm3,xmm2
    movdqu xmm4,XMMWORD ptr[esi+1]
    movdqu xmm5,XMMWORD ptr[esi+edx-1]
    movdqu xmm6,XMMWORD ptr[esi+edx]
    movdqu xmm7,XMMWORD ptr[esi+edx+1]

    movdqa xmm1,xmm2
    pavgb xmm1,xmm4
    pavgb xmm3,xmm1

    movdqa xmm1,xmm5
    pavgb xmm1,xmm7
    pavgb xmm6,xmm1

    movdqa xmm1,xmm3
    psubusb xmm3,xmm6
    psubusb xmm6,xmm1
    por xmm6,xmm3

    movdqu xmm1,XMMWORD ptr[esi+eax]
    movdqu xmm3,XMMWORD ptr[esi+eax+1]
    pavgb xmm5,xmm2
    pavgb xmm7,xmm4
    pavgb xmm1,xmm5
    pavgb xmm3,xmm7
    movdqa xmm5,xmm1
    psubusb xmm1,xmm3
    psubusb xmm3,xmm5
    por xmm1,xmm3

    movdqa xmm2,xmm6
    paddusb xmm2,xmm1
    pmaxub xmm1,xmm6
    paddusb xmm2,xmm1

    movdqa xmm3,xmm2
    paddusb xmm2,xmm2
    paddusb xmm2,xmm3
    paddusb xmm2,xmm2
    pminub xmm2,xmm0  ;thresh
    add	esi,ebx
    sub	ecx,4
    jb JPSDR_Sobel_8_SSE2_2_a
    movntdq XMMWORD ptr[esi+edi],xmm2
    jz JPSDR_Sobel_8_SSE2_a_Fin
	
JPSDR_Sobel_8_SSE2_1_a:
    movdqu xmm2,XMMWORD ptr[esi-1]
    movdqu xmm3,XMMWORD ptr[esi]
    movdqu xmm4,XMMWORD ptr[esi+1]
    movdqu xmm5,XMMWORD ptr[esi+edx-1]
    movdqu xmm6,XMMWORD ptr[esi+edx]
    movdqu xmm7,XMMWORD ptr[esi+edx+1]

    movdqa xmm1,xmm2
    pavgb xmm1,xmm4
    pavgb xmm3,xmm1

    movdqa xmm1,xmm5
    pavgb xmm1,xmm7
    pavgb xmm6,xmm1

    movdqa xmm1,xmm3
    psubusb xmm3,xmm6
    psubusb xmm6,xmm1
    por xmm6,xmm3

    movdqu xmm1,XMMWORD ptr[esi+eax-1]
    movdqu xmm3,XMMWORD ptr[esi+eax+1]
    pavgb xmm5,xmm2
    pavgb xmm7,xmm4
    pavgb xmm1,xmm5
    pavgb xmm3,xmm7
    movdqa xmm5,xmm1
    psubusb xmm1,xmm3
    psubusb xmm3,xmm5
    por xmm1,xmm3

    movdqa xmm2,xmm6
    paddusb xmm2,xmm1
    pmaxub xmm1,xmm6
    paddusb xmm2,xmm1

    movdqa xmm3,xmm2
    paddusb xmm2,xmm2
    paddusb xmm2,xmm3
    paddusb xmm2,xmm2
    pminub xmm2,xmm0  ;thresh
    add	esi,ebx
    sub	ecx,4
    jb short JPSDR_Sobel_8_SSE2_2_a
    movntdq XMMWORD ptr[esi+edi],xmm2
	jz short JPSDR_Sobel_8_SSE2_a_Fin
    jmp JPSDR_Sobel_8_SSE2_1_a

JPSDR_Sobel_8_SSE2_2_a:
    test ecx,2
    jz short JPSDR_Sobel_8_SSE2_3_a
    movq qword ptr[esi+edi],xmm2
    test ecx,1
    jz short JPSDR_Sobel_8_SSE2_a_Fin
    add esi,8
    psrldq xmm2,8

JPSDR_Sobel_8_SSE2_3_a:
    movd dword ptr[esi+edi],xmm2

JPSDR_Sobel_8_SSE2_a_Fin:
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_Sobel_8_SSE2_a endp


JPSDR_Sobel_8_SSE2_b proc psrc:dword,pdst:dword,src_pitch:dword,y_:dword,height:dword,i_:dword,thresh:dword

	public JPSDR_Sobel_8_SSE2_b
	
	push esi
	push edi
	push ebx

	mov ebx,16
    mov esi,psrc
    mov edi,pdst
    mov edx,src_pitch
    xor	eax,eax
    mov ecx,y_
    test ecx,ecx
    cmovnz eax,edx
    inc	ecx
    add	edx,eax
    cmp	ecx,height     ;32 bit O.K.
    cmovz edx,eax
    sub esi,eax
	mov ecx,i_
    sub	edi,ebx
    sub	edi,esi
    movd xmm0,thresh
    pshufd xmm0,xmm0,0
    packssdw xmm0,xmm0
    packuswb xmm0,xmm0
	
	sub ecx,4
	jbe JPSDR_Sobel_8_SSE2_4_b
	
JPSDR_Sobel_8_SSE2_1_b:
    movdqu xmm2,XMMWORD ptr[esi-1]
    movdqu xmm3,XMMWORD ptr[esi]
    movdqu xmm4,XMMWORD ptr[esi+1]
    movdqu xmm5,XMMWORD ptr[esi+edx-1]
    movdqu xmm6,XMMWORD ptr[esi+edx]
    movdqu xmm7,XMMWORD ptr[esi+edx+1]

    movdqa xmm1,xmm2
    pavgb xmm1,xmm4
    pavgb xmm3,xmm1

    movdqa xmm1,xmm5
    pavgb xmm1,xmm7
    pavgb xmm6,xmm1

    movdqa xmm1,xmm3
    psubusb xmm3,xmm6
    psubusb xmm6,xmm1
    por xmm6,xmm3

    movdqu xmm1,XMMWORD ptr[esi+eax-1]
    movdqu xmm3,XMMWORD ptr[esi+eax+1]
    pavgb xmm5,xmm2
    pavgb xmm7,xmm4
    pavgb xmm1,xmm5
    pavgb xmm3,xmm7
    movdqa xmm5,xmm1
    psubusb xmm1,xmm3
    psubusb xmm3,xmm5
    por xmm1,xmm3

    movdqa xmm2,xmm6
    paddusb xmm2,xmm1
    pmaxub xmm1,xmm6
    paddusb xmm2,xmm1

    movdqa xmm3,xmm2
    paddusb xmm2,xmm2
    paddusb xmm2,xmm3
    paddusb xmm2,xmm2
    pminub xmm2,xmm0  ;thresh
    add	esi,ebx
    sub	ecx,4
	movntdq XMMWORD ptr[esi+edi],xmm2
    ja JPSDR_Sobel_8_SSE2_1_b
	
	; Last pixel
JPSDR_Sobel_8_SSE2_4_b:
	add ecx,4

    movdqu xmm2,XMMWORD ptr[esi-1]
    movdqu xmm3,XMMWORD ptr[esi]
    movdqu xmm4,XMMWORD ptr[esi+1]
    movdqu xmm5,XMMWORD ptr[esi+edx-1]
    movdqu xmm6,XMMWORD ptr[esi+edx]
    movdqu xmm7,xmm6

    movdqa xmm1,xmm2
    pavgb xmm1,xmm4
    pavgb xmm3,xmm1

    movdqa xmm1,xmm5
    pavgb xmm1,xmm7
    pavgb xmm6,xmm1

    movdqa xmm1,xmm3
    psubusb xmm3,xmm6
    psubusb xmm6,xmm1
    por xmm6,xmm3

    movdqu xmm1,XMMWORD ptr[esi+eax-1]
    movdqu xmm3,XMMWORD ptr[esi+eax]
    pavgb xmm5,xmm2
    pavgb xmm7,xmm4
    pavgb xmm1,xmm5
    pavgb xmm3,xmm7
    movdqa xmm5,xmm1
    psubusb xmm1,xmm3
    psubusb xmm3,xmm5
    por xmm1,xmm3

    movdqa xmm2,xmm6
    paddusb xmm2,xmm1
    pmaxub xmm1,xmm6
    paddusb xmm2,xmm1

    movdqa xmm3,xmm2
    paddusb xmm2,xmm2
    paddusb xmm2,xmm3
    paddusb xmm2,xmm2
    pminub xmm2,xmm0  ;thresh
    add	esi,ebx
    sub	ecx,4
    jb short JPSDR_Sobel_8_SSE2_2_b
    movntdq XMMWORD ptr[esi+edi],xmm2
    jmp short JPSDR_Sobel_8_SSE2_b_Fin

JPSDR_Sobel_8_SSE2_2_b:
    test ecx,2
    jz short JPSDR_Sobel_8_SSE2_3_b
    movq qword ptr[esi+edi],xmm2
    test ecx,1
    jz short JPSDR_Sobel_8_SSE2_b_Fin
    add esi,8
    psrldq xmm2,8

JPSDR_Sobel_8_SSE2_3_b:
    movd dword ptr[esi+edi],xmm2

JPSDR_Sobel_8_SSE2_b_Fin:
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_Sobel_8_SSE2_b endp


JPSDR_Sobel_8_AVX proc psrc:dword,pdst:dword,src_pitch:dword,y_:dword,height:dword,i_:dword,thresh:dword

	public JPSDR_Sobel_8_AVX
	
	push esi
	push edi
	push ebx

	mov ebx,16
    mov esi,psrc
    mov edi,pdst
    mov edx,src_pitch
    xor	eax,eax
    mov ecx,y_
    test ecx,ecx
    cmovnz eax,edx
    inc	ecx
    add	edx,eax
    cmp	ecx,height     ;32 bit O.K.
    cmovz edx,eax
    sub esi,eax
	mov ecx,i_
    sub	edi,ebx
    sub	edi,esi
    vmovd xmm0,thresh
    vpshufd xmm0,xmm0,0
    vpackssdw xmm0,xmm0,xmm0
    vpackuswb xmm0,xmm0,xmm0
	
JPSDR_Sobel_8_AVX_1:
    vmovdqu xmm2,XMMWORD ptr[esi-1]
    vmovdqu xmm3,XMMWORD ptr[esi]
    vmovdqu xmm4,XMMWORD ptr[esi+1]
    vmovdqu xmm5,XMMWORD ptr[esi+edx-1]
    vmovdqu xmm6,XMMWORD ptr[esi+edx]
    vmovdqu xmm7,XMMWORD ptr[esi+edx+1]

    vpavgb xmm1,xmm2,xmm4
    vpavgb xmm3,xmm3,xmm1

    vpavgb xmm1,xmm5,xmm7
    vpavgb xmm6,xmm6,xmm1

    vpsubusb xmm1,xmm3,xmm6
    vpsubusb xmm6,xmm6,xmm3
    vpor xmm6,xmm6,xmm1

    vmovdqu xmm1,XMMWORD ptr[esi+eax-1]
    vmovdqu xmm3,XMMWORD ptr[esi+eax+1]
    vpavgb xmm5,xmm5,xmm2
    vpavgb xmm7,xmm7,xmm4
    vpavgb xmm1,xmm1,xmm5
    vpavgb xmm3,xmm3,xmm7
    vpsubusb xmm5,xmm1,xmm3
    vpsubusb xmm1,xmm3,xmm1
    vpor xmm1,xmm1,xmm5

    vpaddusb xmm2,xmm6,xmm1
    vpmaxub xmm1,xmm1,xmm6
    vpaddusb xmm2,xmm2,xmm1

    vpaddusb xmm1,xmm2,xmm2
    vpaddusb xmm1,xmm1,xmm2
    vpaddusb xmm1,xmm1,xmm1
    vpminub xmm1,xmm1,xmm0  ;thresh
    add	esi,ebx
    sub	ecx,4
    jb short JPSDR_Sobel_8_AVX_2
    vmovntdq XMMWORD ptr[esi+edi],xmm1
	jz short JPSDR_Sobel_8_AVX_Fin
    jmp JPSDR_Sobel_8_AVX_1

JPSDR_Sobel_8_AVX_2:
    test ecx,2
    jz short JPSDR_Sobel_8_AVX_3
    vmovq qword ptr[esi+edi],xmm1
    test ecx,1
    jz short JPSDR_Sobel_8_AVX_Fin
    add esi,8
    vpsrldq xmm1,xmm1,8

JPSDR_Sobel_8_AVX_3:
    vmovd dword ptr[esi+edi],xmm1

JPSDR_Sobel_8_AVX_Fin:
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_Sobel_8_AVX endp


JPSDR_Sobel_8_AVX_a proc psrc:dword,pdst:dword,src_pitch:dword,y_:dword,height:dword,i_:dword,thresh:dword

	public JPSDR_Sobel_8_AVX_a
	
	push esi
	push edi
	push ebx

	mov ebx,16
    mov esi,psrc
    mov edi,pdst
    mov edx,src_pitch
    xor	eax,eax
    mov ecx,y_
    test ecx,ecx
    cmovnz eax,edx
    inc	ecx
    add	edx,eax
    cmp	ecx,height     ;32 bit O.K.
    cmovz edx,eax
    sub esi,eax
	mov ecx,i_
    sub	edi,ebx
    sub	edi,esi
    vmovd xmm0,thresh
    vpshufd xmm0,xmm0,0
    vpackssdw xmm0,xmm0,xmm0
    vpackuswb xmm0,xmm0,xmm0
	
	; First pixel
    vmovdqu xmm2,XMMWORD ptr[esi]
    vmovdqu xmm3,xmm2
    vmovdqu xmm4,XMMWORD ptr[esi+1]
    vmovdqu xmm5,XMMWORD ptr[esi+edx-1]
    vmovdqu xmm6,XMMWORD ptr[esi+edx]
    vmovdqu xmm7,XMMWORD ptr[esi+edx+1]

    vpavgb xmm1,xmm2,xmm4
    vpavgb xmm3,xmm3,xmm1

    vpavgb xmm1,xmm5,xmm7
    vpavgb xmm6,xmm6,xmm1

    vpsubusb xmm1,xmm3,xmm6
    vpsubusb xmm6,xmm6,xmm3
    vpor xmm6,xmm6,xmm1

    vmovdqu xmm1,XMMWORD ptr[esi+eax]
    vmovdqu xmm3,XMMWORD ptr[esi+eax+1]
    vpavgb xmm5,xmm5,xmm2
    vpavgb xmm7,xmm7,xmm4
    vpavgb xmm1,xmm1,xmm5
    vpavgb xmm3,xmm3,xmm7
    vpsubusb xmm5,xmm1,xmm3
    vpsubusb xmm1,xmm3,xmm1
    vpor xmm1,xmm1,xmm5

    vpaddusb xmm2,xmm6,xmm1
    vpmaxub xmm1,xmm1,xmm6
    vpaddusb xmm2,xmm2,xmm1

    vpaddusb xmm1,xmm2,xmm2
    vpaddusb xmm1,xmm1,xmm2
    vpaddusb xmm1,xmm1,xmm1
    vpminub xmm1,xmm1,xmm0  ;thresh
    add	esi,ebx
    sub	ecx,4
    jb JPSDR_Sobel_8_AVX_2_a
    vmovntdq XMMWORD ptr[esi+edi],xmm1
    jz JPSDR_Sobel_8_AVX_a_Fin
	
JPSDR_Sobel_8_AVX_1_a:
    vmovdqu xmm2,XMMWORD ptr[esi-1]
    vmovdqu xmm3,XMMWORD ptr[esi]
    vmovdqu xmm4,XMMWORD ptr[esi+1]
    vmovdqu xmm5,XMMWORD ptr[esi+edx-1]
    vmovdqu xmm6,XMMWORD ptr[esi+edx]
    vmovdqu xmm7,XMMWORD ptr[esi+edx+1]

    vpavgb xmm1,xmm2,xmm4
    vpavgb xmm3,xmm3,xmm1

    vpavgb xmm1,xmm5,xmm7
    vpavgb xmm6,xmm6,xmm1

    vpsubusb xmm1,xmm3,xmm6
    vpsubusb xmm6,xmm6,xmm3
    vpor xmm6,xmm6,xmm1

    vmovdqu xmm1,XMMWORD ptr[esi+eax-1]
    vmovdqu xmm3,XMMWORD ptr[esi+eax+1]
    vpavgb xmm5,xmm5,xmm2
    vpavgb xmm7,xmm7,xmm4
    vpavgb xmm1,xmm1,xmm5
    vpavgb xmm3,xmm3,xmm7
    vpsubusb xmm5,xmm1,xmm3
    vpsubusb xmm1,xmm3,xmm1
    vpor xmm1,xmm1,xmm5

    vpaddusb xmm2,xmm6,xmm1
    vpmaxub xmm1,xmm1,xmm6
    vpaddusb xmm2,xmm2,xmm1

    vpaddusb xmm1,xmm2,xmm2
    vpaddusb xmm1,xmm1,xmm2
    vpaddusb xmm1,xmm1,xmm1
    vpminub xmm1,xmm1,xmm0  ;thresh
    add	esi,ebx
    sub	ecx,4
    jb short JPSDR_Sobel_8_AVX_2_a
    vmovntdq XMMWORD ptr[esi+edi],xmm1
	jz short JPSDR_Sobel_8_AVX_a_Fin
    jmp JPSDR_Sobel_8_AVX_1_a

JPSDR_Sobel_8_AVX_2_a:
    test ecx,2
    jz short JPSDR_Sobel_8_AVX_3_a
    vmovq qword ptr[esi+edi],xmm1
    test ecx,1
    jz short JPSDR_Sobel_8_AVX_a_Fin
    add esi,8
    vpsrldq xmm1,xmm1,8

JPSDR_Sobel_8_AVX_3_a:
    vmovd dword ptr[esi+edi],xmm1

JPSDR_Sobel_8_AVX_a_Fin:
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_Sobel_8_AVX_a endp


JPSDR_Sobel_8_AVX_b proc psrc:dword,pdst:dword,src_pitch:dword,y_:dword,height:dword,i_:dword,thresh:dword

	public JPSDR_Sobel_8_AVX_b
	
	push esi
	push edi
	push ebx

	mov ebx,16
    mov esi,psrc
    mov edi,pdst
    mov edx,src_pitch
    xor	eax,eax
    mov ecx,y_
    test ecx,ecx
    cmovnz eax,edx
    inc	ecx
    add	edx,eax
    cmp	ecx,height     ;32 bit O.K.
    cmovz edx,eax
    sub esi,eax
	mov ecx,i_
    sub	edi,ebx
    sub	edi,esi
    vmovd xmm0,thresh
    vpshufd xmm0,xmm0,0
    vpackssdw xmm0,xmm0,xmm0
    vpackuswb xmm0,xmm0,xmm0
	
    sub	ecx,4
    jbe JPSDR_Sobel_8_AVX_4_b
	
JPSDR_Sobel_8_AVX_1_b:
    vmovdqu xmm2,XMMWORD ptr[esi-1]
    vmovdqu xmm3,XMMWORD ptr[esi]
    vmovdqu xmm4,XMMWORD ptr[esi+1]
    vmovdqu xmm5,XMMWORD ptr[esi+edx-1]
    vmovdqu xmm6,XMMWORD ptr[esi+edx]
    vmovdqu xmm7,XMMWORD ptr[esi+edx+1]

    vpavgb xmm1,xmm2,xmm4
    vpavgb xmm3,xmm3,xmm1

    vpavgb xmm1,xmm5,xmm7
    vpavgb xmm6,xmm6,xmm1

    vpsubusb xmm1,xmm3,xmm6
    vpsubusb xmm6,xmm6,xmm3
    vpor xmm6,xmm6,xmm1

    vmovdqu xmm1,XMMWORD ptr[esi+eax-1]
    vmovdqu xmm3,XMMWORD ptr[esi+eax+1]
    vpavgb xmm5,xmm5,xmm2
    vpavgb xmm7,xmm7,xmm4
    vpavgb xmm1,xmm1,xmm5
    vpavgb xmm3,xmm3,xmm7
    vpsubusb xmm5,xmm1,xmm3
    vpsubusb xmm1,xmm3,xmm1
    vpor xmm1,xmm1,xmm5

    vpaddusb xmm2,xmm6,xmm1
    vpmaxub xmm1,xmm1,xmm6
    vpaddusb xmm2,xmm2,xmm1

    vpaddusb xmm1,xmm2,xmm2
    vpaddusb xmm1,xmm1,xmm2
    vpaddusb xmm1,xmm1,xmm1
    vpminub xmm1,xmm1,xmm0  ;thresh
    add	esi,ebx
    sub	ecx,4
	vmovntdq XMMWORD ptr[esi+edi],xmm1
    ja JPSDR_Sobel_8_AVX_1_b
	
	; Last pixel
JPSDR_Sobel_8_AVX_4_b:
	add ecx,4
	
    vmovdqu xmm2,XMMWORD ptr[esi-1]
    vmovdqu xmm3,XMMWORD ptr[esi]
    vmovdqu xmm4,XMMWORD ptr[esi+1]
    vmovdqu xmm5,XMMWORD ptr[esi+edx-1]
    vmovdqu xmm6,XMMWORD ptr[esi+edx]
    vmovdqu xmm7,xmm6

    vpavgb xmm1,xmm2,xmm4
    vpavgb xmm3,xmm3,xmm1

    vpavgb xmm1,xmm5,xmm7
    vpavgb xmm6,xmm6,xmm1

    vpsubusb xmm1,xmm3,xmm6
    vpsubusb xmm6,xmm6,xmm3
    vpor xmm6,xmm6,xmm1

    vmovdqu xmm1,XMMWORD ptr[esi+eax-1]
    vmovdqu xmm3,XMMWORD ptr[esi+eax]
    vpavgb xmm5,xmm5,xmm2
    vpavgb xmm7,xmm7,xmm4
    vpavgb xmm1,xmm1,xmm5
    vpavgb xmm3,xmm3,xmm7
    vpsubusb xmm5,xmm1,xmm3
    vpsubusb xmm1,xmm3,xmm1
    vpor xmm1,xmm1,xmm5

    vpaddusb xmm2,xmm6,xmm1
    vpmaxub xmm1,xmm1,xmm6
    vpaddusb xmm2,xmm2,xmm1

    vpaddusb xmm1,xmm2,xmm2
    vpaddusb xmm1,xmm1,xmm2
    vpaddusb xmm1,xmm1,xmm1
    vpminub xmm1,xmm1,xmm0  ;thresh
    add	esi,ebx
    sub	ecx,4
    jb short JPSDR_Sobel_8_AVX_2_b
    vmovntdq XMMWORD ptr[esi+edi],xmm1
    jmp short JPSDR_Sobel_8_AVX_b_Fin

JPSDR_Sobel_8_AVX_2_b:
    test ecx,2
    jz short JPSDR_Sobel_8_AVX_3_b
    vmovq qword ptr[esi+edi],xmm1
    test ecx,1
    jz short JPSDR_Sobel_8_AVX_b_Fin
    add esi,8
    vpsrldq xmm1,xmm1,8

JPSDR_Sobel_8_AVX_3_b:
    vmovd dword ptr[esi+edi],xmm1

JPSDR_Sobel_8_AVX_b_Fin:
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_Sobel_8_AVX_b endp


JPSDR_Sobel_16_SSE2 proc psrc:dword,pdst:dword,src_pitch:dword,y_:dword,height:dword,i_:dword,thresh:dword

	public JPSDR_Sobel_16_SSE2
	
	push esi
	push edi
	push ebx

	pxor xmm0,xmm0
	mov ebx,16
    mov esi,psrc
    mov edi,pdst
    mov edx,src_pitch
    xor	eax,eax
    mov ecx,y_
    test ecx,ecx
    cmovnz eax,edx
    inc	ecx
    add	edx,eax
    cmp	ecx,height     ;32 bit O.K.
    cmovz edx,eax
    sub esi,eax
	mov ecx,i_
    sub	edi,ebx
    sub	edi,esi
    movd xmm0,thresh
    pshuflw xmm0,xmm0,0
	movdqa xmm1,xmm0
	pslldq xmm0,8
	por xmm0,xmm1
	psubw xmm0,XMMWORD ptr uw_8000
	
JPSDR_Sobel_16_SSE2_1:
    movdqu xmm2,XMMWORD ptr[esi-2]
    movdqu xmm3,XMMWORD ptr[esi]
    movdqu xmm4,XMMWORD ptr[esi+2]
    movdqu xmm5,XMMWORD ptr[esi+edx-2]
    movdqu xmm6,XMMWORD ptr[esi+edx]
    movdqu xmm7,XMMWORD ptr[esi+edx+2]

    movdqa xmm1,xmm2
    pavgw xmm1,xmm4
    pavgw xmm3,xmm1

    movdqa xmm1,xmm5
    pavgw xmm1,xmm7
    pavgw xmm6,xmm1

    movdqa xmm1,xmm3
    psubusw xmm3,xmm6
    psubusw xmm6,xmm1
    por xmm6,xmm3

    movdqu xmm1,XMMWORD ptr[esi+eax-2]
    movdqu xmm3,XMMWORD ptr[esi+eax+2]
    pavgw xmm5,xmm2
    pavgw xmm7,xmm4
    pavgw xmm1,xmm5
    pavgw xmm3,xmm7
    movdqa xmm5,xmm1
    psubusw xmm1,xmm3
    psubusw xmm3,xmm5
    por xmm1,xmm3

    movdqa xmm2,xmm6
    paddusw xmm2,xmm1
	psubw xmm6,XMMWORD ptr uw_8000
	psubw xmm1,XMMWORD ptr uw_8000
	pmaxsw xmm1,xmm6
	paddw xmm1,XMMWORD ptr uw_8000
    paddusw xmm2,xmm1

    movdqa xmm3,xmm2
    paddusw xmm2,xmm2
    paddusw xmm2,xmm3
    paddusw xmm2,xmm2
	psubw xmm2,XMMWORD ptr uw_8000
	pminsw xmm2,xmm0
	paddw xmm2,XMMWORD ptr uw_8000	
    add	esi,ebx
    sub	ecx,4
    jb short JPSDR_Sobel_16_SSE2_2
    movntdq XMMWORD ptr[esi+edi],xmm2
	jz short JPSDR_Sobel_16_SSE2_Fin
    jmp JPSDR_Sobel_16_SSE2_1

JPSDR_Sobel_16_SSE2_2:
    test ecx,2
    jz short JPSDR_Sobel_16_SSE2_3
    movq qword ptr[esi+edi],xmm2
    test ecx,1
    jz short JPSDR_Sobel_16_SSE2_Fin
    add esi,8
    psrldq xmm2,8

JPSDR_Sobel_16_SSE2_3:
    movd dword ptr[esi+edi],xmm2

JPSDR_Sobel_16_SSE2_Fin:
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_Sobel_16_SSE2 endp


JPSDR_Sobel_16_SSE2_a proc psrc:dword,pdst:dword,src_pitch:dword,y_:dword,height:dword,i_:dword,thresh:dword

	public JPSDR_Sobel_16_SSE2_a
	
	push esi
	push edi
	push ebx

	pxor xmm0,xmm0
	mov ebx,16
    mov esi,psrc
    mov edi,pdst
    mov edx,src_pitch
    xor	eax,eax
    mov ecx,y_
    test ecx,ecx
    cmovnz eax,edx
    inc	ecx
    add	edx,eax
    cmp	ecx,height     ;32 bit O.K.
    cmovz edx,eax
    sub esi,eax
	mov ecx,i_
    sub	edi,ebx
    sub	edi,esi
    movd xmm0,thresh
    pshuflw xmm0,xmm0,0
	movdqa xmm1,xmm0
	pslldq xmm0,8
	por xmm0,xmm1
	psubw xmm0,XMMWORD ptr uw_8000
	
	; Fist pixel
    movdqu xmm2,XMMWORD ptr[esi]
    movdqu xmm3,xmm2
    movdqu xmm4,XMMWORD ptr[esi+2]
    movdqu xmm5,XMMWORD ptr[esi+edx-2]
    movdqu xmm6,XMMWORD ptr[esi+edx]
    movdqu xmm7,XMMWORD ptr[esi+edx+2]

    movdqa xmm1,xmm2
    pavgw xmm1,xmm4
    pavgw xmm3,xmm1

    movdqa xmm1,xmm5
    pavgw xmm1,xmm7
    pavgw xmm6,xmm1

    movdqa xmm1,xmm3
    psubusw xmm3,xmm6
    psubusw xmm6,xmm1
    por xmm6,xmm3

    movdqu xmm1,XMMWORD ptr[esi+eax]
    movdqu xmm3,XMMWORD ptr[esi+eax+2]
    pavgw xmm5,xmm2
    pavgw xmm7,xmm4
    pavgw xmm1,xmm5
    pavgw xmm3,xmm7
    movdqa xmm5,xmm1
    psubusw xmm1,xmm3
    psubusw xmm3,xmm5
    por xmm1,xmm3

    movdqa xmm2,xmm6
    paddusw xmm2,xmm1
	psubw xmm6,XMMWORD ptr uw_8000
	psubw xmm1,XMMWORD ptr uw_8000
	pmaxsw xmm1,xmm6
	paddw xmm1,XMMWORD ptr uw_8000
    paddusw xmm2,xmm1

    movdqa xmm3,xmm2
    paddusw xmm2,xmm2
    paddusw xmm2,xmm3
    paddusw xmm2,xmm2
	psubw xmm2,XMMWORD ptr uw_8000
	pminsw xmm2,xmm0
	paddw xmm2,XMMWORD ptr uw_8000	
    add	esi,ebx
    sub	ecx,4
    jb JPSDR_Sobel_16_SSE2_2_a
    movntdq XMMWORD ptr[esi+edi],xmm2
    jz JPSDR_Sobel_16_SSE2_a_Fin
	
JPSDR_Sobel_16_SSE2_1_a:
    movdqu xmm2,XMMWORD ptr[esi-2]
    movdqu xmm3,XMMWORD ptr[esi]
    movdqu xmm4,XMMWORD ptr[esi+2]
    movdqu xmm5,XMMWORD ptr[esi+edx-2]
    movdqu xmm6,XMMWORD ptr[esi+edx]
    movdqu xmm7,XMMWORD ptr[esi+edx+2]

    movdqa xmm1,xmm2
    pavgw xmm1,xmm4
    pavgw xmm3,xmm1

    movdqa xmm1,xmm5
    pavgw xmm1,xmm7
    pavgw xmm6,xmm1

    movdqa xmm1,xmm3
    psubusw xmm3,xmm6
    psubusw xmm6,xmm1
    por xmm6,xmm3

    movdqu xmm1,XMMWORD ptr[esi+eax-2]
    movdqu xmm3,XMMWORD ptr[esi+eax+2]
    pavgw xmm5,xmm2
    pavgw xmm7,xmm4
    pavgw xmm1,xmm5
    pavgw xmm3,xmm7
    movdqa xmm5,xmm1
    psubusw xmm1,xmm3
    psubusw xmm3,xmm5
    por xmm1,xmm3

    movdqa xmm2,xmm6
    paddusw xmm2,xmm1
	psubw xmm6,XMMWORD ptr uw_8000
	psubw xmm1,XMMWORD ptr uw_8000
	pmaxsw xmm1,xmm6
	paddw xmm1,XMMWORD ptr uw_8000
    paddusw xmm2,xmm1

    movdqa xmm3,xmm2
    paddusw xmm2,xmm2
    paddusw xmm2,xmm3
    paddusw xmm2,xmm2
	psubw xmm2,XMMWORD ptr uw_8000
	pminsw xmm2,xmm0
	paddw xmm2,XMMWORD ptr uw_8000	
    add	esi,ebx
    sub	ecx,4
    jb short JPSDR_Sobel_16_SSE2_2_a
    movntdq XMMWORD ptr[esi+edi],xmm2
	jz short JPSDR_Sobel_16_SSE2_a_Fin
    jmp JPSDR_Sobel_16_SSE2_1_a

JPSDR_Sobel_16_SSE2_2_a:
    test ecx,2
    jz short JPSDR_Sobel_16_SSE2_3_a
    movq qword ptr[esi+edi],xmm2
    test ecx,1
    jz short JPSDR_Sobel_16_SSE2_a_Fin
    add esi,8
    psrldq xmm2,8

JPSDR_Sobel_16_SSE2_3_a:
    movd dword ptr[esi+edi],xmm2

JPSDR_Sobel_16_SSE2_a_Fin:
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_Sobel_16_SSE2_a endp


JPSDR_Sobel_16_SSE2_b proc psrc:dword,pdst:dword,src_pitch:dword,y_:dword,height:dword,i_:dword,thresh:dword

	public JPSDR_Sobel_16_SSE2_b
	
	push esi
	push edi
	push ebx

	pxor xmm0,xmm0
	mov ebx,16
    mov esi,psrc
    mov edi,pdst
    mov edx,src_pitch
    xor	eax,eax
    mov ecx,y_
    test ecx,ecx
    cmovnz eax,edx
    inc	ecx
    add	edx,eax
    cmp	ecx,height     ;32 bit O.K.
    cmovz edx,eax
    sub esi,eax
	mov ecx,i_
    sub	edi,ebx
    sub	edi,esi
    movd xmm0,thresh
    pshuflw xmm0,xmm0,0
	movdqa xmm1,xmm0
	pslldq xmm0,8
	por xmm0,xmm1
	psubw xmm0,XMMWORD ptr uw_8000
	
    sub	ecx,4
    jbe JPSDR_Sobel_16_SSE2_4_b
	
JPSDR_Sobel_16_SSE2_1_b:
    movdqu xmm2,XMMWORD ptr[esi-2]
    movdqu xmm3,XMMWORD ptr[esi]
    movdqu xmm4,XMMWORD ptr[esi+2]
    movdqu xmm5,XMMWORD ptr[esi+edx-2]
    movdqu xmm6,XMMWORD ptr[esi+edx]
    movdqu xmm7,XMMWORD ptr[esi+edx+2]

    movdqa xmm1,xmm2
    pavgw xmm1,xmm4
    pavgw xmm3,xmm1

    movdqa xmm1,xmm5
    pavgw xmm1,xmm7
    pavgw xmm6,xmm1

    movdqa xmm1,xmm3
    psubusw xmm3,xmm6
    psubusw xmm6,xmm1
    por xmm6,xmm3

    movdqu xmm1,XMMWORD ptr[esi+eax-2]
    movdqu xmm3,XMMWORD ptr[esi+eax+2]
    pavgw xmm5,xmm2
    pavgw xmm7,xmm4
    pavgw xmm1,xmm5
    pavgw xmm3,xmm7
    movdqa xmm5,xmm1
    psubusw xmm1,xmm3
    psubusw xmm3,xmm5
    por xmm1,xmm3

    movdqa xmm2,xmm6
    paddusw xmm2,xmm1
	psubw xmm6,XMMWORD ptr uw_8000
	psubw xmm1,XMMWORD ptr uw_8000
	pmaxsw xmm1,xmm6
	paddw xmm1,XMMWORD ptr uw_8000
    paddusw xmm2,xmm1

    movdqa xmm3,xmm2
    paddusw xmm2,xmm2
    paddusw xmm2,xmm3
    paddusw xmm2,xmm2
	psubw xmm2,XMMWORD ptr uw_8000
	pminsw xmm2,xmm0
	paddw xmm2,XMMWORD ptr uw_8000	
    add	esi,ebx
    sub	ecx,4
    movntdq XMMWORD ptr[esi+edi],xmm2
    ja JPSDR_Sobel_16_SSE2_1_b
	
	; Last pixel
JPSDR_Sobel_16_SSE2_4_b:
	add ecx,4
	
    movdqu xmm2,XMMWORD ptr[esi-2]
    movdqu xmm3,XMMWORD ptr[esi]
    movdqu xmm4,XMMWORD ptr[esi+2]
    movdqu xmm5,XMMWORD ptr[esi+edx-2]
    movdqu xmm6,XMMWORD ptr[esi+edx]
    movdqu xmm7,xmm6

    movdqa xmm1,xmm2
    pavgw xmm1,xmm4
    pavgw xmm3,xmm1

    movdqa xmm1,xmm5
    pavgw xmm1,xmm7
    pavgw xmm6,xmm1

    movdqa xmm1,xmm3
    psubusw xmm3,xmm6
    psubusw xmm6,xmm1
    por xmm6,xmm3

    movdqu xmm1,XMMWORD ptr[esi+eax-2]
    movdqu xmm3,XMMWORD ptr[esi+eax]
    pavgw xmm5,xmm2
    pavgw xmm7,xmm4
    pavgw xmm1,xmm5
    pavgw xmm3,xmm7
    movdqa xmm5,xmm1
    psubusw xmm1,xmm3
    psubusw xmm3,xmm5
    por xmm1,xmm3

    movdqa xmm2,xmm6
    paddusw xmm2,xmm1
	psubw xmm6,XMMWORD ptr uw_8000
	psubw xmm1,XMMWORD ptr uw_8000
	pmaxsw xmm1,xmm6
	paddw xmm1,XMMWORD ptr uw_8000
    paddusw xmm2,xmm1

    movdqa xmm3,xmm2
    paddusw xmm2,xmm2
    paddusw xmm2,xmm3
    paddusw xmm2,xmm2
	psubw xmm2,XMMWORD ptr uw_8000
	pminsw xmm2,xmm0
	paddw xmm2,XMMWORD ptr uw_8000	
    add	esi,ebx
    sub	ecx,4
    jb short JPSDR_Sobel_16_SSE2_2_b
    movntdq XMMWORD ptr[esi+edi],xmm2
    jmp short JPSDR_Sobel_16_SSE2_b_Fin

JPSDR_Sobel_16_SSE2_2_b:
    test ecx,2
    jz short JPSDR_Sobel_16_SSE2_3_b
    movq qword ptr[esi+edi],xmm2
    test ecx,1
    jz short JPSDR_Sobel_16_SSE2_b_Fin
    add esi,8
    psrldq xmm2,8

JPSDR_Sobel_16_SSE2_3_b:
    movd dword ptr[esi+edi],xmm2

JPSDR_Sobel_16_SSE2_b_Fin:
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_Sobel_16_SSE2_b endp


JPSDR_Sobel_16_AVX proc psrc:dword,pdst:dword,src_pitch:dword,y_:dword,height:dword,i_:dword,thresh:dword

	public JPSDR_Sobel_16_AVX
	
	push esi
	push edi
	push ebx

	mov ebx,16
    mov esi,psrc
    mov edi,pdst
    mov edx,src_pitch
    xor	eax,eax
    mov ecx,y_
    test ecx,ecx
    cmovnz eax,edx
    inc	ecx
    add	edx,eax
    cmp	ecx,height     ;32 bit O.K.
    cmovz edx,eax
    sub esi,eax
	mov ecx,i_
    sub	edi,ebx
    sub	edi,esi
    vmovd xmm0,thresh
    vpshufd xmm0,xmm0,0
	vpackusdw xmm0,xmm0,xmm0
	
JPSDR_Sobel_16_AVX_1:
    vmovdqu xmm2,XMMWORD ptr[esi-2]
    vmovdqu xmm3,XMMWORD ptr[esi]
    vmovdqu xmm4,XMMWORD ptr[esi+2]
    vmovdqu xmm5,XMMWORD ptr[esi+edx-2]
    vmovdqu xmm6,XMMWORD ptr[esi+edx]
    vmovdqu xmm7,XMMWORD ptr[esi+edx+2]

    vpavgw xmm1,xmm2,xmm4
    vpavgw xmm3,xmm3,xmm1

    vpavgw xmm1,xmm5,xmm7
    vpavgw xmm6,xmm6,xmm1

    vpsubusw xmm1,xmm3,xmm6
    vpsubusw xmm6,xmm6,xmm3
    vpor xmm6,xmm6,xmm1

    vmovdqu xmm1,XMMWORD ptr[esi+eax-2]
    vmovdqu xmm3,XMMWORD ptr[esi+eax+2]
    vpavgw xmm5,xmm5,xmm2
    vpavgw xmm7,xmm7,xmm4
    vpavgw xmm1,xmm1,xmm5
    vpavgw xmm3,xmm3,xmm7
	
    vpsubusw xmm5,xmm1,xmm3
    vpsubusw xmm1,xmm3,xmm1
    vpor xmm1,xmm1,xmm5

    vpaddusw xmm2,xmm6,xmm1
    vpmaxuw xmm1,xmm1,xmm6
    vpaddusw xmm2,xmm2,xmm1

    vpaddusw xmm1,xmm2,xmm2
    vpaddusw xmm1,xmm1,xmm2
    vpaddusw xmm1,xmm1,xmm1
    vpminuw xmm1,xmm1,xmm0  ;thresh
    add	esi,ebx
    sub	ecx,4
    jb short JPSDR_Sobel_16_AVX_2
    vmovntdq XMMWORD ptr[esi+edi],xmm1
	jz short JPSDR_Sobel_16_AVX_Fin
    jmp JPSDR_Sobel_16_AVX_1

JPSDR_Sobel_16_AVX_2:
    test ecx,2
    jz short JPSDR_Sobel_16_AVX_3
    vmovq qword ptr[esi+edi],xmm1
    test ecx,1
    jz short JPSDR_Sobel_16_AVX_Fin
    add esi,8
    vpsrldq xmm1,xmm1,8

JPSDR_Sobel_16_AVX_3:
    vmovd dword ptr[esi+edi],xmm1

JPSDR_Sobel_16_AVX_Fin:
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_Sobel_16_AVX endp


JPSDR_Sobel_16_AVX_a proc psrc:dword,pdst:dword,src_pitch:dword,y_:dword,height:dword,i_:dword,thresh:dword

	public JPSDR_Sobel_16_AVX_a
	
	push esi
	push edi
	push ebx

	mov ebx,16
    mov esi,psrc
    mov edi,pdst
    mov edx,src_pitch
    xor	eax,eax
    mov ecx,y_
    test ecx,ecx
    cmovnz eax,edx
    inc	ecx
    add	edx,eax
    cmp	ecx,height     ;32 bit O.K.
    cmovz edx,eax
    sub esi,eax
	mov ecx,i_
    sub	edi,ebx
    sub	edi,esi
    vmovd xmm0,thresh
    vpshufd xmm0,xmm0,0
	vpackusdw xmm0,xmm0,xmm0
	
	; First pixel
    vmovdqu xmm2,XMMWORD ptr[esi]
    vmovdqu xmm3,xmm2
    vmovdqu xmm4,XMMWORD ptr[esi+2]
    vmovdqu xmm5,XMMWORD ptr[esi+edx-2]
    vmovdqu xmm6,XMMWORD ptr[esi+edx]
    vmovdqu xmm7,XMMWORD ptr[esi+edx+2]

    vpavgw xmm1,xmm2,xmm4
    vpavgw xmm3,xmm3,xmm1

    vpavgw xmm1,xmm5,xmm7
    vpavgw xmm6,xmm6,xmm1

    vpsubusw xmm1,xmm3,xmm6
    vpsubusw xmm6,xmm6,xmm3
    vpor xmm6,xmm6,xmm1

    vmovdqu xmm1,XMMWORD ptr[esi+eax]
    vmovdqu xmm3,XMMWORD ptr[esi+eax+2]
    vpavgw xmm5,xmm5,xmm2
    vpavgw xmm7,xmm7,xmm4
    vpavgw xmm1,xmm1,xmm5
    vpavgw xmm3,xmm3,xmm7
	
    vpsubusw xmm5,xmm1,xmm3
    vpsubusw xmm1,xmm3,xmm1
    vpor xmm1,xmm1,xmm5

    vpaddusw xmm2,xmm6,xmm1
    vpmaxuw xmm1,xmm1,xmm6
    vpaddusw xmm2,xmm2,xmm1

    vpaddusw xmm1,xmm2,xmm2
    vpaddusw xmm1,xmm1,xmm2
    vpaddusw xmm1,xmm1,xmm1
    vpminuw xmm1,xmm1,xmm0  ;thresh
    add	esi,ebx
    sub	ecx,4
    jb JPSDR_Sobel_16_AVX_2_a
    vmovntdq XMMWORD ptr[esi+edi],xmm1
    jz JPSDR_Sobel_16_AVX_a_Fin
	
JPSDR_Sobel_16_AVX_1_a:
    vmovdqu xmm2,XMMWORD ptr[esi-2]
    vmovdqu xmm3,XMMWORD ptr[esi]
    vmovdqu xmm4,XMMWORD ptr[esi+2]
    vmovdqu xmm5,XMMWORD ptr[esi+edx-2]
    vmovdqu xmm6,XMMWORD ptr[esi+edx]
    vmovdqu xmm7,XMMWORD ptr[esi+edx+2]

    vpavgw xmm1,xmm2,xmm4
    vpavgw xmm3,xmm3,xmm1

    vpavgw xmm1,xmm5,xmm7
    vpavgw xmm6,xmm6,xmm1

    vpsubusw xmm1,xmm3,xmm6
    vpsubusw xmm6,xmm6,xmm3
    vpor xmm6,xmm6,xmm1

    vmovdqu xmm1,XMMWORD ptr[esi+eax-2]
    vmovdqu xmm3,XMMWORD ptr[esi+eax+2]
    vpavgw xmm5,xmm5,xmm2
    vpavgw xmm7,xmm7,xmm4
    vpavgw xmm1,xmm1,xmm5
    vpavgw xmm3,xmm3,xmm7
	
    vpsubusw xmm5,xmm1,xmm3
    vpsubusw xmm1,xmm3,xmm1
    vpor xmm1,xmm1,xmm5

    vpaddusw xmm2,xmm6,xmm1
    vpmaxuw xmm1,xmm1,xmm6
    vpaddusw xmm2,xmm2,xmm1

    vpaddusw xmm1,xmm2,xmm2
    vpaddusw xmm1,xmm1,xmm2
    vpaddusw xmm1,xmm1,xmm1
    vpminuw xmm1,xmm1,xmm0  ;thresh
    add	esi,ebx
    sub	ecx,4
    jb short JPSDR_Sobel_16_AVX_2_a
    vmovntdq XMMWORD ptr[esi+edi],xmm1
	jz short JPSDR_Sobel_16_AVX_a_Fin
    jmp JPSDR_Sobel_16_AVX_1_a

JPSDR_Sobel_16_AVX_2_a:
    test ecx,2
    jz short JPSDR_Sobel_16_AVX_3_a
    vmovq qword ptr[esi+edi],xmm1
    test ecx,1
    jz short JPSDR_Sobel_16_AVX_a_Fin
    add esi,8
    vpsrldq xmm1,xmm1,8

JPSDR_Sobel_16_AVX_3_a:
    vmovd dword ptr[esi+edi],xmm1

JPSDR_Sobel_16_AVX_a_Fin:
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_Sobel_16_AVX_a endp


JPSDR_Sobel_16_AVX_b proc psrc:dword,pdst:dword,src_pitch:dword,y_:dword,height:dword,i_:dword,thresh:dword

	public JPSDR_Sobel_16_AVX_b
	
	push esi
	push edi
	push ebx

	mov ebx,16
    mov esi,psrc
    mov edi,pdst
    mov edx,src_pitch
    xor	eax,eax
    mov ecx,y_
    test ecx,ecx
    cmovnz eax,edx
    inc	ecx
    add	edx,eax
    cmp	ecx,height     ;32 bit O.K.
    cmovz edx,eax
    sub esi,eax
	mov ecx,i_
    sub	edi,ebx
    sub	edi,esi
    vmovd xmm0,thresh
    vpshufd xmm0,xmm0,0
	vpackusdw xmm0,xmm0,xmm0
	
    sub	ecx,4
    jbe JPSDR_Sobel_16_AVX_4_b
	
JPSDR_Sobel_16_AVX_1_b:
    vmovdqu xmm2,XMMWORD ptr[esi-2]
    vmovdqu xmm3,XMMWORD ptr[esi]
    vmovdqu xmm4,XMMWORD ptr[esi+2]
    vmovdqu xmm5,XMMWORD ptr[esi+edx-2]
    vmovdqu xmm6,XMMWORD ptr[esi+edx]
    vmovdqu xmm7,XMMWORD ptr[esi+edx+2]

    vpavgw xmm1,xmm2,xmm4
    vpavgw xmm3,xmm3,xmm1

    vpavgw xmm1,xmm5,xmm7
    vpavgw xmm6,xmm6,xmm1

    vpsubusw xmm1,xmm3,xmm6
    vpsubusw xmm6,xmm6,xmm3
    vpor xmm6,xmm6,xmm1

    vmovdqu xmm1,XMMWORD ptr[esi+eax-2]
    vmovdqu xmm3,XMMWORD ptr[esi+eax+2]
    vpavgw xmm5,xmm5,xmm2
    vpavgw xmm7,xmm7,xmm4
    vpavgw xmm1,xmm1,xmm5
    vpavgw xmm3,xmm3,xmm7
	
    vpsubusw xmm5,xmm1,xmm3
    vpsubusw xmm1,xmm3,xmm1
    vpor xmm1,xmm1,xmm5

    vpaddusw xmm2,xmm6,xmm1
    vpmaxuw xmm1,xmm1,xmm6
    vpaddusw xmm2,xmm2,xmm1

    vpaddusw xmm1,xmm2,xmm2
    vpaddusw xmm1,xmm1,xmm2
    vpaddusw xmm1,xmm1,xmm1
    vpminuw xmm1,xmm1,xmm0  ;thresh
    add	esi,ebx
    sub	ecx,4
    vmovntdq XMMWORD ptr[esi+edi],xmm1
    ja JPSDR_Sobel_16_AVX_1_b
	
	; Last pixel
JPSDR_Sobel_16_AVX_4_b:
	add ecx,4
	
    vmovdqu xmm2,XMMWORD ptr[esi-2]
    vmovdqu xmm3,XMMWORD ptr[esi]
    vmovdqu xmm4,XMMWORD ptr[esi+2]
    vmovdqu xmm5,XMMWORD ptr[esi+edx-2]
    vmovdqu xmm6,XMMWORD ptr[esi+edx]
    vmovdqu xmm7,xmm6

    vpavgw xmm1,xmm2,xmm4
    vpavgw xmm3,xmm3,xmm1

    vpavgw xmm1,xmm5,xmm7
    vpavgw xmm6,xmm6,xmm1

    vpsubusw xmm1,xmm3,xmm6
    vpsubusw xmm6,xmm6,xmm3
    vpor xmm6,xmm6,xmm1

    vmovdqu xmm1,XMMWORD ptr[esi+eax-2]
    vmovdqu xmm3,XMMWORD ptr[esi+eax]
    vpavgw xmm5,xmm5,xmm2
    vpavgw xmm7,xmm7,xmm4
    vpavgw xmm1,xmm1,xmm5
    vpavgw xmm3,xmm3,xmm7
	
    vpsubusw xmm5,xmm1,xmm3
    vpsubusw xmm1,xmm3,xmm1
    vpor xmm1,xmm1,xmm5

    vpaddusw xmm2,xmm6,xmm1
    vpmaxuw xmm1,xmm1,xmm6
    vpaddusw xmm2,xmm2,xmm1

    vpaddusw xmm1,xmm2,xmm2
    vpaddusw xmm1,xmm1,xmm2
    vpaddusw xmm1,xmm1,xmm1
    vpminuw xmm1,xmm1,xmm0  ;thresh
    add	esi,ebx
    sub	ecx,4
    jb short JPSDR_Sobel_16_AVX_2_b
    vmovntdq XMMWORD ptr[esi+edi],xmm1
    jmp short JPSDR_Sobel_16_AVX_b_Fin

JPSDR_Sobel_16_AVX_2_b:
    test ecx,2
    jz short JPSDR_Sobel_16_AVX_3_b
    vmovq qword ptr[esi+edi],xmm1
    test ecx,1
    jz short JPSDR_Sobel_16_AVX_b_Fin
    add esi,8
    vpsrldq xmm1,xmm1,8

JPSDR_Sobel_16_AVX_3_b:
    vmovd dword ptr[esi+edi],xmm1

JPSDR_Sobel_16_AVX_b_Fin:
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_Sobel_16_AVX_b endp


JPSDR_H_BlurR6_8_AVX proc psrc2:dword,ptmp2:dword,src_row_size:dword,dq0toF:dword

	public JPSDR_H_BlurR6_8_AVX
	
	push esi
	push edi

	mov eax,16
    mov esi,psrc2
    mov edi,ptmp2
    mov ecx,src_row_size
    add esi,eax
    sub edi,esi
    vmovdqa xmm6,XMMWORD ptr[esi-16]
    vmovdqa xmm5,xmm6
    vmovdqa xmm7,xmm6
    vpxor xmm0,xmm0,xmm0
    vpshufb xmm5,xmm5,xmm0
    sub ecx,eax
    jna JPSDR_H_BlurR6_8_AVX_2
    
JPSDR_H_BlurR6_8_AVX_1:
    vmovdqa xmm7,XMMWORD ptr[esi]
    vpalignr xmm0,xmm6,xmm5,10
    vpalignr xmm2,xmm7,xmm6,6
    vpavgb xmm0,xmm0,xmm2
    vpalignr xmm3,xmm6,xmm5,11
    vpalignr xmm4,xmm7,xmm6,5
    vpavgb xmm3,xmm3,xmm4
    vpavgb xmm0,xmm0,xmm3
    vpalignr xmm1,xmm6,xmm5,12
    vpalignr xmm2,xmm7,xmm6,4
    vpavgb xmm1,xmm1,xmm2
    vpalignr xmm3,xmm6,xmm5,13
    vpalignr xmm4,xmm7,xmm6,3
    vpavgb xmm3,xmm3,xmm4
    vpavgb xmm1,xmm1,xmm3
    vpavgb xmm0,xmm0,xmm1
    vpalignr xmm1,xmm6,xmm5,14
    vpalignr xmm2,xmm7,xmm6,2
    vpavgb xmm1,xmm1,xmm2
    vpalignr xmm3,xmm6,xmm5,15
    vpalignr xmm4,xmm7,xmm6,1
    vpavgb xmm3,xmm3,xmm4
    vpavgb xmm1,xmm1,xmm3
    vpavgb xmm1,xmm1,xmm6
    vmovdqa xmm5,xmm6
    vmovdqa xmm6,xmm7
    vpavgb xmm0,xmm0,xmm1
    vpavgb xmm0,xmm0,xmm1
    vmovntdq XMMWORD ptr[esi+edi],xmm0
    add esi,eax
    sub ecx,eax
    ja JPSDR_H_BlurR6_8_AVX_1
	
JPSDR_H_BlurR6_8_AVX_2:
    add	ecx,15
	mov edx,dq0toF
    vpxor xmm0,xmm0,xmm0
    vmovd xmm1,ecx
    vpshufb xmm1,xmm1,xmm0
    vpminub xmm1,xmm1,XMMWORD ptr[edx]	;0x0F0E..00
    vpshufb xmm6,xmm6,xmm1
    vpsrldq xmm7,xmm7,15
    vpshufb xmm7,xmm7,xmm0
    vpalignr xmm0,xmm6,xmm5,10
    vpalignr xmm2,xmm7,xmm6,6
    vpavgb xmm0,xmm0,xmm2
    vpalignr xmm3,xmm6,xmm5,11
    vpalignr xmm4,xmm7,xmm6,5
    vpavgb xmm3,xmm3,xmm4
    vpavgb xmm0,xmm0,xmm3
    vpalignr xmm1,xmm6,xmm5,12
    vpalignr xmm2,xmm7,xmm6,4
    vpavgb xmm1,xmm1,xmm2
    vpalignr xmm3,xmm6,xmm5,13
    vpalignr xmm4,xmm7,xmm6,3
    vpavgb xmm3,xmm3,xmm4
    vpavgb xmm1,xmm1,xmm3
    vpavgb xmm0,xmm0,xmm1
    vpalignr xmm1,xmm6,xmm5,14
    vpalignr xmm2,xmm7,xmm6,2
    vpavgb xmm1,xmm1,xmm2
    vpalignr xmm3,xmm6,xmm5,15
    vpalignr xmm4,xmm7,xmm6,1
    vpavgb xmm3,xmm3,xmm4
    vpavgb xmm1,xmm1,xmm3
    vpavgb xmm1,xmm1,xmm6
    vpavgb xmm0,xmm0,xmm1
    vpavgb xmm0,xmm0,xmm1
    vmovntdq XMMWORD ptr[esi+edi],xmm0
	
	pop edi
	pop esi

	ret
JPSDR_H_BlurR6_8_AVX endp


JPSDR_H_BlurR6_8_SSE2 proc psrc2:dword,ptmp2:dword,src_row_size_16:dword

	public JPSDR_H_BlurR6_8_SSE2
	
	push esi
	push edi

    mov esi,psrc2
    mov edi,ptmp2
    mov ecx,src_row_size_16
    sub edi,esi
	mov eax,16

JPSDR_H_BlurR6_8_SSE2_1:
    movdqu xmm6,XMMWORD ptr[esi-6]
    movdqu xmm5,XMMWORD ptr[esi-5]
    movdqu xmm4,XMMWORD ptr[esi-4]
    movdqu xmm3,XMMWORD ptr[esi-3]
    movdqu xmm2,XMMWORD ptr[esi-2]
    movdqu xmm1,XMMWORD ptr[esi-1]
	movdqu xmm0,XMMWORD ptr[esi+1]
	movdqu xmm7,XMMWORD ptr[esi+2]
	pavgb xmm1,xmm0
	pavgb xmm2,xmm7
	movdqu xmm0,XMMWORD ptr[esi+3]
	movdqu xmm7,XMMWORD ptr[esi+4]
	pavgb xmm3,xmm0
	pavgb xmm4,xmm7
	movdqu xmm0,XMMWORD ptr[esi+5]
	movdqu xmm7,XMMWORD ptr[esi+6]
	pavgb xmm5,xmm0
	pavgb xmm6,xmm7
	movdqu xmm0,XMMWORD ptr[esi]
    pavgb xmm6,xmm5
    pavgb xmm4,xmm3
    pavgb xmm2,xmm1
    pavgb xmm6,xmm4
    pavgb xmm2,xmm0
    pavgb xmm6,xmm2
    pavgb xmm6,xmm2
    movdqu XMMWORD ptr[esi+edi],xmm6
    add esi,eax
	loop JPSDR_H_BlurR6_8_SSE2_1
	
	pop edi
	pop esi

	ret
JPSDR_H_BlurR6_8_SSE2 endp


JPSDR_H_BlurR6_16_SSE2 proc psrc2:dword,ptmp2:dword,src_row_size_16:dword

	public JPSDR_H_BlurR6_16_SSE2
	
	push esi
	push edi

    mov esi,psrc2
    mov edi,ptmp2
    mov ecx,src_row_size_16
    sub edi,esi
	mov eax,16

JPSDR_H_BlurR6_16_SSE2_1:
    movdqu xmm6,XMMWORD ptr[esi-12]
    movdqu xmm5,XMMWORD ptr[esi-10]
    movdqu xmm4,XMMWORD ptr[esi-8]
    movdqu xmm3,XMMWORD ptr[esi-6]
    movdqu xmm2,XMMWORD ptr[esi-4]
    movdqu xmm1,XMMWORD ptr[esi-2]
    movdqu xmm0,XMMWORD ptr[esi+2]
    movdqu xmm7,XMMWORD ptr[esi+4]
	pavgw xmm1,xmm0
	pavgw xmm2,xmm7
	movdqu xmm0,XMMWORD ptr[esi+6]
	movdqu xmm7,XMMWORD ptr[esi+8]
	pavgw xmm3,xmm0
	pavgw xmm4,xmm7
	movdqu xmm0,XMMWORD ptr[esi+10]
	movdqu xmm7,XMMWORD ptr[esi+12]
	pavgw xmm5,xmm0
	pavgw xmm6,xmm7
	movdqu xmm0,XMMWORD ptr[esi]
    pavgw xmm6,xmm5
    pavgw xmm4,xmm3
    pavgw xmm2,xmm1
    pavgw xmm6,xmm4
    pavgw xmm2,xmm0
    pavgw xmm6,xmm2
    pavgw xmm6,xmm2
    movdqu XMMWORD ptr[esi+edi],xmm6
    add esi,eax
	loop JPSDR_H_BlurR6_16_SSE2_1
	
	pop edi
	pop esi

	ret
JPSDR_H_BlurR6_16_SSE2 endp


JPSDR_H_BlurR6_16_AVX proc psrc2:dword,ptmp2:dword,src_row_size_16:dword

	public JPSDR_H_BlurR6_16_AVX
	
	push esi
	push edi

    mov esi,psrc2
    mov edi,ptmp2
    mov ecx,src_row_size_16
    sub edi,esi
	mov eax,16

JPSDR_H_BlurR6_16_AVX_1:
    vmovdqu xmm6,XMMWORD ptr[esi-12]
    vmovdqu xmm5,XMMWORD ptr[esi-10]
    vmovdqu xmm4,XMMWORD ptr[esi-8]
    vmovdqu xmm3,XMMWORD ptr[esi-6]
    vmovdqu xmm2,XMMWORD ptr[esi-4]
    vmovdqu xmm1,XMMWORD ptr[esi-2]
    vmovdqu xmm0,XMMWORD ptr[esi+2]
    vmovdqu xmm7,XMMWORD ptr[esi+4]
	vpavgw xmm1,xmm1,xmm0
	vpavgw xmm2,xmm2,xmm7
	vmovdqu xmm0,XMMWORD ptr[esi+6]
	vmovdqu xmm7,XMMWORD ptr[esi+8]
	vpavgw xmm3,xmm3,xmm0
	vpavgw xmm4,xmm4,xmm7
	vmovdqu xmm0,XMMWORD ptr[esi+10]
	vmovdqu xmm7,XMMWORD ptr[esi+12]
	vpavgw xmm5,xmm5,xmm0
	vpavgw xmm6,xmm6,xmm7
    vmovdqu xmm0,XMMWORD ptr[esi]
    vpavgw xmm6,xmm6,xmm5
    vpavgw xmm4,xmm4,xmm3
    vpavgw xmm2,xmm2,xmm1
    vpavgw xmm6,xmm6,xmm4
    vpavgw xmm2,xmm2,xmm0
    vpavgw xmm6,xmm6,xmm2
    vpavgw xmm6,xmm6,xmm2
    vmovdqu XMMWORD ptr[esi+edi],xmm6
    add esi,eax
	loop JPSDR_H_BlurR6_16_AVX_1
	
	pop edi
	pop esi

	ret
JPSDR_H_BlurR6_16_AVX endp


JPSDR_H_BlurR6a_8_SSE2 proc psrc2:dword,ptmp2:dword

	public JPSDR_H_BlurR6a_8_SSE2

    mov ecx,psrc2
    mov edx,ptmp2

	movdqa xmm0,XMMWORD ptr[ecx]
    movdqu xmm1,XMMWORD ptr[ecx+1]
    movdqu xmm2,XMMWORD ptr[ecx+2]
    movdqu xmm3,XMMWORD ptr[ecx+3]
    movdqu xmm4,XMMWORD ptr[ecx+4]
    movdqu xmm5,XMMWORD ptr[ecx+5]
    movdqu xmm6,XMMWORD ptr[ecx+6]
	pavgb xmm1,xmm2
	pavgb xmm3,xmm4
	pavgb xmm5,xmm6
	pavgb xmm1,xmm0
	pavgb xmm3,xmm5
	pavgb xmm3,xmm1
	pavgb xmm3,xmm1	
    movntdq XMMWORD ptr[edx],xmm3

	ret
JPSDR_H_BlurR6a_8_SSE2 endp


JPSDR_H_BlurR6a_16_SSE2 proc psrc2:dword,ptmp2:dword

	public JPSDR_H_BlurR6a_16_SSE2

    mov ecx,psrc2
    mov edx,ptmp2

	movdqa xmm0,XMMWORD ptr[ecx]
    movdqu xmm1,XMMWORD ptr[ecx+2]
    movdqu xmm2,XMMWORD ptr[ecx+4]
    movdqu xmm3,XMMWORD ptr[ecx+6]
    movdqu xmm4,XMMWORD ptr[ecx+8]
    movdqu xmm5,XMMWORD ptr[ecx+10]
    movdqu xmm6,XMMWORD ptr[ecx+12]
	pavgw xmm1,xmm2
	pavgw xmm3,xmm4
	pavgw xmm5,xmm6
	pavgw xmm1,xmm0
	pavgw xmm3,xmm5
	pavgw xmm3,xmm1
	pavgw xmm3,xmm1	
    movntdq XMMWORD ptr[edx],xmm3

	ret
JPSDR_H_BlurR6a_16_SSE2 endp


JPSDR_H_BlurR6a_16_AVX proc psrc2:dword,ptmp2:dword

	public JPSDR_H_BlurR6a_16_AVX

    mov ecx,psrc2
    mov edx,ptmp2

	vmovdqa xmm0,XMMWORD ptr[ecx]
    vmovdqu xmm1,XMMWORD ptr[ecx+2]
    vmovdqu xmm2,XMMWORD ptr[ecx+4]
    vmovdqu xmm3,XMMWORD ptr[ecx+6]
    vmovdqu xmm4,XMMWORD ptr[ecx+8]
    vmovdqu xmm5,XMMWORD ptr[ecx+10]
    vmovdqu xmm6,XMMWORD ptr[ecx+12]
	vpavgw xmm1,xmm1,xmm2
	vpavgw xmm3,xmm3,xmm4
	vpavgw xmm5,xmm5,xmm6
	vpavgw xmm1,xmm1,xmm0
	vpavgw xmm3,xmm3,xmm5
	vpavgw xmm3,xmm3,xmm1
	vpavgw xmm3,xmm3,xmm1	
    vmovntdq XMMWORD ptr[edx],xmm3

	ret
JPSDR_H_BlurR6a_16_AVX endp


JPSDR_H_BlurR6b_8_SSE2 proc psrc2:dword,ptmp2:dword

	public JPSDR_H_BlurR6b_8_SSE2

    mov ecx,psrc2
    mov edx,ptmp2

    movdqu xmm6,XMMWORD ptr[ecx-6]
    movdqu xmm5,XMMWORD ptr[ecx-5]
    movdqu xmm4,XMMWORD ptr[ecx-4]
    movdqu xmm3,XMMWORD ptr[ecx-3]
    movdqu xmm2,XMMWORD ptr[ecx-2]
    movdqu xmm1,XMMWORD ptr[ecx-1]
	movdqu xmm0,XMMWORD ptr[ecx+1]
	movdqu xmm7,XMMWORD ptr[ecx+2]
	pavgb xmm1,xmm0
	pavgb xmm2,xmm7
	movdqu xmm0,XMMWORD ptr[ecx+3]
	movdqu xmm7,XMMWORD ptr[ecx+4]
	pavgb xmm3,xmm0
	pavgb xmm4,xmm7
	movdqu xmm0,XMMWORD ptr[ecx+5]
	movdqu xmm7,XMMWORD ptr[ecx+6]
	pavgb xmm5,xmm0
	pavgb xmm6,xmm7
	movdqu xmm0,XMMWORD ptr[ecx]
    pavgb xmm6,xmm5
    pavgb xmm4,xmm3
    pavgb xmm2,xmm1
	
    pavgb xmm6,xmm4
    pavgb xmm2,xmm0
    pavgb xmm6,xmm2
    pavgb xmm6,xmm2
    movdqu XMMWORD ptr[edx],xmm6

	ret
JPSDR_H_BlurR6b_8_SSE2 endp


JPSDR_H_BlurR6b_16_SSE2 proc psrc2:dword,ptmp2:dword

	public JPSDR_H_BlurR6b_16_SSE2

    mov ecx,psrc2
    mov edx,ptmp2

    movdqu xmm6,XMMWORD ptr[ecx-12]
    movdqu xmm5,XMMWORD ptr[ecx-10]
    movdqu xmm4,XMMWORD ptr[ecx-8]
    movdqu xmm3,XMMWORD ptr[ecx-6]
    movdqu xmm2,XMMWORD ptr[ecx-4]
    movdqu xmm1,XMMWORD ptr[ecx-2]
	movdqu xmm0,XMMWORD ptr[ecx+2]
	movdqu xmm7,XMMWORD ptr[ecx+4]
	pavgw xmm1,xmm0
	pavgw xmm2,xmm7
	movdqu xmm0,XMMWORD ptr[ecx+6]
	movdqu xmm7,XMMWORD ptr[ecx+8]
	pavgw xmm3,xmm0
	pavgw xmm4,xmm7
	movdqu xmm0,XMMWORD ptr[ecx+10]
	movdqu xmm7,XMMWORD ptr[ecx+12]
	pavgw xmm5,xmm0
	pavgw xmm6,xmm7
	movdqu xmm0,XMMWORD ptr[ecx]
    pavgw xmm6,xmm5
    pavgw xmm4,xmm3
    pavgw xmm2,xmm1
	
    pavgw xmm6,xmm4
    pavgw xmm2,xmm0
    pavgw xmm6,xmm2
    pavgw xmm6,xmm2
    movdqu XMMWORD ptr[edx],xmm6

	ret
JPSDR_H_BlurR6b_16_SSE2 endp


JPSDR_H_BlurR6b_16_AVX proc psrc2:dword,ptmp2:dword

	public JPSDR_H_BlurR6b_16_AVX

    mov ecx,psrc2
    mov edx,ptmp2

    vmovdqu xmm6,XMMWORD ptr[ecx-12]
    vmovdqu xmm5,XMMWORD ptr[ecx-10]
    vmovdqu xmm4,XMMWORD ptr[ecx-8]
    vmovdqu xmm3,XMMWORD ptr[ecx-6]
    vmovdqu xmm2,XMMWORD ptr[ecx-4]
    vmovdqu xmm1,XMMWORD ptr[ecx-2]
	vmovdqu xmm0,XMMWORD ptr[ecx+2]
	vmovdqu xmm7,XMMWORD ptr[ecx+4]
	vpavgw xmm1,xmm1,xmm0
	vpavgw xmm2,xmm2,xmm7
	vmovdqu xmm0,XMMWORD ptr[ecx+6]
	vmovdqu xmm7,XMMWORD ptr[ecx+8]
	vpavgw xmm3,xmm3,xmm0
	vpavgw xmm4,xmm4,xmm7
	vmovdqu xmm0,XMMWORD ptr[ecx+10]
	vmovdqu xmm7,XMMWORD ptr[ecx+12]
	vpavgw xmm5,xmm5,xmm0
	vpavgw xmm6,xmm6,xmm7
	vmovdqu xmm0,XMMWORD ptr[ecx]
    vpavgw xmm6,xmm6,xmm5
    vpavgw xmm4,xmm4,xmm3
    vpavgw xmm2,xmm2,xmm1
	
    vpavgw xmm6,xmm6,xmm4
    vpavgw xmm2,xmm2,xmm0
    vpavgw xmm6,xmm6,xmm2
    vpavgw xmm6,xmm6,xmm2
    vmovdqu XMMWORD ptr[edx],xmm6

	ret
JPSDR_H_BlurR6b_16_AVX endp


JPSDR_H_BlurR6c_8_SSE2 proc psrc2:dword,ptmp2:dword

	public JPSDR_H_BlurR6c_8_SSE2

    mov ecx,psrc2
    mov edx,ptmp2

	movdqu xmm6,XMMWORD ptr[ecx-6]
    movdqu xmm5,XMMWORD ptr[ecx-5]
    movdqu xmm4,XMMWORD ptr[ecx-4]
    movdqu xmm3,XMMWORD ptr[ecx-3]
    movdqu xmm2,XMMWORD ptr[ecx-2]
    movdqu xmm1,XMMWORD ptr[ecx-1]
    movdqu xmm0,XMMWORD ptr[ecx]
	pavgb xmm5,xmm6
	pavgb xmm1,xmm2
	pavgb xmm3,xmm4
	pavgb xmm0,xmm1
	pavgb xmm3,xmm5
	pavgb xmm3,xmm0
	pavgb xmm3,xmm0
	movzx eax,word ptr[edx]
	movzx ecx,word ptr[edx+2]
	pinsrw xmm3,eax,0
	pinsrw xmm3,ecx,1
	movzx eax,word ptr[edx+4]
	movzx ecx,word ptr[edx+6]
	pinsrw xmm3,eax,2
	pinsrw xmm3,ecx,3
	movzx eax,word ptr[edx+8]
	pinsrw xmm3,eax,4
    movdqu XMMWORD ptr[edx],xmm3

	ret
JPSDR_H_BlurR6c_8_SSE2 endp


JPSDR_H_BlurR6c_16_SSE2 proc psrc2:dword,ptmp2:dword

	public JPSDR_H_BlurR6c_16_SSE2

    mov ecx,psrc2
    mov edx,ptmp2

	movdqu xmm6,XMMWORD ptr[ecx-12]
    movdqu xmm5,XMMWORD ptr[ecx-10]
    movdqu xmm4,XMMWORD ptr[ecx-8]
    movdqu xmm3,XMMWORD ptr[ecx-6]
    movdqu xmm2,XMMWORD ptr[ecx-4]
    movdqu xmm1,XMMWORD ptr[ecx-2]
    movdqu xmm0,XMMWORD ptr[ecx]
	pavgw xmm5,xmm6
	pavgw xmm1,xmm2
	pavgw xmm3,xmm4
	pavgw xmm0,xmm1
	pavgw xmm3,xmm5
	pavgw xmm3,xmm0
	pavgw xmm3,xmm0
	movzx eax,word ptr[edx]
	movzx ecx,word ptr[edx+2]
	pinsrw xmm3,eax,0
	pinsrw xmm3,ecx,1
    movdqu XMMWORD ptr[edx],xmm3

	ret
JPSDR_H_BlurR6c_16_SSE2 endp


JPSDR_H_BlurR6c_16_AVX proc psrc2:dword,ptmp2:dword

	public JPSDR_H_BlurR6c_16_AVX

    mov ecx,psrc2
    mov edx,ptmp2

	vmovdqu xmm6,XMMWORD ptr[ecx-12]
    vmovdqu xmm5,XMMWORD ptr[ecx-10]
    vmovdqu xmm4,XMMWORD ptr[ecx-8]
    vmovdqu xmm3,XMMWORD ptr[ecx-6]
    vmovdqu xmm2,XMMWORD ptr[ecx-4]
    vmovdqu xmm1,XMMWORD ptr[ecx-2]
    vmovdqu xmm0,XMMWORD ptr[ecx]
	vpavgw xmm5,xmm5,xmm6
	vpavgw xmm1,xmm1,xmm2
	vpavgw xmm3,xmm3,xmm4
	vpavgw xmm0,xmm0,xmm1
	vpavgw xmm3,xmm3,xmm5
	vpavgw xmm3,xmm3,xmm0
	vpavgw xmm3,xmm3,xmm0
	movzx eax,word ptr[edx]
	movzx ecx,word ptr[edx+2]
	vpinsrw xmm3,xmm3,eax,0
	vpinsrw xmm3,xmm3,ecx,1
    vmovdqu XMMWORD ptr[edx],xmm3

	ret
JPSDR_H_BlurR6c_16_AVX endp


JPSDR_V_BlurR6a_8_SSE2 proc psrc2:dword,ptmp2:dword,tmp_pitch:dword,src_row_size_16:dword

	public JPSDR_V_BlurR6a_8_SSE2
	
	push esi
	push edi
	push ebx
	
	mov eax,tmp_pitch
	mov	esi,ptmp2
	mov	edi,psrc2
	mov ecx,src_row_size_16
	lea	ebx,[eax+2*eax]
	lea	edx,[ebx+2*eax]
	add	edx,esi
	sub	edi,esi
	
JPSDR_V_BlurR6a_8_SSE2_1:
	movdqa xmm0,XMMWORD ptr[esi]
	movdqa xmm1,XMMWORD ptr[esi+eax]
	movdqa xmm2,XMMWORD ptr[esi+2*eax]
	movdqa xmm3,XMMWORD ptr[esi+ebx]
	movdqa xmm4,XMMWORD ptr[esi+4*eax]
	movdqa xmm5,XMMWORD ptr[edx]
	movdqa xmm6,XMMWORD ptr[edx+eax]
	pavgb xmm6,xmm5
	pavgb xmm4,xmm3
	pavgb xmm2,xmm1
	pavgb xmm6,xmm4
	pavgb xmm2,xmm0
	pavgb xmm6,xmm2
	pavgb xmm6,xmm2
	movntdq XMMWORD ptr[esi+edi],xmm6
	add esi,16
	add edx,16
	loop JPSDR_V_BlurR6a_8_SSE2_1

	pop ebx
	pop edi
	pop esi

	ret
JPSDR_V_BlurR6a_8_SSE2 endp


JPSDR_V_BlurR6a_8_AVX proc psrc2:dword,ptmp2:dword,tmp_pitch:dword,src_row_size_16:dword

	public JPSDR_V_BlurR6a_8_AVX
	
	push esi
	push edi
	push ebx
	
	mov eax,tmp_pitch
	mov	esi,ptmp2
	mov	edi,psrc2
	mov ecx,src_row_size_16
	lea	ebx,[eax+2*eax]
	lea	edx,[ebx+2*eax]
	add	edx,esi
	sub	edi,esi
	
JPSDR_V_BlurR6a_8_AVX_1:
	vmovdqa xmm0,XMMWORD ptr[esi]
	vmovdqa xmm1,XMMWORD ptr[esi+eax]
	vmovdqa xmm2,XMMWORD ptr[esi+2*eax]
	vmovdqa xmm3,XMMWORD ptr[esi+ebx]
	vmovdqa xmm4,XMMWORD ptr[esi+4*eax]
	vmovdqa xmm5,XMMWORD ptr[edx]
	vmovdqa xmm6,XMMWORD ptr[edx+eax]
	vpavgb xmm6,xmm6,xmm5
	vpavgb xmm4,xmm4,xmm3
	vpavgb xmm2,xmm2,xmm1
	vpavgb xmm6,xmm6,xmm4
	vpavgb xmm2,xmm2,xmm0
	vpavgb xmm6,xmm6,xmm2
	vpavgb xmm6,xmm6,xmm2
	vmovntdq XMMWORD ptr[esi+edi],xmm6
	add esi,16
	add edx,16
	loop JPSDR_V_BlurR6a_8_AVX_1

	pop ebx
	pop edi
	pop esi

	ret
JPSDR_V_BlurR6a_8_AVX endp


JPSDR_V_BlurR6a_16_SSE2 proc psrc2:dword,ptmp2:dword,tmp_pitch:dword,src_row_size_16:dword

	public JPSDR_V_BlurR6a_16_SSE2
	
	push esi
	push edi
	push ebx
	
	mov eax,tmp_pitch
	mov	esi,ptmp2
	mov	edi,psrc2
	mov ecx,src_row_size_16
	lea	ebx,[eax+2*eax]
	lea	edx,[ebx+2*eax]
	add	edx,esi
	sub	edi,esi
	
JPSDR_V_BlurR6a_16_SSE2_1:
	movdqa xmm0,XMMWORD ptr[esi]
	movdqa xmm1,XMMWORD ptr[esi+eax]
	movdqa xmm2,XMMWORD ptr[esi+2*eax]
	movdqa xmm3,XMMWORD ptr[esi+ebx]
	movdqa xmm4,XMMWORD ptr[esi+4*eax]
	movdqa xmm5,XMMWORD ptr[edx]
	movdqa xmm6,XMMWORD ptr[edx+eax]
	pavgw xmm6,xmm5
	pavgw xmm4,xmm3
	pavgw xmm2,xmm1
	pavgw xmm6,xmm4
	pavgw xmm2,xmm0
	pavgw xmm6,xmm2
	pavgw xmm6,xmm2
	movntdq XMMWORD ptr[esi+edi],xmm6
	add esi,16
	add edx,16
	loop JPSDR_V_BlurR6a_16_SSE2_1

	pop ebx
	pop edi
	pop esi

	ret
JPSDR_V_BlurR6a_16_SSE2 endp


JPSDR_V_BlurR6a_16_AVX proc psrc2:dword,ptmp2:dword,tmp_pitch:dword,src_row_size_16:dword

	public JPSDR_V_BlurR6a_16_AVX
	
	push esi
	push edi
	push ebx
	
	mov eax,tmp_pitch
	mov	esi,ptmp2
	mov	edi,psrc2
	mov ecx,src_row_size_16
	lea	ebx,[eax+2*eax]
	lea	edx,[ebx+2*eax]
	add	edx,esi
	sub	edi,esi
	
JPSDR_V_BlurR6a_16_AVX_1:
	vmovdqa xmm0,XMMWORD ptr[esi]
	vmovdqa xmm1,XMMWORD ptr[esi+eax]
	vmovdqa xmm2,XMMWORD ptr[esi+2*eax]
	vmovdqa xmm3,XMMWORD ptr[esi+ebx]
	vmovdqa xmm4,XMMWORD ptr[esi+4*eax]
	vmovdqa xmm5,XMMWORD ptr[edx]
	vmovdqa xmm6,XMMWORD ptr[edx+eax]
	vpavgw xmm6,xmm6,xmm5
	vpavgw xmm4,xmm4,xmm3
	vpavgw xmm2,xmm2,xmm1
	vpavgw xmm6,xmm6,xmm4
	vpavgw xmm2,xmm2,xmm0
	vpavgw xmm6,xmm6,xmm2
	vpavgw xmm6,xmm6,xmm2
	vmovntdq XMMWORD ptr[esi+edi],xmm6
	add esi,16
	add edx,16
	loop JPSDR_V_BlurR6a_16_AVX_1

	pop ebx
	pop edi
	pop esi

	ret
JPSDR_V_BlurR6a_16_AVX endp


JPSDR_V_BlurR6b_8_SSE2 proc psrc2:dword,ptmp2:dword,tmp_pitch:dword,src_row_size_16:dword

	public JPSDR_V_BlurR6b_8_SSE2
	
	push esi
	push edi
	push ebx
	push ebp
	
	mov eax,tmp_pitch
	mov esi,ptmp2
	mov edi,psrc2
	mov ecx,src_row_size_16
	lea ebp,[eax+2*eax]
	lea edx,[ebp+2*eax]
	lea ebx,[esi+2*edx]
	add edx,esi
	sub edi,esi
	
JPSDR_V_BlurR6b_8_SSE2_1:
	movdqa xmm6,XMMWORD ptr[esi]
	pavgb xmm6,XMMWORD ptr[ebx+2*eax]
	movdqa xmm5,XMMWORD ptr[esi+eax]
	pavgb xmm5,XMMWORD ptr[ebx+eax]
	movdqa xmm4,XMMWORD ptr[esi+2*eax]
	pavgb xmm4,XMMWORD ptr[ebx]
	movdqa xmm3,XMMWORD ptr[esi+ebp]
	pavgb xmm3,XMMWORD ptr[edx+4*eax]
	movdqa xmm2,XMMWORD ptr[esi+4*eax]
	pavgb xmm2,XMMWORD ptr[esi+8*eax]
	movdqa xmm1,XMMWORD ptr[edx]
	pavgb xmm1,XMMWORD ptr[edx+2*eax]
	movdqa xmm0,XMMWORD ptr[edx+eax]
	pavgb xmm6,xmm5
	pavgb xmm4,xmm3
	pavgb xmm2,xmm1
	pavgb xmm6,xmm4
	pavgb xmm2,xmm0
	pavgb xmm6,xmm2
	pavgb xmm6,xmm2
	
	movntdq XMMWORD ptr[esi+edi],xmm6
	add esi,16
	add edx,16
	add ebx,16
	loop JPSDR_V_BlurR6b_8_SSE2_1

	pop ebp
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_V_BlurR6b_8_SSE2 endp


JPSDR_V_BlurR6b_8_AVX proc psrc2:dword,ptmp2:dword,tmp_pitch:dword,src_row_size_16:dword

	public JPSDR_V_BlurR6b_8_AVX
	
	push esi
	push edi
	push ebx
	push ebp
	
	mov eax,tmp_pitch
	mov esi,ptmp2
	mov edi,psrc2
	mov ecx,src_row_size_16
	lea ebp,[eax+2*eax]
	lea edx,[ebp+2*eax]
	lea ebx,[esi+2*edx]
	add edx,esi
	sub edi,esi
	
JPSDR_V_BlurR6b_8_AVX_1:
	vmovdqa xmm6,XMMWORD ptr[esi]
	vpavgb xmm6,xmm6,XMMWORD ptr[ebx+2*eax]
	vmovdqa xmm5,XMMWORD ptr[esi+eax]
	vpavgb xmm5,xmm5,XMMWORD ptr[ebx+eax]
	vmovdqa xmm4,XMMWORD ptr[esi+2*eax]
	vpavgb xmm4,xmm4,XMMWORD ptr[ebx]
	vmovdqa xmm3,XMMWORD ptr[esi+ebp]
	vpavgb xmm3,xmm3,XMMWORD ptr[edx+4*eax]
	vmovdqa xmm2,XMMWORD ptr[esi+4*eax]
	vpavgb xmm2,xmm2,XMMWORD ptr[esi+8*eax]
	vmovdqa xmm1,XMMWORD ptr[edx]
	vpavgb xmm1,xmm1,XMMWORD ptr[edx+2*eax]
	vmovdqa xmm0,XMMWORD ptr[edx+eax]
	vpavgb xmm6,xmm6,xmm5
	vpavgb xmm4,xmm4,xmm3
	vpavgb xmm2,xmm2,xmm1
	vpavgb xmm6,xmm6,xmm4
	vpavgb xmm2,xmm2,xmm0
	vpavgb xmm6,xmm6,xmm2
	vpavgb xmm6,xmm6,xmm2
	
	vmovntdq XMMWORD ptr[esi+edi],xmm6
	add esi,16
	add edx,16
	add ebx,16
	loop JPSDR_V_BlurR6b_8_AVX_1

	pop ebp
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_V_BlurR6b_8_AVX endp
	

JPSDR_V_BlurR6b_16_SSE2 proc psrc2:dword,ptmp2:dword,tmp_pitch:dword,src_row_size_16:dword

	public JPSDR_V_BlurR6b_16_SSE2
	
	push esi
	push edi
	push ebx
	push ebp
	
	mov eax,tmp_pitch
	mov esi,ptmp2
	mov edi,psrc2
	mov ecx,src_row_size_16
	lea ebp,[eax+2*eax]
	lea edx,[ebp+2*eax]
	lea ebx,[esi+2*edx]
	add edx,esi
	sub edi,esi
	
JPSDR_V_BlurR6b_16_SSE2_1:
	movdqa xmm6,XMMWORD ptr[esi]
	pavgw xmm6,XMMWORD ptr[ebx+2*eax]
	movdqa xmm5,XMMWORD ptr[esi+eax]
	pavgw xmm5,XMMWORD ptr[ebx+eax]
	movdqa xmm4,XMMWORD ptr[esi+2*eax]
	pavgw xmm4,XMMWORD ptr[ebx]
	movdqa xmm3,XMMWORD ptr[esi+ebp]
	pavgw xmm3,XMMWORD ptr[edx+4*eax]
	movdqa xmm2,XMMWORD ptr[esi+4*eax]
	pavgw xmm2,XMMWORD ptr[esi+8*eax]
	movdqa xmm1,XMMWORD ptr[edx]
	pavgw xmm1,XMMWORD ptr[edx+2*eax]
	movdqa xmm0,XMMWORD ptr[edx+eax]
	pavgw xmm6,xmm5
	pavgw xmm4,xmm3
	pavgw xmm2,xmm1
	pavgw xmm6,xmm4
	pavgw xmm2,xmm0
	pavgw xmm6,xmm2
	pavgw xmm6,xmm2
	
	movntdq XMMWORD ptr[esi+edi],xmm6
	add esi,16
	add edx,16
	add ebx,16
	loop JPSDR_V_BlurR6b_16_SSE2_1

	pop ebp
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_V_BlurR6b_16_SSE2 endp


JPSDR_V_BlurR6b_16_AVX proc psrc2:dword,ptmp2:dword,tmp_pitch:dword,src_row_size_16:dword

	public JPSDR_V_BlurR6b_16_AVX
	
	push esi
	push edi
	push ebx
	push ebp
	
	mov eax,tmp_pitch
	mov esi,ptmp2
	mov edi,psrc2
	mov ecx,src_row_size_16
	lea ebp,[eax+2*eax]
	lea edx,[ebp+2*eax]
	lea ebx,[esi+2*edx]
	add edx,esi
	sub edi,esi
	
JPSDR_V_BlurR6b_16_AVX_1:
	vmovdqa xmm6,XMMWORD ptr[esi]
	vpavgw xmm6,xmm6,XMMWORD ptr[ebx+2*eax]
	vmovdqa xmm5,XMMWORD ptr[esi+eax]
	vpavgw xmm5,xmm5,XMMWORD ptr[ebx+eax]
	vmovdqa xmm4,XMMWORD ptr[esi+2*eax]
	vpavgw xmm4,xmm4,XMMWORD ptr[ebx]
	vmovdqa xmm3,XMMWORD ptr[esi+ebp]
	vpavgw xmm3,xmm3,XMMWORD ptr[edx+4*eax]
	vmovdqa xmm2,XMMWORD ptr[esi+4*eax]
	vpavgw xmm2,xmm2,XMMWORD ptr[esi+8*eax]
	vmovdqa xmm1,XMMWORD ptr[edx]
	vpavgw xmm1,xmm1,XMMWORD ptr[edx+2*eax]
	vmovdqa xmm0,XMMWORD ptr[edx+eax]
	vpavgw xmm6,xmm6,xmm5
	vpavgw xmm4,xmm4,xmm3
	vpavgw xmm2,xmm2,xmm1
	vpavgw xmm6,xmm6,xmm4
	vpavgw xmm2,xmm2,xmm0
	vpavgw xmm6,xmm6,xmm2
	vpavgw xmm6,xmm6,xmm2
	
	vmovntdq XMMWORD ptr[esi+edi],xmm6
	add esi,16
	add edx,16
	add ebx,16
	loop JPSDR_V_BlurR6b_16_AVX_1

	pop ebp
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_V_BlurR6b_16_AVX endp


JPSDR_V_BlurR6c_8_SSE2 proc psrc2:dword,ptmp2:dword,tmp_pitch:dword,src_row_size_16:dword

	public JPSDR_V_BlurR6c_8_SSE2
	
	push esi
	push edi
	push ebx
	
	mov eax,tmp_pitch
	mov	esi,ptmp2
	mov	edi,psrc2
	mov ecx,src_row_size_16
	lea	ebx,[eax+2*eax]
	lea	edx,[ebx+2*eax]
	add	edx,esi
	sub	edi,esi
	
JPSDR_V_BlurR6c_8_SSE2_1:
	movdqa xmm6,XMMWORD ptr[esi]
	movdqa xmm5,XMMWORD ptr[esi+eax]
	movdqa xmm4,XMMWORD ptr[esi+2*eax]
	movdqa xmm3,XMMWORD ptr[esi+ebx]
	movdqa xmm2,XMMWORD ptr[esi+4*eax]
	movdqa xmm1,XMMWORD ptr[edx]
	movdqa xmm0,XMMWORD ptr[edx+eax]
	pavgb xmm6,xmm5
	pavgb xmm4,xmm3
	pavgb xmm2,xmm1
	pavgb xmm6,xmm4
	pavgb xmm2,xmm0
	pavgb xmm6,xmm2
	pavgb xmm6,xmm2
	movntdq XMMWORD ptr[esi+edi],xmm6
	add esi,16
	add edx,16
	loop JPSDR_V_BlurR6c_8_SSE2_1

	pop ebx
	pop edi
	pop esi

	ret
JPSDR_V_BlurR6c_8_SSE2 endp


JPSDR_V_BlurR6c_8_AVX proc psrc2:dword,ptmp2:dword,tmp_pitch:dword,src_row_size_16:dword

	public JPSDR_V_BlurR6c_8_AVX
	
	push esi
	push edi
	push ebx
	
	mov eax,tmp_pitch
	mov	esi,ptmp2
	mov	edi,psrc2
	mov ecx,src_row_size_16
	lea	ebx,[eax+2*eax]
	lea	edx,[ebx+2*eax]
	add	edx,esi
	sub	edi,esi
	
JPSDR_V_BlurR6c_8_AVX_1:
	vmovdqa xmm6,XMMWORD ptr[esi]
	vmovdqa xmm5,XMMWORD ptr[esi+eax]
	vmovdqa xmm4,XMMWORD ptr[esi+2*eax]
	vmovdqa xmm3,XMMWORD ptr[esi+ebx]
	vmovdqa xmm2,XMMWORD ptr[esi+4*eax]
	vmovdqa xmm1,XMMWORD ptr[edx]
	vmovdqa xmm0,XMMWORD ptr[edx+eax]
	vpavgb xmm6,xmm6,xmm5
	vpavgb xmm4,xmm4,xmm3
	vpavgb xmm2,xmm2,xmm1
	vpavgb xmm6,xmm6,xmm4
	vpavgb xmm2,xmm2,xmm0
	vpavgb xmm6,xmm6,xmm2
	vpavgb xmm6,xmm6,xmm2
	vmovntdq XMMWORD ptr[esi+edi],xmm6
	add esi,16
	add edx,16
	loop JPSDR_V_BlurR6c_8_AVX_1

	pop ebx
	pop edi
	pop esi

	ret
JPSDR_V_BlurR6c_8_AVX endp


JPSDR_V_BlurR6c_16_SSE2 proc psrc2:dword,ptmp2:dword,tmp_pitch:dword,src_row_size_16:dword

	public JPSDR_V_BlurR6c_16_SSE2
	
	push esi
	push edi
	push ebx
	
	mov eax,tmp_pitch
	mov	esi,ptmp2
	mov	edi,psrc2
	mov ecx,src_row_size_16
	lea	ebx,[eax+2*eax]
	lea	edx,[ebx+2*eax]
	add	edx,esi
	sub	edi,esi
	
JPSDR_V_BlurR6c_16_SSE2_1:
	movdqa xmm6,XMMWORD ptr[esi]
	movdqa xmm5,XMMWORD ptr[esi+eax]
	movdqa xmm4,XMMWORD ptr[esi+2*eax]
	movdqa xmm3,XMMWORD ptr[esi+ebx]
	movdqa xmm2,XMMWORD ptr[esi+4*eax]
	movdqa xmm1,XMMWORD ptr[edx]
	movdqa xmm0,XMMWORD ptr[edx+eax]
	pavgw xmm6,xmm5
	pavgw xmm4,xmm3
	pavgw xmm2,xmm1
	pavgw xmm6,xmm4
	pavgw xmm2,xmm0
	pavgw xmm6,xmm2
	pavgw xmm6,xmm2
	movntdq XMMWORD ptr[esi+edi],xmm6
	add esi,16
	add edx,16
	loop JPSDR_V_BlurR6c_16_SSE2_1

	pop ebx
	pop edi
	pop esi

	ret
JPSDR_V_BlurR6c_16_SSE2 endp
	
	
JPSDR_V_BlurR6c_16_AVX proc psrc2:dword,ptmp2:dword,tmp_pitch:dword,src_row_size_16:dword

	public JPSDR_V_BlurR6c_16_AVX
	
	push esi
	push edi
	push ebx
	
	mov eax,tmp_pitch
	mov	esi,ptmp2
	mov	edi,psrc2
	mov ecx,src_row_size_16
	lea	ebx,[eax+2*eax]
	lea	edx,[ebx+2*eax]
	add	edx,esi
	sub	edi,esi
	
JPSDR_V_BlurR6c_16_AVX_1:
	vmovdqa xmm6,XMMWORD ptr[esi]
	vmovdqa xmm5,XMMWORD ptr[esi+eax]
	vmovdqa xmm4,XMMWORD ptr[esi+2*eax]
	vmovdqa xmm3,XMMWORD ptr[esi+ebx]
	vmovdqa xmm2,XMMWORD ptr[esi+4*eax]
	vmovdqa xmm1,XMMWORD ptr[edx]
	vmovdqa xmm0,XMMWORD ptr[edx+eax]
	vpavgw xmm6,xmm6,xmm5
	vpavgw xmm4,xmm4,xmm3
	vpavgw xmm2,xmm2,xmm1
	vpavgw xmm6,xmm6,xmm4
	vpavgw xmm2,xmm2,xmm0
	vpavgw xmm6,xmm6,xmm2
	vpavgw xmm6,xmm6,xmm2
	vmovntdq XMMWORD ptr[esi+edi],xmm6
	add esi,16
	add edx,16
	loop JPSDR_V_BlurR6c_16_AVX_1

	pop ebx
	pop edi
	pop esi

	ret
JPSDR_V_BlurR6c_16_AVX endp
	

JPSDR_H_BlurR2_8_AVX proc psrc2:dword,ptmp2:dword,i_:dword,dq0toF:dword

	public JPSDR_H_BlurR2_8_AVX
	
	push esi
	push edi

	mov esi,dq0toF
	vmovdqa xmm4,XMMWORD ptr[esi]
	mov eax,16
	mov esi,psrc2
	mov edi,ptmp2
	mov ecx,i_
	add esi,eax
	sub edi,esi
	vmovdqa xmm6,XMMWORD ptr[esi-16]
	vmovdqa xmm5,xmm6
	vmovdqa xmm7,xmm6
	vpxor xmm0,xmm0,xmm0
	vpshufb xmm5,xmm5,xmm0
	sub ecx,eax
	jna short JPSDR_H_BlurR2_8_AVX_2
	
JPSDR_H_BlurR2_8_AVX_1:
	vmovdqa xmm7,XMMWORD ptr[esi]
	vpalignr xmm0,xmm6,xmm5,14
	vpalignr xmm2,xmm7,xmm6,2
	vpavgb xmm0,xmm0,xmm2
	vpalignr xmm1,xmm6,xmm5,15
	vpalignr xmm3,xmm7,xmm6,1
	vpavgb xmm0,xmm0,xmm6
	vpavgb xmm1,xmm1,xmm3
	vpavgb xmm0,xmm0,xmm6
	vmovdqa xmm5,xmm6
	vmovdqa xmm6,xmm7
	vpavgb xmm0,xmm0,xmm1
	vmovntdq XMMWORD ptr[esi+edi],xmm0
	add esi,eax
	sub ecx,eax
	ja short JPSDR_H_BlurR2_8_AVX_1
	
JPSDR_H_BlurR2_8_AVX_2:
	add ecx,15
	vpxor xmm0,xmm0,xmm0
	vmovd xmm1,ecx
	vpshufb xmm1,xmm1,xmm0
	vpminub xmm1,xmm1,xmm4 ;0x0F0E..00
	vpshufb xmm6,xmm6,xmm1
	vpsrldq xmm7,xmm7,15
	vpshufb xmm7,xmm7,xmm0
	vpalignr xmm0,xmm6,xmm5,14
	vpalignr xmm2,xmm7,xmm6,2
	vpavgb xmm0,xmm0,xmm2
	vpalignr xmm1,xmm6,xmm5,15
	vpalignr xmm3,xmm7,xmm6,1
	vpavgb xmm0,xmm0,xmm6
	vpavgb xmm1,xmm1,xmm3
	vpavgb xmm0,xmm0,xmm6
	vpavgb xmm0,xmm0,xmm1
	vmovntdq XMMWORD ptr[esi+edi],xmm0
	
	pop edi
	pop esi

	ret
JPSDR_H_BlurR2_8_AVX endp


JPSDR_H_BlurR2_8_SSE2 proc psrc2:dword,ptmp2:dword,ia:dword

	public JPSDR_H_BlurR2_8_SSE2

	push esi
	push edi

	mov eax,16	
	mov esi,psrc2
	mov edi,ptmp2
	mov ecx,ia
	add esi,ecx
	add edi,ecx
	neg ecx
	
JPSDR_H_BlurR2_8_SSE2_1:
	movdqu xmm0,XMMWORD ptr[ecx+esi-2]
	movdqu xmm1,XMMWORD ptr[ecx+esi-1]
	movdqu xmm2,XMMWORD ptr[ecx+esi]
	movdqu xmm3,XMMWORD ptr[ecx+esi+1]
	movdqu xmm4,XMMWORD ptr[ecx+esi+2]
	pavgb xmm1,xmm3
	pavgb xmm0,xmm4
	pavgb xmm0,xmm2
	pavgb xmm0,xmm2
	pavgb xmm0,xmm1
	movdqu XMMWORD ptr[ecx+edi],xmm0
	add	ecx,eax
	jnz	short JPSDR_H_BlurR2_8_SSE2_1
	
	pop edi
	pop esi

	ret
JPSDR_H_BlurR2_8_SSE2 endp


JPSDR_H_BlurR2_16_SSE2 proc psrc2:dword,ptmp2:dword,ia:dword

	public JPSDR_H_BlurR2_16_SSE2

	push esi
	push edi

	mov eax,16	
	mov esi,psrc2
	mov edi,ptmp2
	mov ecx,ia
	add esi,ecx
	add edi,ecx
	neg ecx
	
JPSDR_H_BlurR2_16_SSE2_1:
	movdqu xmm0,XMMWORD ptr[ecx+esi-4]
	movdqu xmm1,XMMWORD ptr[ecx+esi-2]
	movdqu xmm2,XMMWORD ptr[ecx+esi]
	movdqu xmm3,XMMWORD ptr[ecx+esi+2]
	movdqu xmm4,XMMWORD ptr[ecx+esi+4]
	pavgw xmm1,xmm3
	pavgw xmm0,xmm4
	pavgw xmm0,xmm2
	pavgw xmm0,xmm2
	pavgw xmm0,xmm1
	movdqu XMMWORD ptr[ecx+edi],xmm0
	add	ecx,eax
	jnz	short JPSDR_H_BlurR2_16_SSE2_1
	
	pop edi
	pop esi

	ret
JPSDR_H_BlurR2_16_SSE2 endp


JPSDR_H_BlurR2_16_AVX proc psrc2:dword,ptmp2:dword,ia:dword

	public JPSDR_H_BlurR2_16_AVX

	push esi
	push edi

	mov eax,16	
	mov esi,psrc2
	mov edi,ptmp2
	mov ecx,ia
	add esi,ecx
	add edi,ecx
	neg ecx
	
JPSDR_H_BlurR2_16_AVX_1:
	vmovdqu xmm0,XMMWORD ptr[ecx+esi-4]
	vmovdqu xmm1,XMMWORD ptr[ecx+esi-2]
	vmovdqu xmm2,XMMWORD ptr[ecx+esi]
	vmovdqu xmm3,XMMWORD ptr[ecx+esi+2]
	vmovdqu xmm4,XMMWORD ptr[ecx+esi+4]
	vpavgw xmm1,xmm1,xmm3
	vpavgw xmm0,xmm0,xmm4
	vpavgw xmm0,xmm0,xmm2
	vpavgw xmm0,xmm0,xmm2
	vpavgw xmm0,xmm0,xmm1
	vmovdqu XMMWORD ptr[ecx+edi],xmm0
	add	ecx,eax
	jnz	short JPSDR_H_BlurR2_16_AVX_1
	
	pop edi
	pop esi

	ret
JPSDR_H_BlurR2_16_AVX endp


JPSDR_H_BlurR2a_8_SSE2 proc psrc2:dword,ptmp2

	public JPSDR_H_BlurR2a_8_SSE2

	mov ecx,psrc2
	mov edx,ptmp2
	
	movdqu xmm0,XMMWORD ptr[ecx-2]
	movdqu xmm1,XMMWORD ptr[ecx-1]
	movdqu xmm2,XMMWORD ptr[ecx]
	movdqu xmm3,XMMWORD ptr[ecx+1]
	movdqu xmm4,XMMWORD ptr[ecx+2]
	pavgb xmm1,xmm3
	pavgb xmm0,xmm4
	pavgb xmm0,xmm2
	pavgb xmm0,xmm2
	pavgb xmm0,xmm1
	movdqu XMMWORD ptr[edx],xmm0

	ret
JPSDR_H_BlurR2a_8_SSE2 endp


JPSDR_H_BlurR2a_16_SSE2 proc psrc2:dword,ptmp2

	public JPSDR_H_BlurR2a_16_SSE2

	mov ecx,psrc2
	mov edx,ptmp2
	
	movdqu xmm0,XMMWORD ptr[ecx-4]
	movdqu xmm1,XMMWORD ptr[ecx-2]
	movdqu xmm2,XMMWORD ptr[ecx]
	movdqu xmm3,XMMWORD ptr[ecx+2]
	movdqu xmm4,XMMWORD ptr[ecx+4]
	pavgw xmm1,xmm3
	pavgw xmm0,xmm4
	pavgw xmm0,xmm2
	pavgw xmm0,xmm2
	pavgw xmm0,xmm1
	movdqu XMMWORD ptr[edx],xmm0

	ret
JPSDR_H_BlurR2a_16_SSE2 endp


JPSDR_H_BlurR2a_16_AVX proc psrc2:dword,ptmp2

	public JPSDR_H_BlurR2a_16_AVX

	mov ecx,psrc2
	mov edx,ptmp2
	
	vmovdqu xmm0,XMMWORD ptr[ecx-4]
	vmovdqu xmm1,XMMWORD ptr[ecx-2]
	vmovdqu xmm2,XMMWORD ptr[ecx]
	vmovdqu xmm3,XMMWORD ptr[ecx+2]
	vmovdqu xmm4,XMMWORD ptr[ecx+4]
	vpavgw xmm1,xmm1,xmm3
	vpavgw xmm0,xmm0,xmm4
	vpavgw xmm0,xmm0,xmm2
	vpavgw xmm0,xmm0,xmm2
	vpavgw xmm0,xmm0,xmm1
	vmovdqu XMMWORD ptr[edx],xmm0

	ret
JPSDR_H_BlurR2a_16_AVX endp

	
JPSDR_V_BlurR2_8_SSE2 proc psrc2:dword,ptmp2:dword,ia:dword,tmp_pitchp1:dword,tmp_pitchp2:dword,tmp_pitchn1:dword,tmp_pitchn2:dword,

	public JPSDR_V_BlurR2_8_SSE2	

	push esi
	push edi
	push ebx
	push ebp
	
	mov esi,ptmp2
	mov edi,psrc2
	mov ecx,ia
	mov eax,tmp_pitchp2
	mov edx,tmp_pitchn2
	mov ebx,tmp_pitchp1
	mov ebp,tmp_pitchn1
	sub edi,esi
	
JPSDR_V_BlurR2_8_SSE2_1:
	movdqa xmm0,XMMWORD ptr[esi+eax]
	pavgb xmm0,XMMWORD ptr[esi+edx]
	movdqa xmm1,XMMWORD ptr[esi+ebx]
	pavgb xmm1,XMMWORD ptr[esi+ebp]
	movdqa xmm2,XMMWORD ptr[esi]
	pavgb xmm0,xmm2
	pavgb xmm0,xmm2
	pavgb xmm0,xmm1
	movntdq XMMWORD ptr[esi+edi],xmm0
	add	esi,16
	sub	ecx,16
	jnz	short JPSDR_V_BlurR2_8_SSE2_1
		  
	pop ebp
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_V_BlurR2_8_SSE2 endp


JPSDR_V_BlurR2_8_AVX proc psrc2:dword,ptmp2:dword,ia:dword,tmp_pitchp1:dword,tmp_pitchp2:dword,tmp_pitchn1:dword,tmp_pitchn2:dword,

	public JPSDR_V_BlurR2_8_AVX	

	push esi
	push edi
	push ebx
	push ebp
	
	mov esi,ptmp2
	mov edi,psrc2
	mov ecx,ia
	mov eax,tmp_pitchp2
	mov edx,tmp_pitchn2
	mov ebx,tmp_pitchp1
	mov ebp,tmp_pitchn1
	sub edi,esi
	
JPSDR_V_BlurR2_8_AVX_1:
	vmovdqa xmm0,XMMWORD ptr[esi+eax]
	vpavgb xmm0,xmm0,XMMWORD ptr[esi+edx]
	vmovdqa xmm1,XMMWORD ptr[esi+ebx]
	vpavgb xmm1,xmm1,XMMWORD ptr[esi+ebp]
	vmovdqa xmm2,XMMWORD ptr[esi]
	vpavgb xmm0,xmm0,xmm2
	vpavgb xmm0,xmm0,xmm2
	vpavgb xmm0,xmm0,xmm1
	vmovntdq XMMWORD ptr[esi+edi],xmm0
	add	esi,16
	sub	ecx,16
	jnz	short JPSDR_V_BlurR2_8_AVX_1
		  
	pop ebp
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_V_BlurR2_8_AVX endp


JPSDR_V_BlurR2_16_SSE2 proc psrc2:dword,ptmp2:dword,ia:dword,tmp_pitchp1:dword,tmp_pitchp2:dword,tmp_pitchn1:dword,tmp_pitchn2:dword,

	public JPSDR_V_BlurR2_16_SSE2	

	push esi
	push edi
	push ebx
	push ebp
	
	mov esi,ptmp2
	mov edi,psrc2
	mov ecx,ia
	mov eax,tmp_pitchp2
	mov edx,tmp_pitchn2
	mov ebx,tmp_pitchp1
	mov ebp,tmp_pitchn1
	sub edi,esi
	
JPSDR_V_BlurR2_16_SSE2_1:
	movdqa xmm0,XMMWORD ptr[esi+eax]
	pavgw xmm0,XMMWORD ptr[esi+edx]
	movdqa xmm1,XMMWORD ptr[esi+ebx]
	pavgw xmm1,XMMWORD ptr[esi+ebp]
	movdqa xmm2,XMMWORD ptr[esi]
	pavgw xmm0,xmm2
	pavgw xmm0,xmm2
	pavgw xmm0,xmm1
	movntdq XMMWORD ptr[esi+edi],xmm0
	add	esi,16
	sub	ecx,16
	jnz	short JPSDR_V_BlurR2_16_SSE2_1
		  
	pop ebp
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_V_BlurR2_16_SSE2 endp


JPSDR_V_BlurR2_16_AVX proc psrc2:dword,ptmp2:dword,ia:dword,tmp_pitchp1:dword,tmp_pitchp2:dword,tmp_pitchn1:dword,tmp_pitchn2:dword,

	public JPSDR_V_BlurR2_16_AVX	

	push esi
	push edi
	push ebx
	push ebp
	
	mov esi,ptmp2
	mov edi,psrc2
	mov ecx,ia
	mov eax,tmp_pitchp2
	mov edx,tmp_pitchn2
	mov ebx,tmp_pitchp1
	mov ebp,tmp_pitchn1
	sub edi,esi
	
JPSDR_V_BlurR2_16_AVX_1:
	vmovdqa xmm0,XMMWORD ptr[esi+eax]
	vpavgw xmm0,xmm0,XMMWORD ptr[esi+edx]
	vmovdqa xmm1,XMMWORD ptr[esi+ebx]
	vpavgw xmm1,xmm1,XMMWORD ptr[esi+ebp]
	vmovdqa xmm2,XMMWORD ptr[esi]
	vpavgw xmm0,xmm0,xmm2
	vpavgw xmm0,xmm0,xmm2
	vpavgw xmm0,xmm0,xmm1
	vmovntdq XMMWORD ptr[esi+edi],xmm0
	add	esi,16
	sub	ecx,16
	jnz	short JPSDR_V_BlurR2_16_AVX_1
		  
	pop ebp
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_V_BlurR2_16_AVX endp


JPSDR_GuideChroma1_8_SSE2 proc py:dword,pu:dword,pitch_y:dword,width_uv_8:dword

	public JPSDR_GuideChroma1_8_SSE2

	push esi
	push edi
	push ebx

	mov ebx,16
	mov ecx,width_uv_8
	mov esi,py
	mov eax,pitch_y
	mov edx,pu
	sub esi,ecx
	sub esi,ecx
	add eax,esi
	sub edx,ecx
	sub edx,ebx
	pcmpeqw xmm7,xmm7
	psrlw xmm7,8
	
JPSDR_GuideChroma1_8_SSE2_1:
	movdqa xmm0,XMMWORD ptr[esi+2*ecx]
	movdqa xmm2,XMMWORD ptr[esi+2*ecx+16]
	movdqa xmm1,xmm0
	movdqa xmm3,xmm2
	pand xmm0,xmm7
	pand xmm2,xmm7
	packuswb xmm0,xmm2
	psrlw xmm1,8
	psrlw xmm3,8
	packuswb xmm1,xmm3
	pavgb xmm0,xmm1
	movdqa xmm1,XMMWORD ptr[eax+2*ecx]
	movdqa xmm3,XMMWORD ptr[eax+2*ecx+16]
	movdqa xmm2,xmm1
	movdqa xmm4,xmm3
	pand xmm1,xmm7
	pand xmm3,xmm7
	packuswb xmm1,xmm3
	psrlw xmm2,8
	psrlw xmm4,8
	packuswb xmm2,xmm4
	pavgb xmm1,xmm2
	pavgb xmm0,xmm1
	add ecx,ebx
	jg short JPSDR_GuideChroma1_8_SSE2_2
	movntdq XMMWORD ptr[ecx+edx],xmm0
	jnz short JPSDR_GuideChroma1_8_SSE2_1
	jmp short JPSDR_GuideChroma1_8_SSE2_3
	
JPSDR_GuideChroma1_8_SSE2_2:
	movq qword ptr[ecx+edx],xmm0
			
JPSDR_GuideChroma1_8_SSE2_3:
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_GuideChroma1_8_SSE2 endp


JPSDR_GuideChroma1_8_AVX proc py:dword,pu:dword,pitch_y:dword,width_uv_8:dword

	public JPSDR_GuideChroma1_8_AVX

	push esi
	push edi
	push ebx

	mov ebx,16
	mov ecx,width_uv_8
	mov esi,py
	mov eax,pitch_y
	mov edx,pu
	sub esi,ecx
	sub esi,ecx
	add eax,esi
	sub edx,ecx
	sub edx,ebx
	vpcmpeqw xmm7,xmm7,xmm7
	vpsrlw xmm7,xmm7,8
	
JPSDR_GuideChroma1_8_AVX_1:
	vmovdqa xmm0,XMMWORD ptr[esi+2*ecx]
	vmovdqa xmm2,XMMWORD ptr[esi+2*ecx+16]
	vpsrlw xmm1,xmm0,8
	vpsrlw xmm3,xmm2,8
	vpand xmm0,xmm0,xmm7
	vpand xmm2,xmm2,xmm7
	vpackuswb xmm0,xmm0,xmm2
	vpackuswb xmm1,xmm1,xmm3
	vpavgb xmm0,xmm0,xmm1
	vmovdqa xmm1,XMMWORD ptr[eax+2*ecx]
	vmovdqa xmm3,XMMWORD ptr[eax+2*ecx+16]
	vpsrlw xmm2,xmm1,8
	vpsrlw xmm4,xmm3,8
	vpand xmm1,xmm1,xmm7
	vpand xmm3,xmm3,xmm7
	vpackuswb xmm1,xmm1,xmm3
	vpackuswb xmm2,xmm2,xmm4
	vpavgb xmm1,xmm1,xmm2
	vpavgb xmm0,xmm0,xmm1
	add ecx,ebx
	jg short JPSDR_GuideChroma1_8_AVX_2
	vmovntdq XMMWORD ptr[ecx+edx],xmm0
	jnz short JPSDR_GuideChroma1_8_AVX_1
	jmp short JPSDR_GuideChroma1_8_AVX_3
	
JPSDR_GuideChroma1_8_AVX_2:
	vmovq qword ptr[ecx+edx],xmm0
			
JPSDR_GuideChroma1_8_AVX_3:
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_GuideChroma1_8_AVX endp


JPSDR_GuideChroma1_16_AVX proc py:dword,pu:dword,pitch_y:dword,width_uv_8:dword

	public JPSDR_GuideChroma1_16_AVX

	push esi
	push edi
	push ebx

	mov ebx,16
	mov ecx,width_uv_8
	mov esi,py
	mov eax,pitch_y
	mov edx,pu
	sub esi,ecx
	sub esi,ecx
	add eax,esi
	sub edx,ecx
	sub edx,ebx
	vpcmpeqd xmm7,xmm7,xmm7
	vpsrld xmm7,xmm7,16
	
JPSDR_GuideChroma1_16_AVX_1:
	vmovdqa xmm0,XMMWORD ptr[esi+2*ecx]
	vmovdqa xmm2,XMMWORD ptr[esi+2*ecx+16]
	vpsrld xmm1,xmm0,16
	vpsrld xmm3,xmm2,16
	vpand xmm0,xmm0,xmm7
	vpand xmm2,xmm2,xmm7
	vpackusdw xmm0,xmm0,xmm2
	vpackusdw xmm1,xmm1,xmm3
	vpavgw xmm0,xmm0,xmm1
	vmovdqa xmm1,XMMWORD ptr[eax+2*ecx]
	vmovdqa xmm3,XMMWORD ptr[eax+2*ecx+16]
	vpsrld xmm2,xmm1,16
	vpsrld xmm4,xmm3,16
	vpand xmm1,xmm1,xmm7
	vpand xmm3,xmm3,xmm7
	vpackusdw xmm1,xmm1,xmm3
	vpackusdw xmm2,xmm2,xmm4
	vpavgw xmm1,xmm1,xmm2
	vpavgw xmm0,xmm0,xmm1
	add ecx,ebx
	jg short JPSDR_GuideChroma1_16_AVX_2
	vmovntdq XMMWORD ptr[ecx+edx],xmm0
	jnz short JPSDR_GuideChroma1_16_AVX_1
	jmp short JPSDR_GuideChroma1_16_AVX_3
	
JPSDR_GuideChroma1_16_AVX_2:
	vmovq qword ptr[ecx+edx],xmm0
			
JPSDR_GuideChroma1_16_AVX_3:
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_GuideChroma1_16_AVX endp


JPSDR_GuideChroma2_8_SSE2 proc py:dword,pu:dword,width_uv_8:dword

	public JPSDR_GuideChroma2_8_SSE2

	push esi
	push edi
	
	mov eax,16
	mov ecx,width_uv_8
	mov esi,py
	mov edx,pu
	sub esi,ecx
	sub esi,ecx
	sub edx,ecx
	sub edx,eax
	pcmpeqw xmm7,xmm7
	psrlw xmm7,8
	
JPSDR_GuideChroma2_8_SSE2_1:
	movdqa xmm0,XMMWORD ptr[esi+2*ecx]
	movdqa xmm2,XMMWORD ptr[esi+2*ecx+16]
	movdqa xmm1,xmm0
	movdqa xmm3,xmm2
	pand xmm0,xmm7
	pand xmm2,xmm7
	packuswb xmm0,xmm2
	psrlw xmm1,8
	psrlw xmm3,8
	packuswb xmm1,xmm3
	pavgb xmm0,xmm1
	add ecx,eax
	jg short JPSDR_GuideChroma2_8_SSE2_2
	movntdq XMMWORD ptr[ecx+edx],xmm0
	jnz short JPSDR_GuideChroma2_8_SSE2_1
	jmp short JPSDR_GuideChroma2_8_SSE2_3
	
JPSDR_GuideChroma2_8_SSE2_2:
	movq qword ptr[ecx+edx],xmm0
	
JPSDR_GuideChroma2_8_SSE2_3:	
	pop edi
	pop esi

	ret
JPSDR_GuideChroma2_8_SSE2 endp


JPSDR_GuideChroma2_8_AVX proc py:dword,pu:dword,width_uv_8:dword

	public JPSDR_GuideChroma2_8_AVX

	push esi
	push edi
	
	mov eax,16
	mov ecx,width_uv_8
	mov esi,py
	mov edx,pu
	sub esi,ecx
	sub esi,ecx
	sub edx,ecx
	sub edx,eax
	vpcmpeqw xmm7,xmm7,xmm7
	vpsrlw xmm7,xmm7,8
	
JPSDR_GuideChroma2_8_AVX_1:
	vmovdqa xmm0,XMMWORD ptr[esi+2*ecx]
	vmovdqa xmm2,XMMWORD ptr[esi+2*ecx+16]
	vpsrlw xmm1,xmm0,8
	vpsrlw xmm3,xmm2,8
	vpand xmm0,xmm0,xmm7
	vpand xmm2,xmm2,xmm7
	vpackuswb xmm0,xmm0,xmm2
	vpackuswb xmm1,xmm1,xmm3
	vpavgb xmm0,xmm0,xmm1
	add ecx,eax
	jg short JPSDR_GuideChroma2_8_AVX_2
	vmovntdq XMMWORD ptr[ecx+edx],xmm0
	jnz short JPSDR_GuideChroma2_8_AVX_1
	jmp short JPSDR_GuideChroma2_8_AVX_3
	
JPSDR_GuideChroma2_8_AVX_2:
	vmovq qword ptr[ecx+edx],xmm0
	
JPSDR_GuideChroma2_8_AVX_3:	
	pop edi
	pop esi

	ret
JPSDR_GuideChroma2_8_AVX endp


JPSDR_GuideChroma2_16_AVX proc py:dword,pu:dword,width_uv_8:dword

	public JPSDR_GuideChroma2_16_AVX

	push esi
	push edi
	
	mov eax,16
	mov ecx,width_uv_8
	mov esi,py
	mov edx,pu
	sub esi,ecx
	sub esi,ecx
	sub edx,ecx
	sub edx,eax
	vpcmpeqd xmm7,xmm7,xmm7
	vpsrld xmm7,xmm7,16
	
JPSDR_GuideChroma2_16_AVX_1:
	vmovdqa xmm0,XMMWORD ptr[esi+2*ecx]
	vmovdqa xmm2,XMMWORD ptr[esi+2*ecx+16]
	vpsrld xmm1,xmm0,16
	vpsrld xmm3,xmm2,16
	vpand xmm0,xmm0,xmm7
	vpand xmm2,xmm2,xmm7
	vpackusdw xmm0,xmm0,xmm2
	vpackusdw xmm1,xmm1,xmm3
	vpavgw xmm0,xmm0,xmm1
	add ecx,eax
	jg short JPSDR_GuideChroma2_16_AVX_2
	vmovntdq XMMWORD ptr[ecx+edx],xmm0
	jnz short JPSDR_GuideChroma2_16_AVX_1
	jmp short JPSDR_GuideChroma2_16_AVX_3
	
JPSDR_GuideChroma2_16_AVX_2:
	vmovq qword ptr[ecx+edx],xmm0
	
JPSDR_GuideChroma2_16_AVX_3:	
	pop edi
	pop esi

	ret
JPSDR_GuideChroma2_16_AVX endp
	
end





