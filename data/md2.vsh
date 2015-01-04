; make sure you update aemstro_as for this (01/01/15)
 
; setup constants
	; normals
	.const c0, -0.525731, 0.000000, 0.850651, 0.0
	.const c1, -0.442863, 0.238856, 0.864188, 0.0
	.const c2, -0.295242, 0.000000, 0.955423, 0.0
	.const c3, -0.309017, 0.500000, 0.809017, 0.0
	.const c4, -0.162460, 0.262866, 0.951056, 0.0
	.const c5, 0.000000, 0.000000, 1.000000, 0.0
	.const c6, 0.000000, 0.850651, 0.525731, 0.0
	.const c7, -0.147621, 0.716567, 0.681718, 0.0
	.const c8, 0.147621, 0.716567, 0.681718, 0.0
	.const c9, 0.000000, 0.525731, 0.850651, 0.0
	.const c10, 0.309017, 0.500000, 0.809017, 0.0
	.const c11, 0.525731, 0.000000, 0.850651, 0.0
	.const c12, 0.295242, 0.000000, 0.955423, 0.0
	.const c13, 0.442863, 0.238856, 0.864188, 0.0
	.const c14, 0.162460, 0.262866, 0.951056, 0.0
	.const c15, -0.681718, 0.147621, 0.716567, 0.0
	.const c16, -0.809017, 0.309017, 0.500000, 0.0
	.const c17, -0.587785, 0.425325, 0.688191, 0.0
	.const c18, -0.850651, 0.525731, 0.000000, 0.0
	.const c19, -0.864188, 0.442863, 0.238856, 0.0
	.const c20, -0.716567, 0.681718, 0.147621, 0.0
	.const c21, -0.688191, 0.587785, 0.425325, 0.0
	.const c22, -0.500000, 0.809017, 0.309017, 0.0
	.const c23, -0.238856, 0.864188, 0.442863, 0.0
	.const c24, -0.425325, 0.688191, 0.587785, 0.0
	.const c25, -0.716567, 0.681718, -0.147621, 0.0
	.const c26, -0.500000, 0.809017, -0.309017, 0.0
	.const c27, -0.525731, 0.850651, 0.000000, 0.0
	.const c28, 0.000000, 0.850651, -0.525731, 0.0
	.const c29, -0.238856, 0.864188, -0.442863, 0.0
	.const c30, 0.000000, 0.955423, -0.295242, 0.0
	.const c31, -0.262866, 0.951056, -0.162460, 0.0
	.const c32, 0.000000, 1.000000, 0.000000, 0.0
	.const c33, 0.000000, 0.955423, 0.295242, 0.0
	.const c34, -0.262866, 0.951056, 0.162460, 0.0
	.const c35, 0.238856, 0.864188, 0.442863, 0.0
	.const c36, 0.262866, 0.951056, 0.162460, 0.0
	.const c37, 0.500000, 0.809017, 0.309017, 0.0
	.const c38, 0.238856, 0.864188, -0.442863, 0.0
	.const c39, 0.262866, 0.951056, -0.162460, 0.0
	.const c40, 0.500000, 0.809017, -0.309017, 0.0
	.const c41, 0.850651, 0.525731, 0.000000, 0.0
	.const c42, 0.716567, 0.681718, 0.147621, 0.0
	.const c43, 0.716567, 0.681718, -0.147621, 0.0
	.const c44, 0.525731, 0.850651, 0.000000, 0.0
	.const c45, 0.425325, 0.688191, 0.587785, 0.0
	.const c46, 0.864188, 0.442863, 0.238856, 0.0
	.const c47, 0.688191, 0.587785, 0.425325, 0.0
	.const c48, 0.809017, 0.309017, 0.500000, 0.0
	.const c49, 0.681718, 0.147621, 0.716567, 0.0
	.const c50, 0.587785, 0.425325, 0.688191, 0.0
	.const c51, 0.955423, 0.295242, 0.000000, 0.0
	.const c52, 1.000000, 0.000000, 0.000000, 0.0
	.const c53, 0.951056, 0.162460, 0.262866, 0.0
	.const c54, 0.955423, -0.295242, 0.000000, 0.0
	.const c55, 0.864188, -0.442863, 0.238856, 0.0
	.const c56, 0.951056, -0.162460, 0.262866, 0.0
	.const c57, 0.809017, -0.309017, 0.500000, 0.0
	.const c58, 0.681718, -0.147621, 0.716567, 0.0
	.const c59, 0.850651, 0.000000, 0.525731, 0.0
	.const c60, 0.864188, 0.442863, -0.238856, 0.0
	.const c61, 0.809017, 0.309017, -0.500000, 0.0
	.const c62, 0.951056, 0.162460, -0.262866, 0.0
	.const c63, 0.681718, 0.147621, -0.716567, 0.0
	.const c64, 0.850651, 0.000000, -0.525731, 0.0
	.const c65, 0.951056, -0.162460, -0.262866, 0.0
	.const c66, 0.147621, 0.716567, -0.681718, 0.0
	.const c67, 0.309017, 0.500000, -0.809017, 0.0
	.const c68, 0.425325, 0.688191, -0.587785, 0.0
	.const c69, 0.442863, 0.238856, -0.864188, 0.0
	.const c70, 0.587785, 0.425325, -0.688191, 0.0
	.const c71, 0.688191, 0.587785, -0.425325, 0.0
	.const c72, -0.147621, 0.716567, -0.681718, 0.0
	.const c73, -0.309017, 0.500000, -0.809017, 0.0
	.const c74, 0.000000, 0.525731, -0.850651, 0.0
	.const c75, -0.442863, 0.238856, -0.864188, 0.0
	.const c76, -0.162460, 0.262866, -0.951056, 0.0
	.const c77, 0.162460, 0.262866, -0.951056, 0.0
	.const c78, 0.425325, -0.688191, 0.587785, 0.0
	.const c79, 0.587785, -0.425325, 0.688191, 0.0
	.const c80, 0.688191, -0.587785, 0.425325, 0.0
	.const c81, -0.688191, -0.587785, -0.425320, 0.0

	; stuff
	.const c82, 82.0, 0.5774, 0.5, 1.0
 
