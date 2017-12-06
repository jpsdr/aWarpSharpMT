.586
.model flat,c

;.data

;align 16

;xmm8_ word 8 dup(0)

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
    psrldq xmm2,4
    pinsrw xmm3,word ptr[eax+esi],0
    pinsrw xmm1,word ptr[eax+edx],0
    movd eax,xmm2
    psrldq xmm2,4
    pinsrw xmm3,word ptr[eax+esi+1],1
    pinsrw xmm1,word ptr[eax+edx+1],1
    movd eax,xmm2
    psrldq xmm2,4
    pinsrw xmm3,word ptr[eax+esi+2],2
    pinsrw xmm1,word ptr[eax+edx+2],2
    movd eax,xmm2
    pinsrw xmm3,word ptr[eax+esi+3],3
    pinsrw xmm1,word ptr[eax+edx+3],3
    movd eax,xmm7
    psrldq xmm7,4
    pinsrw xmm3,word ptr[eax+esi+4],4
    pinsrw xmm1,word ptr[eax+edx+4],4
    movd eax,xmm7
    psrldq xmm7,4
    pinsrw xmm3,word ptr[eax+esi+5],5
    pinsrw xmm1,word ptr[eax+edx+5],5
    movd eax,xmm7
    psrldq xmm7,4
    pinsrw xmm3,word ptr[eax+esi+6],6
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
	
;	movdqa xmm6,xmm9_	;preload depth
	
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


JPSDR_Warp0_8_SSE3 proc psrc:dword,pedg:dword,pdst:dword,src_pitch:dword,edg_pitchp:dword,edg_pitchn:dword,
	y_limit_min:dword,y_limit_max:dword,x_limit_min:dword,x_limit_max:dword,i_:dword,depthH:dword,depthV:dword

	public JPSDR_Warp0_8_SSE3
	
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
	
