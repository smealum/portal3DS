; make sure you update aemstro_as for this (01/01/15)
 
; setup constants
	.const c82, 1.0, 1.0, 1.0, 1.0
	.const c83, 0.0, 0.0, 0.0, 0.0
 
; setup outmap
	.out o0, result.position, 0xF
	.out o1, result.color, 0xF
	.out o2, result.texcoord0, 0x3
	.out o3, result.texcoord1, 0x3

; setup uniform map (required to use SHDR_GetUniformRegister)
	.uniform c80, c80, color      ; c80 = color
	.uniform c84, c87, projection      ; c84-c87 = projection matrix
	.uniform c88, c91, modelview       ; c88-c91 = modelview matrix

; setup vsh and gsh
	.vsh main, endmain
 
; code
	main:
		; tempreg = mdlvMtx * in.pos
			dp4 r0,  c88,  v0  (0x0)
			dp4 r0,  c89,  v0  (0x1)
			dp4 r0,  c90,  v0  (0x2)
			mov r0,  c82       (0x3)

		; result.pos = projMtx * tempreg
			dp4 o0,  c84,  r0  (0x0)
			dp4 o0,  c85,  r0  (0x1)
			dp4 o0,  c87,  r0  (0x3)

		ifu end_if, else, b0
			dp4 o0,  c86,  r0  (0x2)
		else:
			mov o0,  c83       (0x2)
		end_if:

		; result.texcoord = in.texcoord
			mov o2,  c82       (0x5)
			mov o3,  c82       (0x5)

		; result.color = EXPERIMENTS !			
			mov o1,  c80       (0x5)
			
		end
		nop
	endmain:

; operand descriptors
	.opdesc x___, xyzw, xyzw ; 0x0
	.opdesc _y__, xyzw, xyzw ; 0x1
	.opdesc __z_, xyzw, xyzw ; 0x2
	.opdesc ___w, xyzw, xyzw ; 0x3
	.opdesc xyzw, zzzy, xyzw ; 0x4
	.opdesc xyzw, xyzw, xyzw ; 0x5
	.opdesc xyz_, xyzw, xyzw ; 0x6
	.opdesc xyzw, yyyw, xyzw ; 0x7
	.opdesc x___, wwww, xyzw ; 0x8
	.opdesc xyzw, xyzw, xxxx, xyzw ; 0x9
	.opdesc xyzw, xyzw, -xyzw ; 0xa
	.opdesc xyzw, xxxx, wwww ; 0xb
	.opdesc xyzw, -xxxx, wwww ; 0xc
	.opdesc xy__, yzzz, xyzw ; 0xd
