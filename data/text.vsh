; make sure you update aemstro_as for this (01/01/15)
 
; setup constants
	.const c20, 1.0, 0.0, 0.5, 1.0
	
	.const c21, 0.0, 0.0, -0.5, 1.0
	.const c22, -0.1, 0.0, -0.5, 1.0
	.const c23, -0.1, -0.05, -0.5, 1.0
	.const c24, 0.0,  -0.05, -0.5, 1.0

	.const c25, 31.0, 32.0, 0.0, 10.0
	.const c26, 0.00, 0.00, 0.0, 1.0

	.const c30, 0.0078125, 0.0068359375, 1.0, 0.0
	.const c31, 1.0, 1.0, 1.0, 1.0

; setup outmap
	.out o0, result.position, 0xF
	.out o1, result.color, 0xF
	.out o2, result.texcoord0, 0x3
	.out o3, result.texcoord1, 0x3

; setup uniform map (required to use SHDR_GetUniformRegister)
	.uniform c0, c3, projection      ; c0-c3 = projection matrix
	.uniform c4, c7, modelview       ; c4-c7 = modelview matrix
	.uniform c8, c8, lightDirection  ; c8    = light direction vector
	.uniform c9, c9, lightAmbient    ; c9    = light ambient color

; setup vsh and gsh
	.vsh main, endmain
	.gsh main_gsh, endmain_gsh
 
; code
	main:
		;mov r0, c20 (0x5)
		mov r0, v0 (0x5)

		cmp c25, GT, GT, v0 (0x5)

		ifc end_if, else, cmp.x
			mov o0,  v0 (0x5)
			mov o1, c25 (0x5)
			mov o2, c25 (0x5)
			mov o3, c25 (0x5)
			nop
		else:
			add r0, c25, r0 (0xb)
			
			mul r0, c30, r0 (0x1)
			mov r0, c30     (0x0)
			
			mov o0, c21 (0x8)
			mov o0, r0  (0xa)
			
			mov o1, c22 (0x8)
			add o1, c30, r0 (0x4)
			
			mov o2, c23 (0x8)
			add o2, c30, r0 (0x6)
			
			mov o3, c24 (0x8)
			add o3, c30, r0 (0x7)

			nop
		end_if:
		nop
		
		end
		nop
	endmain:

	main_gsh:

		cmp c25, EQ, EQ, v2 (0xc)

		ifc end_if_2, else_2, cmp.x
			; special character !
			cmp c25, EQ, EQ, v0 (0xd)
			ifc end_if_3, else_3, cmp.x
				; 10 is newline '\n'
				mov r15, c26 (0xe)
				add r15, c22, r15 (0x8)
				nop
			else_3:
				; we process everything else as carriage return
				mov r15, c26 (0x5)
			end_if_3:
			nop
		else_2:
			; output a character

			setemit vtx0, false, false
				add o0,  v0, r15 (0x8)
				mov o0, c21 (0x7)
				mov o1, c31 (0x5)
				mov o2,  v0 (0x9)
				mov o3, c20 (0x8)
			emit

			setemit vtx1, false, false
				add o0,  v1, r15 (0x8)
				mov o0, c21 (0x7)
				mov o1, c31 (0x5)
				mov o2,  v1 (0x9)
				mov o3, c20 (0x8)
			emit

			setemit vtx2, true, true
				add o0,  v2, r15 (0x8)
				mov o0, c21 (0x7)
				mov o1, c31 (0x5)
				mov o2,  v2 (0x9)
				mov o3, c20 (0x8)
			emit

			setemit vtx1, true, false
				add o0,  v3, r15 (0x8)
				mov o0, c21 (0x7)
				mov o1, c31 (0x5)
				mov o2,  v3 (0x9)
				mov o3, c20 (0x8)
			emit

			add r15, v3, r15 (0x8)
		end_if_2:

		end
		nop
	endmain_gsh:

; operand descriptors
	.opdesc x___, wwww, wwww ; 0x0
	.opdesc _y__, xxxx, xxxx ; 0x1
	.opdesc __z_, xyzw, xyzw ; 0x2
	.opdesc ___w, xyzw, xyzw ; 0x3
	.opdesc __zw, wywy, xyxy ; 0x4
	.opdesc xyzw, xyzw, xyzw ; 0x5
	.opdesc __zw, zyzy, xyxy ; 0x6
	.opdesc __zw, zwzw, xyxy ; 0x7
	.opdesc xy__, xyzw, xyzw ; 0x8
	.opdesc xy__, zwzw, zwzw ; 0x9
	.opdesc __zw, xyxy, xyxy ; 0xa
	.opdesc x___, -yywz, xyzw ; 0xb
	.opdesc xyzw, zzzz, zwzw ; 0xc
	.opdesc xyzw, wwww, xyzw ; 0xd
	.opdesc _y__, xyzw, xyzw ; 0xe
	.opdesc _y__, -xyzw, xyzw ; 0xf