JPSDR_Warp0_8_SSE3_1:
    movq xmm4,qword ptr[edi+ebx]
    movq xmm2,qword ptr[edi+ebp]
    pxor xmm0,xmm0
    punpcklbw xmm7,xmm0
    punpcklbw xmm1,xmm0
    punpcklbw xmm4,xmm0
    punpcklbw xmm2,xmm0
    psubw xmm7,xmm1
    psubw xmm4,xmm2
    psllw xmm7,7
    psllw xmm4,7
    pmulhw xmm7,xmm9_ ;depthH
    pmulhw xmm4,xmm14_ ;depthV
    movd xmm6,edi ;preload

    pmaxsw xmm4,xmm10_	;y_limit_min
    pminsw xmm4,xmm11_	;y_limit_max
	
	pshufd xmm6,xmm6,0
	
	pcmpeqw xmm0,xmm0
    psrlw xmm0,9
    movdqa xmm2,xmm4
    movdqa xmm1,xmm7
	
    pand xmm4,xmm0 ;007F
    pand xmm7,xmm0 ;007F

    psraw xmm1,7
    packssdw xmm6,xmm6
    paddsw xmm1,xmm6
	
    movdqa xmm6,xmm8_	;preload 1 src_pitch

    movdqa xmm0,xmm13_	;x_limit_max
    movdqa xmm3,xmm12_	;x_limit_min

    pcmpgtw xmm0,xmm1
    pcmpgtw xmm3,xmm1
	
    pminsw xmm1,xmm13_	;x_limit_max
    pmaxsw xmm1,xmm12_	;x_limit_min

    pand xmm7,xmm0
    pandn xmm3,xmm7

    psraw xmm2,7
    movdqa xmm7,xmm2
    punpcklwd xmm2,xmm1
    punpckhwd xmm7,xmm1
    pmaddwd xmm2,xmm6 ;1 src_pitch
    pmaddwd xmm7,xmm6 ;1 src_pitch
	
	;movdqa xmm6,xmm9_	;preload depth
	
    psignw xmm3,xmm5 ;8000
    psignw xmm4,xmm5 ;8000
    packsswb xmm5,xmm5
    packsswb xmm3,xmm3
    packsswb xmm4,xmm4

    movdqa xmm0,xmm5 ;80
    movdqa xmm1,xmm5 ;80
    psubb xmm0,xmm3
    psubb xmm1,xmm4
    psrlw xmm5,9
    punpcklbw xmm0,xmm3
    punpcklbw xmm1,xmm4

    movd eax,xmm2
	psrldq xmm2,4
    pinsrw xmm3,word ptr[eax+esi],0
    pinsrw xmm4,word ptr[eax+edx],0
    movd eax,xmm2
	psrldq xmm2,4
    pinsrw xmm3,word ptr[eax+esi+1],1
    pinsrw xmm4,word ptr[eax+edx+1],1
    movd eax,xmm2
	psrldq xmm2,4
    pinsrw xmm3,word ptr[eax+esi+2],2
    pinsrw xmm4,word ptr[eax+edx+2],2
    movd eax,xmm2
    pinsrw xmm3,word ptr[eax+esi+3],3
    pinsrw xmm4,word ptr[eax+edx+3],3
    movd eax,xmm7
	psrldq xmm7,4
    pinsrw xmm3,word ptr[eax+esi+4],4
    pinsrw xmm4,word ptr[eax+edx+4],4
    movd eax,xmm7
	psrldq xmm7,4
    pinsrw xmm3,word ptr[eax+esi+5],5
    pinsrw xmm4,word ptr[eax+edx+5],5
    movd eax,xmm7
	psrldq xmm7,4
    pinsrw xmm3,word ptr[eax+esi+6],6
    pinsrw xmm4,word ptr[eax+edx+6],6
    movd eax,xmm7
    pinsrw xmm3,word ptr[eax+esi+7],7
    pinsrw xmm4,word ptr[eax+edx+7],7

    pcmpeqw xmm2,xmm2
    movdqu xmm7,XMMWORD ptr[edi+ecx+7]
    pmaddubsw xmm3,xmm0
    pmaddubsw xmm4,xmm0
	
	mov	eax,dword ptr[esp+4]
	
    psignw xmm3,xmm2
    psignw xmm4,xmm2
    paddw xmm3,xmm5 ; 0040
    paddw xmm4,xmm5 ; 0040
    psraw xmm3,7
    psraw xmm4,7
    packuswb xmm3,xmm3
    packuswb xmm4,xmm4
    punpcklbw xmm3,xmm4
    pmaddubsw xmm3,xmm1
    palignr xmm1,xmm7,2
    psignw xmm3,xmm2

    paddw xmm3,xmm5 ; 0040
    psraw xmm3,7
    psllw xmm5,9
    packuswb xmm3,xmm3
		  
    add edi,8	
    jg short JPSDR_Warp0_8_SSE3_2
    movq qword ptr[edi+eax],xmm3
    jnz JPSDR_Warp0_8_SSE3_1
    jmp short JPSDR_Warp0_8_SSE3_Fin
	
JPSDR_Warp0_8_SSE3_2:
	movd dword ptr[edi+eax],xmm3
	
JPSDR_Warp0_8_SSE3_Fin:
	pop esp
	pop ecx

	pop ebp
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_Warp0_8_SSE3 endp


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
	
    psllw xmm4,2 ;SMAGL=2
    psllw xmm7,2 ;SMAGL=2
	
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
    psrldq xmm2,4
    pinsrw xmm3,word ptr[eax+esi],0
    pinsrw xmm1,word ptr[eax+edx],0
    movd eax,xmm2
    psrldq xmm2,4
    pinsrw xmm3,word ptr[eax+esi+1*4],1
    pinsrw xmm1,word ptr[eax+edx+1*4],1
    movd eax,xmm2
    psrldq xmm2,4
    pinsrw xmm3,word ptr[eax+esi+2*4],2
    pinsrw xmm1,word ptr[eax+edx+2*4],2
    movd eax,xmm2
    pinsrw xmm3,word ptr[eax+esi+3*4],3
    pinsrw xmm1,word ptr[eax+edx+3*4],3
    movd eax,xmm7
    psrldq xmm7,4
    pinsrw xmm3,word ptr[eax+esi+4*4],4
    pinsrw xmm1,word ptr[eax+edx+4*4],4
    movd eax,xmm7
    psrldq xmm7,4
    pinsrw xmm3,word ptr[eax+esi+5*4],5
    pinsrw xmm1,word ptr[eax+edx+5*4],5
    movd eax,xmm7
    psrldq xmm7,4
    pinsrw xmm3,word ptr[eax+esi+6*4],6
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
	
	;movdqa xmm6,xmm9_	;preload depth
	
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