; setup outmap
	.out o0, result.position, 0xF
	.out o1, result.color, 0xF
	.out o2, result.texcoord0, 0x3
	.out o3, result.texcoord1, 0x3

; setup uniform map (required to use SHDR_GetUniformRegister)
	.uniform c83, c83, frameParam      ; x = interp factor, y = 1 / skin width, z = 1 / skin height, w unused
	.uniform c84, c87, projection      ; c84-c87 = projection matrix
	.uniform c88, c91, modelview       ; c88-c91 = modelview matrix
	.uniform c92, c92, scale0
	.uniform c93, c93, translation0
	.uniform c94, c94, scale1
	.uniform c95, c95, translation1

; setup vsh and gsh
	.vsh main, endmain
 
; code
	main:
		; get vertex position (interpolate)
			; scale + translate frame0 vertex
			mov r1,   v0       (0x6)
			mul r1,  c92,  r1  (0x6)
			add r1,  c93,  r1  (0x6)

			; scale + translate frame1 vertex
			mov r2,   v1       (0x6)
			mul r2,  c94,  r2  (0x6)
			add r2,  c95,  r2  (0x6)

			; interpolate
			add r2, r2, r1 (0xa)
			mad r1, r2, c83, r1 (0x9)

			; set w
			mov r1,  c82       (0x3)

		; get normal (interpolate)
			; grab frame0 normal from index
			mov r0, v0 (0x5)
			call get_normal, end_get_normal
			mov r2, r0 (0x5)

			; grab frame1 normal from index
			mov r0, v1 (0x5)
			call get_normal, end_get_normal
			mov r3, r0 (0x5)

			; interpolate
			add r3, r3, r2 (0xa)
			mad r2, r3, c83, r2 (0x9)

		; tempreg = mdlvMtx * in.pos
			dp4 r0,  c88,  r1  (0x0)
			dp4 r0,  c89,  r1  (0x1)
			dp4 r0,  c90,  r1  (0x2)
			mov r0,  c82       (0x3)

		; result.pos = projMtx * tempreg
			dp4 o0,  c84,  r0  (0x0)
			dp4 o0,  c85,  r0  (0x1)
			dp4 o0,  c86,  r0  (0x2)
			dp4 o0,  c87,  r0  (0x3)

		; result.texcoord = in.texcoord
			mul r3,  c83,  v2  (0xd) ; divide by skin dimensions
			mov o2,   r3       (0x5)
			mov o3,   r3       (0x5)

		; result.color = EXPERIMENTS !
			;mov r0,  c82       (0x7) ; r0 = color (black by default)
			;mov r0,   r2       (0x6)
			;mov o1,   r0       (0x5)
			
			mov r4,  c82       (0x7)
			dp3 o1,   r2,  r4  (0x6)
			mov o1,  c82       (0x3)
			
		end
		nop
	endmain:

	; input : r0.w = normal index
	; output : r0 = normal vector
	get_normal:
		cmp c82, GT, GT, r0 (0xb)

		mova o0,  r0       (0x8)

		ifc end_if, else, cmp.x
			mova o0,  r0       (0x8)
			mov r0, c0[a0.x]   (0x5)
			nop ; necessary for some reason (else gpu crashes ?)
		else:
			add r0, c82, r0    (0xc)
			mova o0,  r0       (0x8)
			mov r0, c0[a0.x]   (0x5)
		end_if:
		nop ; necessary for some reason (else gpu crashes ?)
	end_get_normal:

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