JPSDR_Warp2_8_SSE3 proc psrc:dword,pedg:dword,pdst:dword,src_pitch:dword,edg_pitchp:dword,edg_pitchn:dword,
	y_limit_min:dword,y_limit_max:dword,x_limit_min:dword,x_limit_max:dword,i_:dword,depthH:dword,depthV:dword

	public JPSDR_Warp2_8_SSE3
	
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
	
JPSDR_Warp2_8_SSE3_1:
    movq xmm4,qword ptr[edi+ebx]
    movq xmm2,qword ptr[edi+ebp]
    pxor xmm0,xmm0
    punpcklbw xmm7,xmm0
    punpcklbw xmm1,xmm0
    punpcklbw xmm4,xmm0
    punpcklbw xmm2,xmm0
    psubw xmm7,xmm1
    psubw xmm4,xmm2
    psllw xmm7,7
    psllw xmm4,7
    pmulhw xmm7,xmm9_ ;depthH
    pmulhw xmm4,xmm14_ ;depthV
    movd xmm6,edi ;preload

    pmaxsw xmm4,xmm10_	;y_limit_min
    pminsw xmm4,xmm11_	;y_limit_max
	
	pshufd xmm6,xmm6,0
	
	pslld xmm6,2	;SMAGL=2
	
	pcmpeqw xmm0,xmm0
    psrlw xmm0,9
    movdqa xmm2,xmm4
    movdqa xmm1,xmm7
	
    psllw xmm4,2	;SMAGL=2
    psllw xmm7,2	;SMAGL=2
	
    pand xmm4, xmm0 ;007F
    pand xmm7, xmm0 ;007F

    psraw xmm1,5	;7-SMAGL
    packssdw xmm6,xmm6
    paddsw xmm1,xmm6
	
    movdqa xmm6,xmm8_	;preload 1 src_pitch

    movdqa xmm0,xmm13_	;x_limit_max
    movdqa xmm3,xmm12_	;x_limit_min

    pcmpgtw xmm0,xmm1
    pcmpgtw xmm3,xmm1
	
    pminsw xmm1,xmm13_	;x_limit_max
    pmaxsw xmm1,xmm12_	;x_limit_min

    pand xmm7,xmm0
    pandn xmm3,xmm7

    psraw xmm2,5	;7-SMAGL
    movdqa xmm7,xmm2
    punpcklwd xmm2,xmm1
    punpckhwd xmm7,xmm1
    pmaddwd xmm2,xmm6 ;1 src_pitch
    pmaddwd xmm7,xmm6 ;1 src_pitch
	
	;movdqa xmm6,xmm9_	;preload depth
	
    psignw xmm3,xmm5 ;8000
    psignw xmm4,xmm5 ;8000
    packsswb xmm5,xmm5
    packsswb xmm3,xmm3
    packsswb xmm4,xmm4

    movdqa xmm0,xmm5 ;80
    movdqa xmm1,xmm5 ;80
    psubb xmm0,xmm3
    psubb xmm1,xmm4
    psrlw xmm5,9
    punpcklbw xmm0,xmm3
    punpcklbw xmm1,xmm4

    movd eax,xmm2
	psrldq xmm2,4
    pinsrw xmm3,word ptr[eax+esi],0
    pinsrw xmm4,word ptr[eax+edx],0
    movd eax,xmm2
	psrldq xmm2,4
    pinsrw xmm3,word ptr[eax+esi+1*4],1
    pinsrw xmm4,word ptr[eax+edx+1*4],1
    movd eax,xmm2
	psrldq xmm2,4
    pinsrw xmm3,word ptr[eax+esi+2*4],2
    pinsrw xmm4,word ptr[eax+edx+2*4],2
    movd eax,xmm2
    pinsrw xmm3,word ptr[eax+esi+3*4],3
    pinsrw xmm4,word ptr[eax+edx+3*4],3
    movd eax,xmm7
	psrldq xmm7,4
    pinsrw xmm3,word ptr[eax+esi+4*4],4
    pinsrw xmm4,word ptr[eax+edx+4*4],4
    movd eax,xmm7
	psrldq xmm7,4
    pinsrw xmm3,word ptr[eax+esi+5*4],5
    pinsrw xmm4,word ptr[eax+edx+5*4],5
    movd eax,xmm7
	psrldq xmm7,4
    pinsrw xmm3,word ptr[eax+esi+6*4],6
    pinsrw xmm4,word ptr[eax+edx+6*4],6
    movd eax,xmm7
    pinsrw xmm3,word ptr[eax+esi+7*4],7
    pinsrw xmm4,word ptr[eax+edx+7*4],7

    pcmpeqw xmm2,xmm2
    movdqu xmm7,XMMWORD ptr[edi+ecx+7]
    pmaddubsw xmm3,xmm0
    pmaddubsw xmm4,xmm0
	
	mov	eax,dword ptr[esp+4]
	
    psignw xmm3,xmm2
    psignw xmm4,xmm2
    paddw xmm3,xmm5 ; 0040
    paddw xmm4,xmm5 ; 0040
    psraw xmm3,7
    psraw xmm4,7
    packuswb xmm3,xmm3
    packuswb xmm4,xmm4
    punpcklbw xmm3,xmm4
    pmaddubsw xmm3,xmm1
    palignr xmm1,xmm7,2
    psignw xmm3,xmm2

    paddw xmm3,xmm5 ; 0040
    psraw xmm3,7
    psllw xmm5,9
    packuswb xmm3,xmm3
		  
    add edi,8	
    jg short JPSDR_Warp2_8_SSE3_2
    movq qword ptr[edi+eax],xmm3
    jnz JPSDR_Warp2_8_SSE3_1
    jmp short JPSDR_Warp2_8_SSE3_Fin
	
JPSDR_Warp2_8_SSE3_2:
	movd dword ptr[edi+eax],xmm3
	
JPSDR_Warp2_8_SSE3_Fin:
	pop esp
	pop ecx

	pop ebp
	pop ebx
	pop edi
	pop esi

	ret
JPSDR_Warp2_8_SSE3 endp


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
    jnz JPSDR_Sobel_8_SSE2_1
    jmp short JPSDR_Sobel_8_SSE2_Fin

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


JPSDR_H_BlurR6_8_SSE3 proc psrc2:dword,ptmp2:dword,src_row_size:dword,dq0toF:dword

	public JPSDR_H_BlurR6_8_SSE3
	
	push esi
	push edi

	mov eax,16
    mov esi,psrc2
    mov edi,ptmp2
    mov ecx,src_row_size
    add esi,eax
    sub edi,esi
    movdqa xmm6,XMMWORD ptr[esi-16]
    movdqa xmm5,xmm6
    movdqa xmm7,xmm6
    pxor xmm0,xmm0
    pshufb xmm5,xmm0
    sub ecx,eax
    jna JPSDR_H_BlurR6_8_SSE3_2
    
JPSDR_H_BlurR6_8_SSE3_1:
    movdqa xmm7,XMMWORD ptr[esi]
    movdqa xmm0,xmm6
    movdqa xmm2,xmm7
    palignr xmm0,xmm5,10
    palignr xmm2,xmm6,6
    pavgb xmm0,xmm2
    movdqa xmm3,xmm6
    movdqa xmm4,xmm7
    palignr xmm3,xmm5,11
    palignr xmm4,xmm6,5
    pavgb xmm3,xmm4
    pavgb xmm0,xmm3
    movdqa xmm1,xmm6
    movdqa xmm2,xmm7
    palignr xmm1,xmm5,12
    palignr xmm2,xmm6,4
    pavgb xmm1,xmm2
    movdqa xmm3,xmm6
    movdqa xmm4,xmm7
    palignr xmm3,xmm5,13
    palignr xmm4,xmm6,3
    pavgb xmm3,xmm4
    pavgb xmm1,xmm3
    pavgb xmm0,xmm1
    movdqa xmm1,xmm6
    movdqa xmm2,xmm7
    palignr xmm1,xmm5,14
    palignr xmm2,xmm6,2
    pavgb xmm1,xmm2
    movdqa xmm3,xmm6
    movdqa xmm4,xmm7
    palignr xmm3,xmm5,15
    palignr xmm4,xmm6,1
    pavgb xmm3,xmm4
    pavgb xmm1,xmm3
    pavgb xmm1,xmm6
    movdqa xmm5,xmm6
    movdqa xmm6,xmm7
    pavgb xmm0,xmm1
    pavgb xmm0,xmm1
    movntdq XMMWORD ptr[esi+edi],xmm0
    add esi,eax
    sub ecx,eax
    ja JPSDR_H_BlurR6_8_SSE3_1
	
JPSDR_H_BlurR6_8_SSE3_2:
    add	ecx,15
	mov edx,dq0toF
    pxor xmm0,xmm0
    movd xmm1,ecx
    pshufb xmm1,xmm0
    pminub xmm1,XMMWORD ptr[edx]	;0x0F0E..00
    pshufb xmm6,xmm1
    psrldq xmm7,15
    pshufb xmm7,xmm0
    movdqa xmm0,xmm6
    movdqa xmm2,xmm7
    palignr xmm0,xmm5,10
    palignr xmm2,xmm6,6
    pavgb xmm0,xmm2
    movdqa xmm3,xmm6
    movdqa xmm4,xmm7
    palignr xmm3,xmm5,11
    palignr xmm4,xmm6,5
    pavgb xmm3,xmm4
    pavgb xmm0,xmm3
    movdqa xmm1,xmm6
    movdqa xmm2,xmm7
    palignr xmm1,xmm5,12
    palignr xmm2,xmm6,4
    pavgb xmm1,xmm2
    movdqa xmm3,xmm6
    movdqa xmm4,xmm7
    palignr xmm3,xmm5,13
    palignr xmm4,xmm6,3
    pavgb xmm3,xmm4
    pavgb xmm1,xmm3
    pavgb xmm0,xmm1
    movdqa xmm1,xmm6
    movdqa xmm2,xmm7
    palignr xmm1,xmm5,14
    palignr xmm2,xmm6,2
    pavgb xmm1,xmm2
    movdqa xmm3,xmm6
    movdqa xmm4,xmm7
    palignr xmm3,xmm5,15
    palignr xmm4,xmm6,1
    pavgb xmm3,xmm4
    pavgb xmm1,xmm3
    pavgb xmm1,xmm6
    movdqa xmm5,xmm6
    movdqa xmm6,xmm7
    pavgb xmm0,xmm1
    pavgb xmm0,xmm1
    movntdq XMMWORD ptr[esi+edi],xmm0
	
	pop edi
	pop esi

	ret
JPSDR_H_BlurR6_8_SSE3 endp


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
    movdqu xmm0,XMMWORD ptr[esi+6]
    pavgb xmm6,xmm0
    movdqu xmm5,XMMWORD ptr[esi-5]
    movdqu xmm7,XMMWORD ptr[esi+5]
    pavgb xmm5,xmm7
    movdqu xmm4,XMMWORD ptr[esi-4]
    movdqu xmm0,XMMWORD ptr[esi+4]
    pavgb xmm4,xmm0
    movdqu xmm3,XMMWORD ptr[esi-3]
    movdqu xmm7,XMMWORD ptr[esi+3]
    pavgb xmm3,xmm7
    movdqu xmm2,XMMWORD ptr[esi-2]
    movdqu xmm0,XMMWORD ptr[esi+2]
    pavgb xmm2,xmm0
    movdqu xmm1,XMMWORD ptr[esi-1]
    movdqu xmm7,XMMWORD ptr[esi+1]
    pavgb xmm1,xmm7
    movdqa xmm0,XMMWORD ptr[esi]
    pavgb xmm6,xmm5
    pavgb xmm4,xmm3
    pavgb xmm2,xmm1
    pavgb xmm6,xmm4
    pavgb xmm2,xmm0
    pavgb xmm6,xmm2
    pavgb xmm6,xmm2
    movntdq XMMWORD ptr[esi+edi],xmm6
    add esi,eax
	loop JPSDR_H_BlurR6_8_SSE2_1
	
	pop edi
	pop esi

	ret
JPSDR_H_BlurR6_8_SSE2 endp


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
	
	
JPSDR_H_BlurR2_8_SSE3 proc psrc2:dword,ptmp2:dword,i_:dword,dq0toF:dword

	public JPSDR_H_BlurR2_8_SSE3
	
	push esi
	push edi

	mov esi,dq0toF
	movdqa xmm4,XMMWORD ptr[esi]
	mov eax,16
	mov esi,psrc2
	mov edi,ptmp2
	mov ecx,i_
	add esi,eax
	sub edi,esi
	movdqa xmm6,XMMWORD ptr[esi-16]
	movdqa xmm5,xmm6
	movdqa xmm7,xmm6
	pxor xmm0,xmm0
	pshufb xmm5,xmm0
	sub ecx,eax
	jna short JPSDR_H_BlurR2_8_SSE3_2
	
JPSDR_H_BlurR2_8_SSE3_1:
	movdqa xmm7,XMMWORD ptr[esi]
	movdqa xmm0,xmm6
	movdqa xmm2,xmm7
	palignr xmm0,xmm5,14
	palignr xmm2,xmm6,2
	pavgb xmm0,xmm2
	movdqa xmm1,xmm6
	movdqa xmm3,xmm7
	palignr xmm1,xmm5,15
	palignr xmm3,xmm6,1
	pavgb xmm0,xmm6
	pavgb xmm1,xmm3
	pavgb xmm0,xmm6
	movdqa xmm5,xmm6
	movdqa xmm6,xmm7
	pavgb xmm0,xmm1
	movntdq XMMWORD ptr[esi+edi],xmm0
	add esi,eax
	sub ecx,eax
	ja short JPSDR_H_BlurR2_8_SSE3_1
	
JPSDR_H_BlurR2_8_SSE3_2:
	add ecx,15
	pxor xmm0,xmm0
	movd xmm1,ecx
	pshufb xmm1,xmm0
	pminub xmm1,xmm4 ;0x0F0E..00
	pshufb xmm6,xmm1
	psrldq xmm7,15
	pshufb xmm7,xmm0
	movdqa xmm0,xmm6
	movdqa xmm2,xmm7
	palignr xmm0,xmm5,14
	palignr xmm2,xmm6,2
	pavgb xmm0,xmm2
	movdqa xmm1,xmm6
	movdqa xmm3,xmm7
	palignr xmm1,xmm5,15
	palignr xmm3,xmm6,1
	pavgb xmm0,xmm6
	pavgb xmm1,xmm3
	pavgb xmm0,xmm6
	movdqa xmm5,xmm6
	movdqa xmm6,xmm7
	pavgb xmm0,xmm1
	movntdq XMMWORD ptr[esi+edi],xmm0
	
	pop edi
	pop esi

	ret
JPSDR_H_BlurR2_8_SSE3 endp


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
	movdqu xmm4,XMMWORD ptr[ecx+esi+2]
	pavgb xmm0,xmm4
	movdqu xmm1,XMMWORD ptr[ecx+esi-1]
	movdqu xmm3,XMMWORD ptr[ecx+esi+1]
	pavgb xmm1,xmm3
	movdqa xmm2,XMMWORD ptr[ecx+esi]
	pavgb xmm0,xmm2
	pavgb xmm0,xmm2
	pavgb xmm0,xmm1
	movntdq XMMWORD ptr[ecx+edi],xmm0
	add	ecx,eax
	jnz	short JPSDR_H_BlurR2_8_SSE2_1
	
	pop edi
	pop esi

	ret
JPSDR_H_BlurR2_8_SSE2 endp
	

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

	
end





