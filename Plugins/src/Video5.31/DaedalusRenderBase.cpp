/*
Copyright (C) 2003 Rice1964

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "stdafx.h"

extern FiddledVtx * g_pVtxBase;

#define ENABLE_CLIP_TRI
#define X_CLIP_MAX	0x1
#define X_CLIP_MIN	0x2
#define Y_CLIP_MAX	0x4
#define Y_CLIP_MIN	0x8
#define Z_CLIP_MAX	0x10
#define Z_CLIP_MIN	0x20

#ifdef ENABLE_CLIP_TRI

#define index(a,b)	(((a)<<2)+(b))

inline void RSP_Vtx_Clipping(int i)
{
	g_clipFlag[i] = 0;
	g_clipFlag2[i] = 0;
	if( g_vecProjected[i].w > 0 )
	{
		/*
		if( gRSP.bRejectVtx )
		{
			if( g_vecProjected[i].x > 1 )	
			{
				g_clipFlag2[i] |= X_CLIP_MAX;
				if( g_vecProjected[i].x > gRSP.real_clip_ratio_posx )	
					g_clipFlag[i] |= X_CLIP_MAX;
			}

			if( g_vecProjected[i].x < -1 )	
			{
				g_clipFlag2[i] |= X_CLIP_MIN;
				if( g_vecProjected[i].x < gRSP.real_clip_ratio_negx )	
					g_clipFlag[i] |= X_CLIP_MIN;
			}

			if( g_vecProjected[i].y > 1 )	
			{
				g_clipFlag2[i] |= Y_CLIP_MAX;
				if( g_vecProjected[i].y > gRSP.real_clip_ratio_posy )	
					g_clipFlag[i] |= Y_CLIP_MAX;
			}

			if( g_vecProjected[i].y < -1 )	
			{
				g_clipFlag2[i] |= Y_CLIP_MIN;
				if( g_vecProjected[i].y < gRSP.real_clip_ratio_negy )	
					g_clipFlag[i] |= Y_CLIP_MIN;
			}

			//if( g_vecProjected[i].z > 1.0f )	
			//{
			//	g_clipFlag2[i] |= Z_CLIP_MAX;
			//	g_clipFlag[i] |= Z_CLIP_MAX;
			//}

			//if( gRSP.bNearClip && g_vecProjected[i].z < -1.0f )	
			//{
			//	g_clipFlag2[i] |= Z_CLIP_MIN;
			//	g_clipFlag[i] |= Z_CLIP_MIN;
			//}
		}
		else
		*/
		{
			if( g_vecProjected[i].x > 1 )	g_clipFlag2[i] |= X_CLIP_MAX;
			if( g_vecProjected[i].x < -1 )	g_clipFlag2[i] |= X_CLIP_MIN;
			if( g_vecProjected[i].y > 1 )	g_clipFlag2[i] |= Y_CLIP_MAX;
			if( g_vecProjected[i].y < -1 )	g_clipFlag2[i] |= Y_CLIP_MIN;
			//if( g_vecProjected[i].z > 1.0f )	g_clipFlag2[i] |= Z_CLIP_MAX;
			//if( gRSP.bNearClip && g_vecProjected[i].z < -1.0f )	g_clipFlag2[i] |= Z_CLIP_MIN;
		}

	}
}

#else
inline void RSP_Vtx_Clipping(int i) {}
#endif

/*
 *	Global variables
 */
RSP_Options gRSP;
RDP_Options gRDP;

#if _MSC_VER > 1200
__declspec(align(16)) static D3DXVECTOR4 g_normal;
#else
static D3DXVECTOR4 g_normal;
#endif
static int norms[3];

#if _MSC_VER > 1200
__declspec(align(16)) D3DXVECTOR4	g_vtxNonTransformed[MAX_VERTS];
__declspec(align(16)) D3DXVECTOR4	g_vecProjected[MAX_VERTS];
__declspec(align(16)) D3DXVECTOR4	g_vtxTransformed[MAX_VERTS];
#else
D3DXVECTOR4	g_vtxNonTransformed[MAX_VERTS];
D3DXVECTOR4	g_vecProjected[MAX_VERTS];
D3DXVECTOR4	g_vtxTransformed[MAX_VERTS];
#endif
float		g_vtxProjected5[1000][5];
float		g_fFogCoord[MAX_VERTS];
BYTE		g_FogBytes[MAX_VERTS];
DWORD		g_dwVecFlags[MAX_VERTS];			// Z_POS Z_NEG etc
VECTOR2		g_vecTexture[MAX_VERTS];
VECTOR2		g_vecTextureMapped[MAX_VERTS];
DWORD		g_dwVecDiffuseCol[MAX_VERTS];
DWORD		g_clipFlag[MAX_VERTS];
DWORD		g_clipFlag2[MAX_VERTS];
DaedalusRenderTexture g_textures[MAX_TEXTURES];

TLITVERTEX			g_ucVertexBuffer[1000];
unsigned int		g_vtxIndex[1000];
unsigned int		g_triVtxIndexes[1000];
unsigned int		g_minIndex, g_maxIndex;
bool				g_triVtxInitFlags[1000];
BYTE				g_oglVtxColors[1000][4];
float				gRSPfFogMin;
float				gRSPfFogMax;
float				gRSPfFogDivider;

DWORD			gRSPnumLights;
DaedalusLight	gRSPlights[16];
#if _MSC_VER > 1200
__declspec(align(16)) DaedalusMatrix	gRSPworldProjectTransported;
__declspec(align(16)) DaedalusMatrix	gRSPworldProject;
__declspec(align(16)) DaedalusMatrix	gRSPmodelViewTop;
__declspec(align(16)) DaedalusMatrix	gRSPmodelViewTopTranspose;
__declspec(align(16)) DaedalusMatrix	dkrMatrixTransposed;
#else
DaedalusMatrix	gRSPworldProjectTransported;
DaedalusMatrix	gRSPworldProject;
DaedalusMatrix	gRSPmodelViewTop;
DaedalusMatrix	gRSPmodelViewTopTranspose;
DaedalusMatrix	dkrMatrixTransposed;
#endif
N64Light		gRSPn64lights[16];


void (*SetNewVertexInfo)(DWORD dwAddress, DWORD dwV0, DWORD dwNum)=NULL;

/*
 *	
 */


/*n.x = (g_normal.x * matWorld.m00) + (g_normal.y * matWorld.m10) + (g_normal.z * matWorld.m20);
n.y = (g_normal.x * matWorld.m01) + (g_normal.y * matWorld.m11) + (g_normal.z * matWorld.m21);
n.z = (g_normal.x * matWorld.m02) + (g_normal.y * matWorld.m12) + (g_normal.z * matWorld.m22);*/

// Multiply (x,y,z,0) by matrix m, then normalize
#define Vec3TransformNormal(vec, m) __asm					\
{														\
	__asm fld		dword ptr [vec + 0]							\
	__asm fmul	dword ptr [m + 0]			/* x m00*/		\
	__asm fld		dword ptr [vec + 0]							\
	__asm fmul	dword ptr [m + 4] 	/* x m01  x m00*/			\
	__asm fld		dword ptr [vec + 0]								\
	__asm fmul	dword ptr [m + 8] 	/* x m02  x m01  x m00*/	\
	\
	__asm fld		dword ptr [vec + 4]								\
	__asm fmul	dword ptr [m + 16] 	/* y m10  x m02  x m01  x m00*/	\
	__asm fld		dword ptr [vec + 4]									\
	__asm fmul	dword ptr [m + 20] 	/* y m11  y m10  x m02  x m01  x m00*/		\
	__asm fld		dword ptr [vec + 4]												\
	__asm fmul	dword ptr [m + 24]	/* y m12  y m11  y m10  x m02  x m01  x m00*/	\
	\
	__asm fxch	st(2)				/* y m10  y m11  y m12  x m02  x m01  x m00*/			\
	__asm faddp	st(5), st(0)		/* y m11  y m12  x m02  x m01  (x m00 + y m10)*/		\
	__asm faddp	st(3), st(0)		/* y m12  x m02  (x m01 + ym11)  (x m00 + y m10)*/	\
	__asm faddp	st(1), st(0)		/* (x m02 + y m12) (x m01 + ym11)  (x m00 + y m10)*/	\
	\
	__asm fld		dword ptr [vec + 8]														\
	__asm fmul	dword ptr [m + 32] /* z m20  (x m02 + y m12) (x m01 + ym11)  (x m00 + y m10)*/	\
	__asm fld		dword ptr [vec + 8]																\
	__asm fmul	dword ptr [m + 36] /* z m21  z m20  (x m02 + y m12) (x m01 + ym11)  (x m00 + y m10)*/				\
	__asm fld		dword ptr [vec + 8]																				\
	__asm fmul	dword ptr [m + 40] /* z m22  z m21  z m20  (x m02 + y m12) (x m01 + ym11)  (x m00 + y m10)*/		\
	\
	__asm fxch	st(2)				/* z m20  z m21  z m22  (x m02 + y m12) (x m01 + ym11)  (x m00 + y m10)*/		\
	__asm faddp	st(5), st(0)		/* z m21  z m22  (x m02 + y m12) (x m01 + ym11)  (x m00 + y m10 + z m20)*/	\
	__asm faddp	st(3), st(0)		/* z m22  (x m02 + y m12) (x m01 + ym11 + z m21)  (x m00 + y m10 + z m20)*/	\
	__asm faddp	st(1), st(0)		/* (x m02 + y m12 + z m 22) (x m01 + ym11 + z m21)  (x m00 + y m10 + z m20)*/	\
	\
	__asm fxch	st(2)				/* (x m00 + y m10 + z m20) (x m01 + ym11 + z m21) (x m02 + y m12 + z m 22) */	\
	\
	__asm fld1                      /* 1 x y z */ \
	__asm fld   st(1)				/* x 1 x y z */	\
	__asm fmul  st(0),st(0)			/* xx 1 x y z */  \
	__asm fld   st(3)               /* y xx 1 x y z */ \
	__asm fmul  st(0),st(0)			/* yy xx 1 x y z */ \
	__asm fld   st(5)				/* z yy xx 1 x y z */ \
	__asm fmul  st(0),st(0)			/* zz yy xx 1 x y z */ \
	\
	__asm fxch  st(2)				/* xx yy zz 1 x y z */ \
	\
	__asm faddp st(1),st(0)			/* (xx+yy) zz 1 x y z */ \
	__asm faddp st(1),st(0)			/* (xx+yy+zz) 1 x y z */ \
	\
	__asm fsqrt						/* l 1 x y z */ \
	\
	__asm fdivp st(1),st(0)			/* (1/l) x y z */ \
	\
	__asm fmul  st(3),st(0)		    /* f x y fz */										\
	__asm fmul  st(2),st(0)			/* f x fy fz */										\
	__asm fmulp st(1),st(0)			/* fx fy fz */										\
	\
	__asm fstp	dword ptr [vec + 0]	/* fy fz*/							\
	__asm fstp	dword ptr [vec + 4]	/* fz	*/			\
	__asm fstp	dword ptr [vec + 8]	/* done	*/																		\
}		\


// Multiply (x,y,z,1) by matrix m and project onto w = 1
#define Vec3TransformCoord(vecout, vec, m) __asm			\
{														\
	__asm fld	dword ptr [vec + 0]							\
	__asm fmul	dword ptr [m + 0]			/* x m00*/		\
	__asm fld	dword ptr [vec + 0]							\
	__asm fmul	dword ptr [m + 4] 	/* x m01  x m00*/			\
	__asm fld	dword ptr [vec + 0]								\
	__asm fmul	dword ptr [m + 8] 	/* x m02  x m01  x m00*/	\
	\
	__asm fld	dword ptr [vec + 4]								\
	__asm fmul	dword ptr [m + 16] 	/* y m10  x m02  x m01  x m00*/	\
	__asm fld	dword ptr [vec + 4]									\
	__asm fmul	dword ptr [m + 20] 	/* y m11  y m10  x m02  x m01  x m00*/		\
	__asm fld	dword ptr [vec + 4]												\
	__asm fmul	dword ptr [m + 24]	/* y m12  y m11  y m10  x m02  x m01  x m00*/	\
	\
	__asm fxch	st(2)				/* y m10  y m11  y m12  x m02  x m01  x m00*/			\
	__asm faddp	st(5), st(0)		/* y m11  y m12  x m02  x m01  (x m00 + y m10)*/		\
	__asm faddp	st(3), st(0)		/* y m12  x m02  (x m01 + ym11)  (x m00 + y m10)*/	\
	__asm faddp	st(1), st(0)		/* (x m02 + y m12) (x m01 + ym11)  (x m00 + y m10)*/	\
	\
	__asm fld	dword ptr [vec + 8]														\
	__asm fmul	dword ptr [m + 32] /* z m20  (x m02 + y m12) (x m01 + ym11)  (x m00 + y m10)*/	\
	__asm fld	dword ptr [vec + 8]																\
	__asm fmul	dword ptr [m + 36] /* z m21  z m20  (x m02 + y m12) (x m01 + ym11)  (x m00 + y m10)*/				\
	__asm fld	dword ptr [vec + 8]																				\
	__asm fmul	dword ptr [m + 40] /* z m22  z m21  z m20  (x m02 + y m12) (x m01 + ym11)  (x m00 + y m10)*/		\
	\
	__asm fxch	st(2)				/* z m20  z m21  z m22  (x m02 + y m12) (x m01 + ym11)  (x m00 + y m10)*/		\
	__asm faddp	st(5), st(0)		/* z m21  z m22  (x m02 + y m12) (x m01 + ym11)  (x m00 + y m10 + z m20)*/	\
	__asm faddp	st(3), st(0)		/* z m22  (x m02 + y m12) (x m01 + ym11 + z m21)  (x m00 + y m10 + z m20)*/	\
	__asm faddp	st(1), st(0)		/* (x m02 + y m12 + z m 22) (x m01 + ym11 + z m21)  (x m00 + y m10 + z m20)*/	\
	\
	__asm fxch	st(2)				/* (x m00 + y m10 + z m20) (x m01 + ym11 + z m21) (x m02 + y m12 + z m 22) */	\
	__asm fadd	dword ptr [m + 48]	/* (xm00+ym10+zm20+m30)	 (xm01+ym11+zm21) (xm02+ym12+zm22) */		\
	__asm fxch	st(1)				/* (xm01+ym11+zm21) (xm00+ym10+zm20+m30)  (xm02+ym12+zm22) */		\
	__asm fadd	dword ptr [m + 52] 	/* (xm01+ym11+zm21+m31) (xm00+ym10+zm20+m30)  (xm02+ym12+zm22) */		\
	__asm fxch	st(2)				/* (xm02+ym12+zm22) (xm00+ym10+zm20+m30)(xm01+ym11+zm21+m31)   */	\
	__asm fadd	dword ptr [m + 56]	/* (xm02+ym12+zm22+m32) (xm00+ym10+zm20+m30)(xm01+ym11+zm21+m31)   */	\
	__asm fxch	st(1)				/* (xm00+ym10+zm20+m30)(xm02+ym12+zm22+m32) (xm01+ym11+zm21+m31)   */	\
	\
	__asm fld1						/* 1 abc */											\
	__asm fld	dword ptr [vec + 0]	/* x 1 abc*/										\
	__asm fmul  dword ptr [m + 12]  /* xm03 1 abc*/										\
	__asm fld	dword ptr [vec + 4]	/* y xm03 1 abc*/									\
	__asm fmul  dword ptr [m + 28]	/* ym13 xm03 1 abc*/								\
	__asm fld	dword ptr [vec + 8]	/* z ym13 xm03 1 abc*/								\
	__asm fmul  dword ptr [m + 44]  /* zm23 ym13 xm03 1 abc*/							\
	\
	__asm fxch	st(2)				/* xm03 ym13 zm23 1 abc*/							\
	__asm faddp st(1), st(0)		/* (xm03 + ym13) zm23 1 abc*/						\
	__asm faddp	st(1), st(0)		/* (xm03 + ym13 + zm23) 1 abc*/						\
	\
	__asm fadd  dword ptr [m + 60]	/* (xm03+ym13+zm23+m33) 1 abc*/						\
	\
	__asm fdivp  st(1), st(0)		/* 1.0 / (xm03+ym13+zm23+m33) abc*/					\
	\
	__asm fmul  st(3),st(0)		    /* f a b fc */										\
	__asm fmul  st(2),st(0)			/* f a fb fc */										\
	__asm fmulp st(1),st(0)			/* fx fz fy */										\
	\
	__asm fstp	dword ptr [vecout + 0]	/* (xm02+ym12+zm22+m32) (xm01+ym11+zm21+m31)*/	\
	__asm fstp	dword ptr [vecout + 8]	/* (xm01+ym11+zm21+m31)	*/						\
	__asm fstp	dword ptr [vecout + 4]	/* done	*/										\
}																						\

#ifdef USING_INT_MATRIX
void IntVec3Transform(D3DXVECTOR4 &vec4, FiddledVtx &vtx, N64IntMatrix &mtx)
{
	int x = vtx.x*mtx.nums[0]+vtx.y*mtx.nums[4]+vtx.z*mtx.nums[8]+mtx.nums[12];
	int y = vtx.x*mtx.nums[1]+vtx.y*mtx.nums[5]+vtx.z*mtx.nums[9]+mtx.nums[13];
	int z = vtx.x*mtx.nums[2]+vtx.y*mtx.nums[6]+vtx.z*mtx.nums[10]+mtx.nums[14];
	int w = vtx.x*mtx.nums[3]+vtx.y*mtx.nums[7]+vtx.z*mtx.nums[11]+mtx.nums[15];

	vec4.x = x + (vtx.x*(int)mtx.decs[0]+vtx.y*(int)mtx.decs[4]+vtx.z*(int)mtx.decs[8]+(int)mtx.decs[12])/65536.0f;
	vec4.y = y + (vtx.x*(int)mtx.decs[1]+vtx.y*(int)mtx.decs[5]+vtx.z*(int)mtx.decs[9]+(int)mtx.decs[13])/65536.0f;
	vec4.z = z + (vtx.x*(int)mtx.decs[2]+vtx.y*(int)mtx.decs[6]+vtx.z*(int)mtx.decs[10]+(int)mtx.decs[14])/65536.0f;
	vec4.w = w + (vtx.x*(int)mtx.decs[3]+vtx.y*(int)mtx.decs[7]+vtx.z*(int)mtx.decs[11]+(int)mtx.decs[15])/65536.0f;
}

void IntVec3TransformNormal(int norms[3], NormalStruct &norm, N64IntMatrix &mtx)
{
	int x = norm.nx*mtx.nums[0]+norm.ny*mtx.nums[4]+norm.nz*mtx.nums[8];
	int y = norm.nx*mtx.nums[1]+norm.ny*mtx.nums[5]+norm.nz*mtx.nums[9];
	int z = norm.nx*mtx.nums[2]+norm.ny*mtx.nums[6]+norm.nz*mtx.nums[10];

	norms[0] = x + ((norm.nx*(int)mtx.decs[0]+norm.ny*(int)mtx.decs[4]+norm.nz*(int)mtx.decs[8])>>16);
	norms[1] = y + ((norm.nx*(int)mtx.decs[1]+norm.ny*(int)mtx.decs[5]+norm.nz*(int)mtx.decs[9])>>16);
	norms[2] = z + ((norm.nx*(int)mtx.decs[2]+norm.ny*(int)mtx.decs[6]+norm.nz*(int)mtx.decs[10])>>16);
}

#endif


__declspec( naked ) void  __fastcall SSEVec3Transform(int i)
{
#if _MSC_VER > 1200
	//SSEVec3Transform(g_vtxTransformed[i], g_vtxNonTransformed[i]);
	__asm
	{
		shl		ecx,4;		// ecx = i

		movaps	xmm1,	DWORD PTR g_vtxNonTransformed [ecx];		// xmm1 as original vector

		movaps	xmm4,	DWORD PTR gRSPworldProjectTransported;			// row1
		movaps	xmm5,	DWORD PTR gRSPworldProjectTransported[0x10];	// row2
		movaps	xmm6,	DWORD PTR gRSPworldProjectTransported[0x20];	// row3
		movaps	xmm7,	DWORD PTR gRSPworldProjectTransported[0x30];	// row4

		mulps	xmm4, xmm1;		// row 1
		mulps	xmm5, xmm1;		// row 2
		mulps	xmm6, xmm1;		// row 3
		mulps	xmm7, xmm1;		// row 4

		movhlps	xmm0, xmm4;		// xmm4 high to xmm0 low
		movlhps	xmm0, xmm5;		// xmm5 low to xmm0 high

		addps	xmm4, xmm0;		// result of add are in xmm4 low
		addps	xmm5, xmm0;		// result of add are in xmm5 high

		shufps	xmm0, xmm4, 0x44;	// move xmm4 low DWORDs to xmm0 high
		shufps	xmm4, xmm5, 0xe4;	// move xmm5 high DWORS to xmm4
		movhlps	xmm5, xmm0;			// xmm4, xmm5 are mirrored

		shufps	xmm4, xmm4, 0x08;	// move xmm4's 3rd DWORD to its 2nd DWORD
		shufps	xmm5, xmm5, 0x0d;	// move xmm5's 4th DWORD to its 2nd DWORD, 
									// and move its 2nd DWORD to its 1st DWORD
		
		addps	xmm4, xmm5;		// results are in 1st and 2nd DWORD


		movhlps	xmm0, xmm6;		// xmm6 high to xmm0 low
		movlhps	xmm0, xmm7;		// xmm7 low to xmm0 high

		addps	xmm6, xmm0;		// result of add are in xmm6 low
		addps	xmm7, xmm0;		// result of add are in xmm7 high

		shufps	xmm0, xmm6, 0x44;	// move xmm6 low DWORDs to xmm0 high
		shufps	xmm6, xmm7, 0xe4;	// move xmm7 high DWORS to xmm6
		movhlps	xmm7, xmm0;			// xmm6, xmm7 are mirrored

		shufps	xmm6, xmm6, 0x08;	// move xmm6's 3rd DWORD to its 2nd DWORD
		shufps	xmm7, xmm7, 0x0d;	// move xmm7's 4th DWORD to its 2nd DWORD, 
									// and move its 2nd DWORD to its 1st DWORD
		
		addps	xmm6, xmm7;		// results are in 1st and 2nd DWORD
		
		movlhps xmm4, xmm6;		// final result is in xmm4
		movaps  DWORD PTR g_vtxTransformed [ecx], xmm4;

		movaps	xmm0,xmm4;
		shufps	xmm0,xmm0,0xff;
		divps	xmm4,xmm0;
		rcpps	xmm0,xmm0;
		movhlps	xmm0,xmm4;
		shufps	xmm0,xmm0,0xe8;
		movlhps	xmm4,xmm0;

		movaps	DWORD PTR g_vecProjected [ecx], xmm4;

		emms;
		ret;
	}
#endif
}

// Only used by DKR
__declspec( naked ) void  __fastcall SSEVec3TransformDKR(D3DXVECTOR4 &pOut, const D3DXVECTOR4 &pV)
{
#if _MSC_VER > 1200
	__asm
	{
		movaps	xmm1,	DWORD PTR [edx];		// xmm1 as original vector

		movaps	xmm4,	DWORD PTR dkrMatrixTransposed;	// row1
		movaps	xmm5,	DWORD PTR dkrMatrixTransposed[0x10];	// row2
		movaps	xmm6,	DWORD PTR dkrMatrixTransposed[0x20];	// row3
		movaps	xmm7,	DWORD PTR dkrMatrixTransposed[0x30];	// row4

		mulps	xmm4, xmm1;		// row 1
		mulps	xmm5, xmm1;		// row 2
		mulps	xmm6, xmm1;		// row 3
		mulps	xmm7, xmm1;		// row 4

		movhlps	xmm0, xmm4;		// xmm4 high to xmm0 low
		movlhps	xmm0, xmm5;		// xmm5 low to xmm0 high

		addps	xmm4, xmm0;		// result of add are in xmm4 low
		addps	xmm5, xmm0;		// result of add are in xmm5 high

		shufps	xmm0, xmm4, 0x44;	// move xmm4 low DWORDs to xmm0 high
		shufps	xmm4, xmm5, 0xe4;	// move xmm5 high DWORS to xmm4
		movhlps	xmm5, xmm0;			// xmm4, xmm5 are mirrored

		shufps	xmm4, xmm4, 0x08;	// move xmm4's 3rd DWORD to its 2nd DWORD
		shufps	xmm5, xmm5, 0x0d;	// move xmm5's 4th DWORD to its 2nd DWORD, 
		// and move its 2nd DWORD to its 1st DWORD

		addps	xmm4, xmm5;		// results are in 1st and 2nd DWORD


		movhlps	xmm0, xmm6;		// xmm6 high to xmm0 low
		movlhps	xmm0, xmm7;		// xmm7 low to xmm0 high

		addps	xmm6, xmm0;		// result of add are in xmm6 low
		addps	xmm7, xmm0;		// result of add are in xmm7 high

		shufps	xmm0, xmm6, 0x44;	// move xmm6 low DWORDs to xmm0 high
		shufps	xmm6, xmm7, 0xe4;	// move xmm7 high DWORS to xmm6
		movhlps	xmm7, xmm0;			// xmm6, xmm7 are mirrored

		shufps	xmm6, xmm6, 0x08;	// move xmm6's 3rd DWORD to its 2nd DWORD
		shufps	xmm7, xmm7, 0x0d;	// move xmm7's 4th DWORD to its 2nd DWORD, 
		// and move its 2nd DWORD to its 1st DWORD

		addps	xmm6, xmm7;		// results are in 1st and 2nd DWORD

		movlhps xmm4, xmm6;		// final result is in xmm4
		movaps  DWORD PTR [ecx], xmm4;

		emms;
		ret;
	}
#endif
}

float real255 = 255.0f;
float real128 = 128.0f;

__declspec( naked ) void  __fastcall SSEVec3TransformNormal()
{
#if _MSC_VER > 1200
	__asm
	{
		mov		DWORD PTR [g_normal][12], 0;

		movaps	xmm4,	DWORD PTR gRSPmodelViewTopTranspose;	// row1
		movaps	xmm5,	DWORD PTR gRSPmodelViewTopTranspose[0x10];	// row2
		movaps	xmm1,	DWORD PTR [g_normal];		// xmm1 as the normal vector
		movaps	xmm6,	DWORD PTR gRSPmodelViewTopTranspose[0x20];	// row3

		mulps	xmm4, xmm1;		// row 1
		mulps	xmm5, xmm1;		// row 2
		mulps	xmm6, xmm1;		// row 3

		movhlps	xmm0, xmm4;		// xmm4 high to xmm0 low
		movlhps	xmm0, xmm5;		// xmm5 low to xmm0 high

		addps	xmm4, xmm0;		// result of add are in xmm4 low
		addps	xmm5, xmm0;		// result of add are in xmm5 high

		shufps	xmm0, xmm4, 0x44;	// move xmm4 low DWORDs to xmm0 high
		shufps	xmm4, xmm5, 0xe4;	// move xmm5 high DWORS to xmm4
		movhlps	xmm5, xmm0;			// xmm4, xmm5 are mirrored

		shufps	xmm4, xmm4, 0x08;	// move xmm4's 3rd DWORD to its 2nd DWORD
		shufps	xmm5, xmm5, 0x0d;	// move xmm5's 4th DWORD to its 2nd DWORD, 

		addps	xmm4, xmm5;		// results are in 1st and 2nd DWORD

		movaps	xmm1,xmm4;
		mulps	xmm1,xmm1;	//square
		movlhps	xmm7, xmm1;
		shufps	xmm7, xmm7,0x03;
		addss	xmm7, xmm1;

		movhlps	xmm0, xmm6;		// xmm6 high to xmm0 low
		addps	xmm6, xmm0;		// result of add are in xmm6 low

		movlhps	xmm0, xmm6;
		shufps	xmm0, xmm0, 0x03;
		addss	xmm0, xmm6;		// result of add is at xmm0's 1st DWORD

		movlhps	xmm4, xmm0;

		mulss	xmm0,xmm0;
		addss	xmm7,xmm0;		// xmm7 1st DWORD is the sum of squares

#ifdef _DEBUG
		movaps  DWORD PTR [g_normal], xmm4;
		movss  DWORD PTR [g_normal][12], xmm7;
#endif

		rsqrtss	xmm7,xmm7;
		shufps	xmm7,xmm7,0;
#ifdef _DEBUG
		movss  DWORD PTR [g_normal][12], xmm7;
#endif
		mulps	xmm4,xmm7;

		movaps  DWORD PTR [g_normal], xmm4;		// Normalized
		mov		DWORD PTR [g_normal][12], 0;

		emms;
		ret;
	}
#endif
}


void InitRenderBase()
{
	if( status.isSSEEnabled )
	{
		SetNewVertexInfo = SetNewVertexInfoSSE;
	}
	else
	{
		SetNewVertexInfo = SetNewVertexInfoNoSSE;
	}

	gRSPfFogMin = gRSP.fFogMul = gRSP.fFogOffset = gRSPfFogMax = 0.0f;
	windowSetting.fMultX = windowSetting.fMultY = 2.0f;
	windowSetting.vpLeftW = windowSetting.vpTopW = 0;
	windowSetting.vpRightW = windowSetting.vpWidthW = 640;
	windowSetting.vpBottomW = windowSetting.vpHeightW = 480;
	gRSP.maxZ = 0;
	gRSP.nVPLeftN = gRSP.nVPTopN = 0;
	gRSP.nVPRightN = 640;
	gRSP.nVPBottomN = 640;
	gRSP.nVPWidthN = 640;
	gRSP.nVPHeightN = 640;
	gRDP.scissor.left=gRDP.scissor.top=0;
	gRDP.scissor.right=gRDP.scissor.bottom=640;
	
	gRSP.bLightingEnable = gRSP.bTextureGen = false;
	gRSP.curTile=gRSPnumLights=gRSP.ambientLightColor=gRSP.ambientLightIndex= 0;
	gRSP.fAmbientLightR=gRSP.fAmbientLightG=gRSP.fAmbientLightB=0;
	gRSP.projectionMtxTop = gRSP.modelViewMtxTop = 0;
	gRDP.fogColor = gRDP.primitiveColor = gRDP.envColor = gRDP.primitiveDepth = gRDP.primLODMin = gRDP.primLODFrac = 0;
	gRDP.fPrimitiveDepth = 0;
	gRSP.numVertices = 0;
	gRSP.bCullFront=false;
	gRSP.bCullBack=true;
	gRSP.bFogEnabled=gRDP.bFogEnableInBlender=false;
	gRSP.bZBufferEnabled=true;
	gRSP.shadeMode=SHADE_SMOOTH;
	gRDP.keyR=gRDP.keyG=gRDP.keyB=gRDP.keyA=gRDP.keyRGB=gRDP.keyRGBA = 0;
	gRDP.fKeyA = 0;
	gRSP.DKRCMatrixIndex = gRSP.dwDKRVtxAddr = gRSP.dwDKRMatrixAddr = 0;
	gRSP.DKRVtxAddBase = false;

	gRSP.fTexScaleX = 1/32.0f;
	gRSP.fTexScaleY = 1/32.0f;
	gRSP.bTextureEnabled = FALSE;

	gRSP.clip_ratio_left = 0;
	gRSP.clip_ratio_top = 0;
	gRSP.clip_ratio_right = 640;
	gRSP.clip_ratio_bottom = 480;
	gRSP.clip_ratio_negx = 1;
	gRSP.clip_ratio_negy = 1;
	gRSP.clip_ratio_posx = 1;
	gRSP.clip_ratio_posy = 1;
	gRSP.real_clip_scissor_left = 0;
	gRSP.real_clip_scissor_top = 0;
	gRSP.real_clip_scissor_right = 640;
	gRSP.real_clip_scissor_bottom = 480;
	gRSP.real_clip_ratio_negx = 1;
	gRSP.real_clip_ratio_negy = 1;
	gRSP.real_clip_ratio_posx = 1;
	gRSP.real_clip_ratio_posy = 1;

	gRSP.DKRCMatrixIndex=0;
	gRSP.DKRVtxCount=0;
	gRSP.DKRVtxAddBase = false;
	gRSP.dwDKRVtxAddr=0;
	gRSP.dwDKRMatrixAddr=0;


	gRDP.geometryMode	= 0;
	gRDP.otherModeL		= 0;
	gRDP.otherModeH		= 0;
	gRDP.fillColor		= 0xFFFFFFFF;
	gRDP.originalFillColor	=0;

	gRSP.ucode		= 1;
	gRSP.vertexMult = 10;
	gRSP.bNearClip	= false;
	gRSP.bRejectVtx	= false;

	gRDP.texturesAreReloaded = false;
	gRDP.textureIsChanged = false;

	memset(&gRDP.otherMode,0,sizeof(RDP_OtherMode));
	memset(&gRDP.tiles,0,sizeof(Tile)*8);


	for( int i=0; i<MAX_VERTS; i++ )
		g_clipFlag[i] = 0;
	for( i=0; i<MAX_VERTS; i++ )
		g_vtxNonTransformed[i].w = 1;

	memset(gRSPn64lights, 0, sizeof(N64Light)*16);
}

void SetFogMinMax(float fMin, float fMax, float fMul, float fOffset)
{
	if( fMin<0 || fMax<fMin )
	{
		// Disable Fog
		gRSPfFogMin=gRSPfFogMax=1;
		gRSPfFogDivider=0;
	}
	else
	{
		gRSPfFogMin = max(0,-gRSP.fFogOffset/gRSP.fFogMul);
		//gRSPfFogMax = min(1,(255.0f-gRSP.fFogOffset)/gRSP.fFogMul);
		gRSPfFogMax = (255.0f-gRSP.fFogOffset)/gRSP.fFogMul;
		//gRSPfFogMax = (gRSP.fFogMul-gRSP.fFogOffset)/gRSP.fFogMul;
		gRSP.fFogMul = fMul;
		gRSP.fFogOffset = fOffset;

		gRSPfFogDivider = 255/(gRSPfFogMax-gRSPfFogMin);
	}

}

void InitVertexColors()
{
}

void InitVertexTextureConstants()
{
	float scaleX;
	float scaleY;

	DaedalusRenderTexture &tex0 = g_textures[gRSP.curTile];
	CTexture *surf = tex0.m_pCTexture;
	Tile &tile0 = gRDP.tiles[gRSP.curTile];

	scaleX = gRSP.fTexScaleX;
	scaleY = gRSP.fTexScaleY;

	if( g_curRomInfo.bTextureScaleHack )//&& ( gRSP.shadeMode == 1 || gRSP.ucode == 1 ) )
	{
		if( gRSP.fTexScaleX == 1/64.0f )	scaleX = gRSP.fTexScaleX/2;
		if( gRSP.fTexScaleY == 1/64.0f )	scaleY = gRSP.fTexScaleY/2;
	}

	gRSP.tex0scaleX = scaleX * tile0.fShiftScaleS/tex0.m_fTexWidth;
	gRSP.tex0scaleY = scaleY * tile0.fShiftScaleT/tex0.m_fTexHeight;

	gRSP.tex0OffsetX = tile0.fhilite_sl/tex0.m_fTexWidth;
	gRSP.tex0OffsetY = tile0.fhilite_tl/tex0.m_fTexHeight;

	if( CDaedalusRender::g_pRender->IsTexel1Enable() )
	{
		DaedalusRenderTexture &tex1 = g_textures[(gRSP.curTile+1)&7];
		CTexture *surf = tex1.m_pCTexture;
		Tile &tile1 = gRDP.tiles[(gRSP.curTile+1)&7];

		gRSP.tex1scaleX = scaleX * tile1.fShiftScaleS/tex1.m_fTexWidth;
		gRSP.tex1scaleY = scaleY * tile1.fShiftScaleT/tex1.m_fTexHeight;

		gRSP.tex1OffsetX = tile1.fhilite_sl/tex1.m_fTexWidth;
		gRSP.tex1OffsetY = tile1.fhilite_tl/tex1.m_fTexHeight;
	}

	gRSP.texGenXRatio = tile0.fShiftScaleS;
	gRSP.texGenYRatio = gRSP.fTexScaleX/gRSP.fTexScaleY*tex0.m_fTexWidth/tex0.m_fTexHeight*tile0.fShiftScaleT;
}

void InitVertex(TLITVERTEX &v, DWORD dwV, DWORD vtxIndex, bool bTexture, bool openGL)
{
	StartProfiler(PROFILE_TRANSFORMATING);

	if( openGL )
	{
		g_vtxProjected5[vtxIndex][0] = g_vtxTransformed[dwV].x;
		g_vtxProjected5[vtxIndex][1] = g_vtxTransformed[dwV].y;
		g_vtxProjected5[vtxIndex][2] = g_vtxTransformed[dwV].z;
		g_vtxProjected5[vtxIndex][3] = g_vtxTransformed[dwV].w;
		g_vtxProjected5[vtxIndex][4] = g_fFogCoord[dwV];
		g_vtxIndex[vtxIndex] = vtxIndex;
	}
	else
	{
		v.x = g_vecProjected[dwV].x*gRSP.vtxXMul+gRSP.vtxXAdd;
		v.y = g_vecProjected[dwV].y*gRSP.vtxYMul+gRSP.vtxYAdd;
		v.z = (g_vecProjected[dwV].z + 1.0f) * 0.5f;	// DirectX minZ=0, maxZ=1
		v.rhw = g_vecProjected[dwV].w;

		if( gRSP.bProcessSpecularColor )
		{
			v.dcSpecular = CDaedalusRender::g_pRender->PostProcessSpecularColor();
			if( gRSP.bFogEnabled )
			{
				v.dcSpecular &= 0x00FFFFFF;
				DWORD	fogFct = 0xFF-(BYTE)((g_fFogCoord[dwV]-gRSPfFogMin)*gRSPfFogDivider);
				v.dcSpecular |= (fogFct<<24);
			}
		}
		else if( gRSP.bFogEnabled )
		{
			DWORD	fogFct = 0xFF-(BYTE)((g_fFogCoord[dwV]-gRSPfFogMin)*gRSPfFogDivider);
			v.dcSpecular = (fogFct<<24);
		}
	}


#ifdef _DEBUG
	if( (pauseAtNext && (eventToPause == NEXT_VERTEX_CMD || eventToPause == NEXT_MATRIX_CMD || eventToPause == NEXT_FLUSH_TRI )) && logTriDetails ) 
	{
		DebuggerAppendMsg("      : %f, %f, %f, %f",  v.x,v.y,v.z,v.rhw);
	}
#endif

	v.dcDiffuse = g_dwVecDiffuseCol[dwV];
	if( gRDP.otherMode.key_en )
	{
		v.dcDiffuse &= 0x00FFFFFF;
		v.dcDiffuse |= (gRDP.keyA<<24);
	}
	else if( gRDP.otherMode.aa_en && gRDP.otherMode.clr_on_cvg==0 )
	{
		v.dcDiffuse |= 0xFF000000;
	}

	if( gRSP.bProcessDiffuseColor )
	{
		v.dcDiffuse = CDaedalusRender::g_pRender->PostProcessDiffuseColor(v.dcDiffuse);
	}
#ifdef _DEBUG
	if( options.bWinFrameMode )
	{
		v.dcDiffuse = g_dwVecDiffuseCol[dwV];
	}
#endif

	if( openGL )
	{
		g_oglVtxColors[vtxIndex][0] = v.r;
		g_oglVtxColors[vtxIndex][1] = v.g;
		g_oglVtxColors[vtxIndex][2] = v.b;
		g_oglVtxColors[vtxIndex][3] = v.a;
	}

	if( bTexture )
	{
		// If the vert is already lit, then there is no normal (and hence we can't generate tex coord)
		// Only scale if not generated automatically
		if (gRSP.bTextureGen && gRSP.bLightingEnable)
		{
			// Correction for texGen result

			//DaedalusRenderTexture &tex0 = g_textures[gRSP.curTile];
			//float y = g_vecTexture[dwV].y/gRSP.fTexScaleY*gRSP.fTexScaleX*tex0.m_fTexWidth/tex0.m_fTexHeight;
			CDaedalusRender::g_pRender->SetVertexTextureUVCoord(v, g_vecTexture[dwV].x*gRSP.texGenXRatio, g_vecTexture[dwV].y*gRSP.texGenYRatio);
		}
		else
		{
			float tex0u = g_vecTexture[dwV].x *gRSP.tex0scaleX - gRSP.tex0OffsetX ;
			float tex0v = g_vecTexture[dwV].y *gRSP.tex0scaleY - gRSP.tex0OffsetY ;

			if( CDaedalusRender::g_pRender->IsTexel1Enable() )
			{
				float tex1u = g_vecTexture[dwV].x *gRSP.tex1scaleX - gRSP.tex1OffsetX ;
				float tex1v = g_vecTexture[dwV].y *gRSP.tex1scaleY - gRSP.tex1OffsetY ;

				CDaedalusRender::g_pRender->SetVertexTextureUVCoord(v, tex0u, tex0v, tex1u, tex1v);
#ifdef _DEBUG
				if( (pauseAtNext && (eventToPause == NEXT_VERTEX_CMD || eventToPause == NEXT_MATRIX_CMD || eventToPause == NEXT_FLUSH_TRI )) && logTriDetails ) 
				{
					DebuggerAppendMsg("  (tex0): %f, %f, (tex1): %f, %f",  tex0u,tex0v,tex1u,tex1v);
				}
#endif
			}
			else
			{
				CDaedalusRender::g_pRender->SetVertexTextureUVCoord(v, tex0u, tex0v);
#ifdef _DEBUG
				if( (pauseAtNext && (eventToPause == NEXT_VERTEX_CMD || eventToPause == NEXT_MATRIX_CMD || eventToPause == NEXT_FLUSH_TRI )) && logTriDetails ) 
				{
					DebuggerAppendMsg("  (tex0): %f, %f",  tex0u,tex0v);
				}
#endif
			}

		}
	}


	StopProfiler(PROFILE_TRANSFORMATING);
	DEBUGGER_ONLY_IF( logTriDetails, 
	{
		DebuggerAppendMsg("Vertex: %d: (%f, %f, %f, %f), U=%f, V=%f\n  DIF(%08X), SPE(%08X)\n", dwV, g_vecProjected[dwV].x, g_vecProjected[dwV].y, g_vecProjected[dwV].z, g_vecProjected[dwV].w,
			g_vecTexture[dwV].x, g_vecTexture[dwV].y, v.dcDiffuse, v.dcSpecular );
		if(bTexture) DebuggerAppendMsg("\tT0(%f,%f), T1(%f,%f)\n", v.tcord[0].u, v.tcord[0].v, v.tcord[1].u, v.tcord[1].v );
	}
	);
}


#ifdef USING_INT_MATRIX
DWORD LightVertInt(int norms[3])
{
	int cosT;
	int r = gRSP.iAmbientLightR;
	int g = gRSP.iAmbientLightG;
	int b = gRSP.iAmbientLightB;

	for (register unsigned int l=0; l < gRSP.numLights; l++)
	{
		cosT = norms[0]*gRSP.n64lights[l].x + norms[1]*gRSP.n64lights[l].y + norms[2]*gRSP.n64lights[l].z; 

		if (cosT > 0)
		{
			r += gRSPlights[l].r * cosT / 256;
			g += gRSPlights[l].g * cosT / 256;
			b += gRSPlights[l].b * cosT / 256;
		}
	}

	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;

	return ((0xff000000)|(((DWORD)r)<<16)|(((DWORD)g)<<8)|((DWORD)b));
}
#endif


DWORD LightVert(D3DXVECTOR4 & norm)
{
	float fCosT;

	// Do ambient
	register float r = gRSP.fAmbientLightR;
	register float g = gRSP.fAmbientLightG;
	register float b = gRSP.fAmbientLightB;


	for (register unsigned int l=0; l < gRSPnumLights; l++)
	{
		fCosT = norm.x*gRSPlights[l].x + norm.y*gRSPlights[l].y + norm.z*gRSPlights[l].z; 

		if (fCosT > 0)
		{
			r += gRSPlights[l].fr * fCosT;
			g += gRSPlights[l].fg * fCosT;
			b += gRSPlights[l].fb * fCosT;
		}
	}

	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	return ((0xff000000)|(((DWORD)r)<<16)|(((DWORD)g)<<8)|((DWORD)b));
}



float zero = 0.0f;
float fcosT;
#ifndef _XBOX
#if _MSC_VER > 1200
__m128 cosT128;
__m64 icolor64;
__m128 icolor128;
#endif
#endif

__declspec( naked ) DWORD  __fastcall SSELightVert()
{
	/*
	register __m128 color128 = _mm_set_ps( gRSP.fAmbientLightR, gRSP.fAmbientLightG, gRSP.fAmbientLightB, 0 );
	register __m128 normals = _mm_set_ps(norm.x , norm.y , norm.z , 0 );

	for( register unsigned int l=0; l < gRSP.numLights; l++ )
	{
	register __m128 lightdir = _mm_set_ps(gRSPlights[l].x, gRSPlights[l].y, gRSPlights[l].z, 0);
	cosT128 = _mm_mul_ps(lightdir,normals);
	fcosT = cosT128.m128_f32[3]+cosT128.m128_f32[2]+cosT128.m128_f32[1];
	if (fcosT > 0)
	{
	cosT128 = _mm_set_ps1(fcosT);
	register __m128 lightcolor = _mm_set_ps(gRSPlights[l].fr, gRSPlights[l].fg, gRSPlights[l].fb, 0);
	lightcolor = _mm_mul_ps(lightcolor,cosT128);
	color128 = _mm_add_ps(color128,lightcolor);
	}
	}

	color128 = _mm_min_ps(color128,_mm_set_ps1(255.0f));
	color64 = _mm_cvtps_pi16(color128);
	register DWORD finalcolor = ((0xff000000)|(color64.m64_u16[3]<<16)|(color64.m64_u16[2]<<8)|color64.m64_u16[1]);
	_mm_empty();

	//return finalcolor;
	*/
#if _MSC_VER > 1200
	__asm
	{
		movaps		xmm3, DWORD PTR gRSP;	// loading Ambient colors, xmm3 is the result color
		movaps		xmm4, DWORD PTR [g_normal];	// xmm4 is the normal

		mov			ecx, 0;
loopback:
		cmp			ecx, DWORD PTR gRSPnumLights;
		jae			breakout;
		mov			eax,ecx;
		imul		eax,0x28;
		movups		xmm5, DWORD PTR gRSPlights[eax];		// Light Dir
		movups		xmm1, DWORD PTR gRSPlights[0x18][eax];	// Light color
		mulps       xmm5, xmm4;					// Lightdir * normals

		movlhps		xmm0,xmm5;
		addps		xmm0,xmm5;
		shufps		xmm5,xmm0,0xf0;			// xmm5 3rd float = xmm0 4th float
		addps		xmm0,xmm5;
		shufps		xmm0,xmm0,0x02;			// xmm0 1st float = xmm0 3rd float

		comiss		xmm0,zero;
		jc			endloop

		shufps      xmm0,xmm0,0;					// fcosT
		mulps       xmm1,xmm0; 
		addps       xmm3,xmm1; 
endloop:
		inc			ecx;
		jmp			loopback;
breakout:

		movss		xmm0,DWORD PTR real255;
		shufps      xmm0,xmm0,0;
		minps       xmm0,xmm3;

		// Without using a memory
		cvtss2si	eax,xmm0;	// move the 1st DWORD to eax
		shl			eax,10h;
		or			eax,0FF000000h;
		shufps		xmm0,xmm0,0E5h;	// move the 2nd DWORD to the 1st DWORD
		cvtss2si	ecx,xmm0;	// move the 1st DWORD to ecx
		shl			ecx,8;
		or			eax,ecx;
		shufps		xmm0,xmm0,0E6h;	// Move the 3rd DWORD to the 1st DWORD
		cvtss2si	ecx,xmm0;
		or          eax,ecx;

		ret;
	}
#else
	__asm ret;
#endif
}

// Bits
// +-+-+-
// xxyyzz
#define Z_NEG  0x01
#define Z_POS  0x02
#define Y_NEG  0x04
#define Y_POS  0x08
#define X_NEG  0x10
#define X_POS  0x20

// Assumes dwAddress has already been checked!	
// Don't inline - it's too big with the transform macros

void SetNewVertexInfoSSE(DWORD dwAddress, DWORD dwV0, DWORD dwNum)
{
#if _MSC_VER > 1200


	// This function is called upon SPvertex
	// - do vertex matrix transform
	// - do vertex lighting
	// - do texture cooridinate transform if needed
	// - calculate normal vector

	// Output:  - g_vecProjected[i]				-> transformed vertex x,y,z
	//			- g_vecProjected[i].w						-> saved vertex 1/w
	//			- g_dwVecFlags[i]				-> flags
	//			- g_dwVecDiffuseCol[i]			-> vertex color
	//			- g_vecTexture[i]				-> vertex texture cooridinates

	StartProfiler(PROFILE_TRANSFORMATING);
	FiddledVtx * pVtxBase = (FiddledVtx*)(g_pu8RamBase + dwAddress);
	g_pVtxBase = pVtxBase;

	DWORD i;
	for (i = dwV0; i < dwV0 + dwNum; i++)
	{
		SP_Timing(DLParser_GBI0_Vtx);

		FiddledVtx & vert = pVtxBase[i - dwV0];

		g_vtxNonTransformed[i].x = (float)vert.x;
		g_vtxNonTransformed[i].y = (float)vert.y;
		g_vtxNonTransformed[i].z = (float)vert.z;

		SSEVec3Transform(i);

		if( gRSP.bFogEnabled )
		{
			/*
			if( g_vecProjected[i].w < 0 || g_vecProjected[i].z < gRSPfFogMin )
				g_fFogCoord[i] = gRSPfFogMin;
			else
				g_fFogCoord[i] = g_vecProjected[i].z;

			*(((BYTE*)&(g_dwVecDiffuseCol[i]))+3) = (BYTE)(g_fFogCoord[i]*255);	
			*/

			__asm {
				mov			eax, i;
				shl			eax, 2
				movss		xmm0, dword ptr g_vecProjected[eax*4][8];
				movss		xmm1, dword ptr g_vecProjected[eax*4][12];

				comiss		xmm1,zero;
				jc			step2;

				comiss		xmm0, dword ptr gRSPfFogMin;
				jc			step2;

				jmp			step3;
step2:
				movss		xmm0, dword ptr gRSPfFogMin;
step3:
				movss		dword ptr g_fFogCoord[eax], xmm0;
				mulss		xmm0, real255;
				cvtss2si	ecx,xmm0;	// move the 1st DWORD to ecx
				mov			byte ptr g_dwVecDiffuseCol[eax][3], cl;
			}
		}


#ifdef _DEBUG
		if( (pauseAtNext && (eventToPause == NEXT_VERTEX_CMD || eventToPause == NEXT_MATRIX_CMD || eventToPause == NEXT_FLUSH_TRI )) && logTriDetailsWithVertexMtx ) 
		{
			DWORD *dat = (DWORD*)(&vert);
			DebuggerAppendMsg("vtx %d: %08X %08X %08X %08X", i, dat[0],dat[1],dat[2],dat[3]); 
			DebuggerAppendMsg("      : %f, %f, %f, %f", 
				g_vtxTransformed[i].x,g_vtxTransformed[i].y,g_vtxTransformed[i].z,g_vtxTransformed[i].w);
			DebuggerAppendMsg("      : %f, %f, %f, %f", 
				g_vecProjected[i].x,g_vecProjected[i].y,g_vecProjected[i].z,g_vecProjected[i].w);
		}
#endif

		RSP_Vtx_Clipping(i);

		if( gRSP.bLightingEnable )
		{
			g_normal.x = (float)vert.norma.nx;
			g_normal.y = (float)vert.norma.ny;
			g_normal.z = (float)vert.norma.nz;

			SSEVec3TransformNormal();
			g_dwVecDiffuseCol[i] = SSELightVert();
			*(((BYTE*)&(g_dwVecDiffuseCol[i]))+3) = vert.rgba.a;	// still use alpha from the vertex
		}
		else
		{
			if( (gRDP.geometryMode & G_SHADE) == 0 && gRSP.ucode < 5 )	//Shade is disabled
			{
				//FLAT shade
				g_dwVecDiffuseCol[i] = gRDP.primitiveColor;
			}
			else
			{
				register IColor &color = *(IColor*)&g_dwVecDiffuseCol[i];
				color.b = vert.rgba.r;
				color.g = vert.rgba.g;
				color.r = vert.rgba.b;
				color.a = vert.rgba.a;
			}
		}

#ifdef _DEBUG
		if( options.bWinFrameMode )
		{
			g_dwVecDiffuseCol[i] = DAEDALUS_COLOR_RGBA(vert.rgba.r, vert.rgba.g, vert.rgba.b, vert.rgba.a);
		}
#endif

		// Update texture coords n.b. need to divide tu/tv by bogus scale on addition to buffer

		// If the vert is already lit, then there is no normal (and hence we
		// can't generate tex coord)
		if (gRSP.bTextureGen && gRSP.bLightingEnable )
		{
			DaedalusMatrix & matWV = gRSP.modelviewMtxs[gRSP.modelViewMtxTop];

			// Assign the spheremap's texture coordinates
			g_vecTexture[i].x = (0.5f * ( 1.0f + ( g_normal.x*matWV._11 +
				g_normal.y*matWV._21 +
				g_normal.z*matWV._31 ) ));

			g_vecTexture[i].y = (0.5f * ( 1.0f - ( g_normal.x*matWV._12 +
				g_normal.y*matWV._22 +
				g_normal.z*matWV._32 ) ));

		}
		else
		{
			g_vecTexture[i].x = (float)vert.tu;
			g_vecTexture[i].y = (float)vert.tv; 
		}
	}

	StopProfiler(PROFILE_TRANSFORMATING);

#ifdef _DEBUG
	if( (pauseAtNext && (eventToPause == NEXT_TRIANGLE || eventToPause == NEXT_FLUSH_TRI )) && logTriDetailsWithVertexMtx ) 
	{
		DebuggerAppendMsg("Setting Vertexes: %d - %d\n", dwV0, dwV0+dwNum-1);
	}
	DEBUGGER_PAUSE_AND_DUMP(NEXT_VERTEX_CMD,{TRACE0("Paused at Vertex Cmd");});
#endif
#endif
}

void SetNewVertexInfoNoSSE(DWORD dwAddress, DWORD dwV0, DWORD dwNum)
{

	// This function is called upon SPvertex
	// - do vertex matrix transform
	// - do vertex lighting
	// - do texture cooridinate transform if needed
	// - calculate normal vector

	// Output:  - g_vecProjected[i]				-> transformed vertex x,y,z
	//			- g_vecProjected[i].w						-> saved vertex 1/w
	//			- g_dwVecFlags[i]				-> flags
	//			- g_dwVecDiffuseCol[i]			-> vertex color
	//			- g_vecTexture[i]				-> vertex texture cooridinates

	StartProfiler(PROFILE_TRANSFORMATING);
	FiddledVtx * pVtxBase = (FiddledVtx*)(g_pu8RamBase + dwAddress);
	g_pVtxBase = pVtxBase;

	DWORD i;
	for (i = dwV0; i < dwV0 + dwNum; i++)
	{
		SP_Timing(DLParser_GBI0_Vtx);

		FiddledVtx & vert = pVtxBase[i - dwV0];

#ifdef USING_INT_MATRIX
		IntVec3Transform(g_vtxTransformed[i], vert, gRSPworldProjectInt);
#else
		g_vtxNonTransformed[i].x = (float)vert.x;
		g_vtxNonTransformed[i].y = (float)vert.y;
		g_vtxNonTransformed[i].z = (float)vert.z;

		D3DXVec3Transform(&g_vtxTransformed[i], (D3DXVECTOR3*)&g_vtxNonTransformed[i], &gRSPworldProject);	// Convert to w=1
#endif

		g_vecProjected[i].w = 1.0f / g_vtxTransformed[i].w;
		g_vecProjected[i].x = g_vtxTransformed[i].x * g_vecProjected[i].w;
		g_vecProjected[i].y = g_vtxTransformed[i].y * g_vecProjected[i].w;

		if( (g_curRomInfo.bPrimaryDepthHack || options.enableHackForGames == HACK_FOR_NASCAR ) && gRDP.otherMode.depth_source )
		{
			g_vecProjected[i].z = gRDP.fPrimitiveDepth;
			g_vtxTransformed[i].z = gRDP.fPrimitiveDepth*g_vtxTransformed[i].w;
		}
		else
		{
			g_vecProjected[i].z = g_vtxTransformed[i].z * g_vecProjected[i].w;
		}

		if( gRSP.bFogEnabled )
		{
			g_fFogCoord[i] = g_vecProjected[i].z;
			if( g_vecProjected[i].w < 0 || g_vecProjected[i].z < 0 || g_fFogCoord[i] < gRSPfFogMin )
				g_fFogCoord[i] = gRSPfFogMin;
		}

#ifdef _DEBUG
		if( (pauseAtNext && (eventToPause == NEXT_VERTEX_CMD || eventToPause == NEXT_MATRIX_CMD || eventToPause == NEXT_FLUSH_TRI )) && logTriDetailsWithVertexMtx ) 
		{
			DWORD *dat = (DWORD*)(&vert);
			DebuggerAppendMsg("vtx %d: %08X %08X %08X %08X", i, dat[0],dat[1],dat[2],dat[3]); 
			DebuggerAppendMsg("      : %f, %f, %f, %f", 
				g_vtxTransformed[i].x,g_vtxTransformed[i].y,g_vtxTransformed[i].z,g_vtxTransformed[i].w);
			DebuggerAppendMsg("      : %f, %f, %f, %f", 
				g_vecProjected[i].x,g_vecProjected[i].y,g_vecProjected[i].z,g_vecProjected[i].w);
		}
#endif

		RSP_Vtx_Clipping(i);

		if( gRSP.bLightingEnable )
		{
#ifdef USING_INT_MATRIX
			IntVec3TransformNormal(norms,vert.norma,gRSP.modelViewIntTop);
			extern DWORD LightVertInt(int norms[3]);
			g_dwVecDiffuseCol[i] = LightVertInt(norms);
#else
			g_normal.x = (float)vert.norma.nx;
			g_normal.y = (float)vert.norma.ny;
			g_normal.z = (float)vert.norma.nz;

			Vec3TransformNormal(g_normal, gRSPmodelViewTop);
			g_dwVecDiffuseCol[i] = LightVert(g_normal);
#endif
			*(((BYTE*)&(g_dwVecDiffuseCol[i]))+3) = vert.rgba.a;	// still use alpha from the vertex
		}
		else
		{
			if( (gRDP.geometryMode & G_SHADE) == 0 && gRSP.ucode < 5 )	//Shade is disabled
			{
				//FLAT shade
				g_dwVecDiffuseCol[i] = gRDP.primitiveColor;
			}
			else
			{
				register IColor &color = *(IColor*)&g_dwVecDiffuseCol[i];
				color.b = vert.rgba.r;
				color.g = vert.rgba.g;
				color.r = vert.rgba.b;
				color.a = vert.rgba.a;
			}
		}

#ifdef _DEBUG
		if( options.bWinFrameMode )
		{
			g_dwVecDiffuseCol[i] = DAEDALUS_COLOR_RGBA(vert.rgba.r, vert.rgba.g, vert.rgba.b, vert.rgba.a);
		}
#endif

		//if( gRDP.geometryMode & G_FOG )
		if( gRSP.bFogEnabled )
		{
			// Use fog factor to replace vertex alpha
			*(((BYTE*)&(g_dwVecDiffuseCol[i]))+3) = (BYTE)(g_fFogCoord[i]*255);	
		}

		// Update texture coords n.b. need to divide tu/tv by bogus scale on addition to buffer

		// If the vert is already lit, then there is no normal (and hence we
		// can't generate tex coord)
		if (gRSP.bTextureGen && gRSP.bLightingEnable )
		{
			DaedalusMatrix & matWV = gRSP.modelviewMtxs[gRSP.modelViewMtxTop];

			// Assign the spheremap's texture coordinates
			g_vecTexture[i].x = (0.5f * ( 1.0f + ( g_normal.x*matWV._11 +
				g_normal.y*matWV._21 +
				g_normal.z*matWV._31 ) ));

			g_vecTexture[i].y = (0.5f * ( 1.0f - ( g_normal.x*matWV._12 +
				g_normal.y*matWV._22 +
				g_normal.z*matWV._32 ) ));

		}
		else
		{
			//LONG nTU = (LONG)(SHORT)(vert.tu<<4);
			//LONG nTV = (LONG)(SHORT)(vert.tv<<4);
			//g_vecTexture[i].x = (float)(nTU>>4);
			//g_vecTexture[i].y = (float)(nTV>>4);

			g_vecTexture[i].x = (float)vert.tu;
			g_vecTexture[i].y = (float)vert.tv; 
		}
	}

	StopProfiler(PROFILE_TRANSFORMATING);

#ifdef _DEBUG
	if( (pauseAtNext && (eventToPause == NEXT_TRIANGLE || eventToPause == NEXT_FLUSH_TRI )) && logTriDetailsWithVertexMtx ) 
	{
		DebuggerAppendMsg("Setting Vertexes: %d - %d\n", dwV0, dwV0+dwNum-1);
	}
	DEBUGGER_PAUSE_AND_DUMP(NEXT_VERTEX_CMD,{TRACE0("Paused at Vertex Cmd");});
#endif
}

bool AddTri(DWORD dwV0, DWORD dwV1, DWORD dwV2)
{
	SP_Timing(SP_Each_Triangle);

	bool textureFlag = (CDaedalusRender::g_pRender->IsTextureEnabled() || gRSP.ucode == 6 );
	bool openGL = CDeviceBuilder::m_deviceGeneralType == OGL_DEVICE;

	InitVertex(g_ucVertexBuffer[gRSP.numVertices + 0], dwV0, gRSP.numVertices, textureFlag, openGL);
	InitVertex(g_ucVertexBuffer[gRSP.numVertices + 1], dwV1, gRSP.numVertices+1, textureFlag, openGL);
	InitVertex(g_ucVertexBuffer[gRSP.numVertices + 2], dwV2, gRSP.numVertices+2, textureFlag, openGL);

	/*
	if (gRDP.otherMode.zmode == 3 )//|| CDaedalusRender::g_pRender->m_dwZBias > 0 )
	{
		// Bias points by normal
		g_vtxProjected5[gRSP.numVertices+0][2] -= 0.001f*g_vtxTransformed[dwV0].w;
		g_vtxProjected5[gRSP.numVertices+1][2] -= 0.001f*g_vtxTransformed[dwV1].w;
		g_vtxProjected5[gRSP.numVertices+2][2] -= 0.001f*g_vtxTransformed[dwV2].w;
		g_ucVertexBuffer[gRSP.numVertices + 0].z -= 0.001f;
		g_ucVertexBuffer[gRSP.numVertices + 1].z -= 0.001f;
		g_ucVertexBuffer[gRSP.numVertices + 2].z -= 0.001f;
	}
	*/

	gRSP.numVertices += 3;
	status.dwNumTrisRendered++;

	return true;
}



// Returns TRUE if it thinks the triangle is visible
// Returns FALSE if it is clipped
bool TestTri(DWORD dwV0, DWORD dwV1, DWORD dwV2)
{
	//return true;	//fix me

	DEBUGGER_ONLY_IF( (!debuggerEnableTestTris || !debuggerEnableCullFace), {return TRUE;});

#ifdef _DEBUG
	// Check vertices are valid!
	if (dwV0 >= MAX_VERTS || dwV1 >= MAX_VERTS || dwV2 >= MAX_VERTS)
		return false;
#endif

	// Here we AND all the flags. If any of the bits is set for all
	// 3 vertices, it means that all three x, y or z lie outside of
	// the current viewing volume.
	// Currently disabled - still seems a bit dodgy
	if ((gRSP.bCullFront || gRSP.bCullBack) && gRDP.otherMode.zmode != 3)
	{
		D3DXVECTOR4 & v0 = g_vecProjected[dwV0];
		D3DXVECTOR4 & v1 = g_vecProjected[dwV1];
		D3DXVECTOR4 & v2 = g_vecProjected[dwV2];

		// Only try to clip if the tri is onscreen. For some reason, this
		// method doesnt' work well when the z value is outside of screenspace
		if (v0.z < 1 && v1.z < 1 && v2.z < 1)
		{
			float V1 = v2.x - v0.x;
			float V2 = v2.y - v0.y;
			
			float W1 = v2.x - v1.x;
			float W2 = v2.y - v1.y;

			float fDirection = (V1 * W2) - (V2 * W1);
			//float fDirection = v0.x*v1.y-v1.x*v0.y+v1.x*v2.y-v2.x*v1.y+v2.x*v0.y-v0.x*v2.y;
			/*
			*/

			if (fDirection < 0 && gRSP.bCullBack)
			{
				status.dwNumTrisClipped++;
				return false;
			}
			else if (fDirection > 0 && gRSP.bCullFront)
			{
				status.dwNumTrisClipped++;
				return false;
			}
		}
	}
	
#ifdef ENABLE_CLIP_TRI
	//if( gRSP.bRejectVtx && (g_clipFlag[dwV0]|g_clipFlag[dwV1]|g_clipFlag[dwV2]) )	
	//	return;
	if( g_clipFlag2[dwV0]&g_clipFlag2[dwV1]&g_clipFlag2[dwV2] )
	{
		//DebuggerAppendMsg("Clipped");
		return false;
	}
#endif

	return true;
}


void SetPrimitiveColor(DWORD dwCol, DWORD LODMin, DWORD LODFrac)
{
	gRDP.primitiveColor = dwCol;
	gRDP.primLODMin = LODMin;
	gRDP.primLODFrac = LODFrac;
	if( gRDP.primLODFrac < gRDP.primLODMin )
	{
		gRDP.primLODFrac = gRDP.primLODMin;
	}

	gRDP.fvPrimitiveColor[0] = ((dwCol>>16)&0xFF)/255.0f;		//r
	gRDP.fvPrimitiveColor[1] = ((dwCol>>8)&0xFF)/255.0f;		//g
	gRDP.fvPrimitiveColor[2] = ((dwCol)&0xFF)/255.0f;			//b
	gRDP.fvPrimitiveColor[3] = ((dwCol>>24)&0xFF)/255.0f;		//a
}

void SetPrimitiveDepth(DWORD z, DWORD dwDZ)
{
	gRDP.primitiveDepth = z&0x7FFF;
	//if( gRDP.primitiveDepth != 0 && gRDP.primitiveDepth != 0xFFFF ) 
	{
		gRDP.fPrimitiveDepth = (float)(gRDP.primitiveDepth)/(float)0x8000;

		//gRDP.fPrimitiveDepth = gRDP.fPrimitiveDepth*2-1;	
		/*
		z=0xFFFF	->	1	the farest
		z=0			->	-1	the nearest
		*/
	}

	//how to use dwDZ?

#ifdef _DEBUG
	if( (pauseAtNext && (eventToPause == NEXT_VERTEX_CMD || eventToPause == NEXT_FLUSH_TRI )) )//&& logTriDetails ) 
	{
		DebuggerAppendMsg("Set prim Depth: %f, (%08X, %08X)", gRDP.fPrimitiveDepth, z, dwDZ); 
	}
#endif
}

void SetVertexXYZ(DWORD vertex, float x, float y, float z)
{
	g_vecProjected[vertex].x = x;
	g_vecProjected[vertex].y = y;
	g_vecProjected[vertex].z = z;

	g_vecProjected[vertex].w = 1/z;
}

extern DWORD dwTvSystem;
void ModifyVertexInfo(DWORD where, DWORD vertex, DWORD val)
{
	switch (where)
	{
	case G_MWO_POINT_RGBA:		// Modify RGBA
		{
			DWORD r = (val>>24)&0xFF;
			DWORD g = (val>>16)&0xFF;
			DWORD b = (val>>8)&0xFF;
			DWORD a = val&0xFF;
			g_dwVecDiffuseCol[vertex] = DAEDALUS_COLOR_RGBA(r, g, b, a);
			DL_PF("Modify vert %d color, 0x%08x", vertex, g_dwVecDiffuseCol[vertex]);
		}
		break;
	case G_MWO_POINT_XYSCREEN:		// Modify X,Y
		{
			u16 nX = (u16)(val>>16);
			s16 x = *((s16*)&nX);
			x /= 4;

			u16 nY = u16(val&0xFFFF);
			s16 y = *((s16*)&nY);
			y /= 4;

			// Should do viewport transform.


			x -= windowSetting.uViWidth/2;
			y = windowSetting.uViHeight/2-y;

			if( options.enableHacks && ((*g_GraphicsInfo.VI_X_SCALE_RG)&0xF) != 0 )
			{
				// Tarzan
				// I don't know why Tarzan is different
				SetVertexXYZ(vertex, x/windowSetting.fViWidth, y/windowSetting.fViHeight, g_vecProjected[vertex].z);
			}
			else
			{
				// Toy Story 2 and other games
				SetVertexXYZ(vertex, x*2/windowSetting.fViWidth, y*2/windowSetting.fViHeight, g_vecProjected[vertex].z);
			}

			DL_PF("Modify vert %d: x=%d, y=%d", vertex, x, y);
			DEBUGGER_IF_DUMP( (logTriDetailsWithVertexMtx||(pauseAtNext && eventToPause==NEXT_VERTEX_CMD)), 
				{DebuggerAppendMsg("Modify vert %d: (%d,%d)", vertex, x, y);});
		}
		break;
	case G_MWO_POINT_ZSCREEN:		// Modify C
		{
			int z = val>>16;

			SetVertexXYZ(vertex, g_vecProjected[vertex].x, g_vecProjected[vertex].y, (((float)z/0x03FF)+0.5f)/2.0f );
			DL_PF("Modify vert %d: z=%d", vertex, z);
			DEBUGGER_IF_DUMP( (logTriDetailsWithVertexMtx||(pauseAtNext && eventToPause==NEXT_VERTEX_CMD)), 
				{DebuggerAppendMsg("Modify vert %d: z=%d", vertex, z);});
		}
		break;
	case G_MWO_POINT_ST:		// Texture
		{
			short tu = short(val>>16);
			short tv = short(val & 0xFFFF);
			float ftu = tu / 32.0f;
			float ftv = tv / 32.0f;
			DL_PF("      Setting vertex %d tu/tv to %f, %f", vertex, (float)tu, (float)tv);
			CDaedalusRender::g_pRender->SetVtxTextureCoord(vertex, ftu/gRSP.fTexScaleX, ftv/gRSP.fTexScaleY);
		}
		break;
	}
	DEBUGGER_PAUSE_AND_DUMP(NEXT_VERTEX_CMD,{TRACE0("Paused at ModVertex Cmd");});
}

void SetNewVertexInfoDKR(DWORD dwAddress, DWORD dwV0, DWORD dwNum)
{
	uint32 pVtxBase = uint32(g_pu8RamBase + dwAddress);
	g_pVtxBase = (FiddledVtx*)pVtxBase;

	DaedalusMatrix &matWorldProject = gRSP.DKRMatrixes[gRSP.DKRCMatrixIndex];

	DWORD i;
	LONG nOff;

	bool addbase=false;
	if ((!gRSP.DKRVtxAddBase) || (gRSP.DKRCMatrixIndex != 2) )
		addbase = false;
	else
		addbase = true;

	if( addbase && gRSP.DKRVtxCount == 0 && dwNum > 1 )
	{
		gRSP.DKRVtxCount++;
	}

	DL_PF("    SetNewVertexInfoDKR, CMatrix = %d, Add base=%s", gRSP.DKRCMatrixIndex, gRSP.DKRVtxAddBase?"true":"false");
#ifdef _DEBUG
	if( (pauseAtNext && (eventToPause == NEXT_TRIANGLE || eventToPause == NEXT_MATRIX_CMD || eventToPause == NEXT_FLUSH_TRI )) && logTriDetailsWithVertexMtx ) 
	{
		TRACE0("DKR Setting Vertexes\n");
		DebuggerAppendMsg("CMatrix = %d, Add base=%s", gRSP.DKRCMatrixIndex, gRSP.DKRVtxAddBase?"true":"false");
	}
#endif

	nOff = 0;
	DWORD end = dwV0 + dwNum;
	for (i = dwV0; i < end; i++)
	{
		D3DXVECTOR3 w;

		g_vtxNonTransformed[i].x = (float)*(s16*)((pVtxBase+nOff + 0) ^ 2);
		g_vtxNonTransformed[i].y = (float)*(s16*)((pVtxBase+nOff + 2) ^ 2);
		g_vtxNonTransformed[i].z = (float)*(s16*)((pVtxBase+nOff + 4) ^ 2);

		//if( status.isSSEEnabled )
		//	SSEVec3TransformDKR(g_vtxTransformed[i], g_vtxNonTransformed[i]);
		//else
			D3DXVec3Transform(&g_vtxTransformed[i], (D3DXVECTOR3*)&g_vtxNonTransformed[i], &matWorldProject);	// Convert to w=1

		if( gRSP.DKRVtxCount == 0 && dwNum==1 )
		{
			gRSP.DKRBaseVec.x = g_vtxTransformed[i].x;
			gRSP.DKRBaseVec.y = g_vtxTransformed[i].y;
			gRSP.DKRBaseVec.z = g_vtxTransformed[i].z;
			gRSP.DKRBaseVec.w = g_vtxTransformed[i].w;
		}
		else if( addbase )
		{
			g_vtxTransformed[i].x += gRSP.DKRBaseVec.x;
			g_vtxTransformed[i].y += gRSP.DKRBaseVec.y;
			g_vtxTransformed[i].z += gRSP.DKRBaseVec.z;
			g_vtxTransformed[i].w  = gRSP.DKRBaseVec.w;
		}

		g_vecProjected[i].w = 1.0f / g_vtxTransformed[i].w;
		g_vecProjected[i].x = g_vtxTransformed[i].x * g_vecProjected[i].w;
		g_vecProjected[i].y = g_vtxTransformed[i].y * g_vecProjected[i].w;
		g_vecProjected[i].z = g_vtxTransformed[i].z * g_vecProjected[i].w;

		gRSP.DKRVtxCount++;

#ifdef _DEBUG
		if( (pauseAtNext && (eventToPause == NEXT_TRIANGLE || eventToPause == NEXT_MATRIX_CMD || eventToPause == NEXT_FLUSH_TRI )) && logTriDetailsWithVertexMtx ) 
		{
			DebuggerAppendMsg("vtx %d: %f, %f, %f, %f", i, 
				g_vtxTransformed[i].x,g_vtxTransformed[i].y,g_vtxTransformed[i].z,g_vtxTransformed[i].w);
		}
#endif

		if( gRSP.bFogEnabled )
		{
			g_fFogCoord[i] = g_vecProjected[i].z;
			if( g_vecProjected[i].w < 0 || g_vecProjected[i].z < 0 || g_fFogCoord[i] < gRSPfFogMin )
				g_fFogCoord[i] = gRSPfFogMin;
		}

		RSP_Vtx_Clipping(i);

		s16 wA = *(s16*)((pVtxBase+nOff + 6) ^ 2);
		s16 wB = *(s16*)((pVtxBase+nOff + 8) ^ 2);

		s8 r = (s8)(wA >> 8);
		s8 g = (s8)(wA);
		s8 b = (s8)(wB >> 8);
		s8 a = (s8)(wB);

		if (gRSP.bLightingEnable)
		{
			g_normal.x = (float)r; //norma.nx;
			g_normal.y = (float)g; //norma.ny;
			g_normal.z = (float)b; //norma.nz;

			Vec3TransformNormal(g_normal, matWorldProject)
			if( status.isSSESupported )
				g_dwVecDiffuseCol[i] = SSELightVert();
			else
				g_dwVecDiffuseCol[i] = LightVert(g_normal);
		}
		else
		{
			LONG nR, nG, nB, nA;

			nR = r;
			nG = g;
			nB = b;
			nA = a;
			// Assign true vert colour after lighting/fogging
			g_dwVecDiffuseCol[i] = DAEDALUS_COLOR_RGBA(nR, nG, nB, nA);
		}

		if( gRDP.geometryMode & G_FOG )
		{
			// Use fog factor to replace vertex alpha
			*(((BYTE*)&(g_dwVecDiffuseCol[i]))+3) = (BYTE)(g_fFogCoord[i]*255);	
		}

		g_vecTexture[i].x = g_vecTexture[i].y = 1;

		nOff += 10;
	}


	DEBUGGER_PAUSE_AND_DUMP(NEXT_VERTEX_CMD,{DebuggerAppendMsg("Paused at DKR Vertex Cmd, v0=%d, vn=%d, addr=%08X", dwV0, dwNum, dwAddress);});
}

extern DWORD dwPDCIAddr;
void SetNewVertexInfoPD(DWORD dwAddress, DWORD dwV0, DWORD dwNum)
{
	StartProfiler(PROFILE_TRANSFORMATING);
	N64VtxPD * pVtxBase = (N64VtxPD*)(g_pu8RamBase + dwAddress);
	g_pVtxBase = (FiddledVtx*)pVtxBase;	// Fix me

	for (u32 i = dwV0; i < dwV0 + dwNum; i++)
	{
		N64VtxPD &vert = pVtxBase[i - dwV0];

		g_vtxNonTransformed[i].x = (float)vert.x;
		g_vtxNonTransformed[i].y = (float)vert.y;
		g_vtxNonTransformed[i].z = (float)vert.z;

		if( status.isSSEEnabled )
			SSEVec3Transform(i);
		else
		{
			D3DXVec3Transform(&g_vtxTransformed[i], (D3DXVECTOR3*)&g_vtxNonTransformed[i], &gRSPworldProject);	// Convert to w=1
			g_vecProjected[i].w = 1.0f / g_vtxTransformed[i].w;
			g_vecProjected[i].x = g_vtxTransformed[i].x * g_vecProjected[i].w;
			g_vecProjected[i].y = g_vtxTransformed[i].y * g_vecProjected[i].w;
			g_vecProjected[i].z = g_vtxTransformed[i].z * g_vecProjected[i].w;
		}

		g_fFogCoord[i] = g_vecProjected[i].z;
		if( g_vecProjected[i].w < 0 || g_vecProjected[i].z < 0 || g_fFogCoord[i] < gRSPfFogMin )
			g_fFogCoord[i] = gRSPfFogMin;

		RSP_Vtx_Clipping(i);

		BYTE *addr = g_pu8RamBase+dwPDCIAddr+vert.cidx;

		// PD hack by oDD to fix colours
		DWORD a = addr[0];
		DWORD avg = (addr[1] + addr[2] + addr[3]) / 3;
		DWORD r = avg;
		DWORD g = avg;
		DWORD b = avg;

		/*DWORD a = addr[0];
		DWORD r = addr[1];
		DWORD g = addr[2];
		DWORD b = addr[3];*/

		if( gRSP.bLightingEnable )
		{
			g_normal.x = (float)r;
			g_normal.y = (float)g;
			g_normal.z = (float)a;

			if( status.isSSESupported )
			{
				SSEVec3TransformNormal();
				g_dwVecDiffuseCol[i] = SSELightVert();
			}
			else
			{
				Vec3TransformNormal(g_normal, gRSPmodelViewTop);
				g_dwVecDiffuseCol[i] = LightVert(g_normal);
			}
			*(((BYTE*)&(g_dwVecDiffuseCol[i]))+3) = (BYTE)a;	// still use alpha from the vertex
		}
		else
		{
			if( (gRDP.geometryMode & G_SHADE) == 0 && gRSP.ucode < 5 )	//Shade is disabled
			{
				g_dwVecDiffuseCol[i] = gRDP.primitiveColor;
			}
			else	//FLAT shade
			{
				g_dwVecDiffuseCol[i] = DAEDALUS_COLOR_RGBA(r, g, b, a);
			}
		}

#ifdef _DEBUG
		if( options.bWinFrameMode )
		{
			g_dwVecDiffuseCol[i] = DAEDALUS_COLOR_RGBA(r, g, b, a);
		}
#endif

		if( gRDP.geometryMode & G_FOG )
		{
			// Use fog factor to replace vertex alpha
			*(((BYTE*)&(g_dwVecDiffuseCol[i]))+3) = (BYTE)(g_fFogCoord[i]*255);	
		}

		VECTOR2 & t = g_vecTexture[i];
		/*
		if (gRSP.bTextureGen && gRSP.bLightingEnable && g_textures[gRSP.curTile].m_bTextureEnable )
		{
			DaedalusMatrix & matWV = gRSP.modelviewMtxs[gRSP.modelViewMtxTop];

			// Assign the spheremap's texture coordinates
			t.x = (0.5f * ( 1.0f + ( g_normal.x*matWV._11 +
				g_normal.y*matWV._21 +
				g_normal.z*matWV._31 ) ));

			t.y = (0.5f * ( 1.0f - ( g_normal.x*matWV._12 +
				g_normal.y*matWV._22 +
				g_normal.z*matWV._32 ) ));

		}
		else
		*/
		{
			t.x = vert.s;
			t.y = vert.t; 
		}
	}

	StopProfiler(PROFILE_TRANSFORMATING);

#ifdef _DEBUG
	if( (pauseAtNext && (eventToPause == NEXT_TRIANGLE || eventToPause == NEXT_FLUSH_TRI )) && logTriDetailsWithVertexMtx ) 
	{
		DebuggerAppendMsg("Setting Vertexes: %d - %d\n", dwV0, dwV0+dwNum-1);
	}
	DEBUGGER_PAUSE_AND_DUMP(NEXT_VERTEX_CMD,{TRACE0("Paused at Vertex Cmd");});
#endif
}

extern DWORD dwConkerVtxZAddr;
void SetNewVertexInfoConker(DWORD dwAddress, DWORD dwV0, DWORD dwNum)
{
	StartProfiler(PROFILE_TRANSFORMATING);
	FiddledVtx * pVtxBase = (FiddledVtx*)(g_pu8RamBase + dwAddress);
	g_pVtxBase = pVtxBase;
	s16 *vertexColoraddr = (s16*)(g_pu8RamBase+dwConkerVtxZAddr);

	DWORD i;
	for (i = dwV0; i < dwV0 + dwNum; i++)
	{
		SP_Timing(DLParser_GBI0_Vtx);

		FiddledVtx & vert = pVtxBase[i - dwV0];

		g_vtxNonTransformed[i].x = (float)vert.x;
		g_vtxNonTransformed[i].y = (float)vert.y;
		g_vtxNonTransformed[i].z = (float)vert.z;

		if( status.isSSEEnabled )
			SSEVec3Transform(i);
		else
		{
			D3DXVec3Transform(&g_vtxTransformed[i], (D3DXVECTOR3*)&g_vtxNonTransformed[i], &gRSPworldProject);	// Convert to w=1
			g_vecProjected[i].w = 1.0f / g_vtxTransformed[i].w;
			g_vecProjected[i].x = g_vtxTransformed[i].x * g_vecProjected[i].w;
			g_vecProjected[i].y = g_vtxTransformed[i].y * g_vecProjected[i].w;
			g_vecProjected[i].z = g_vtxTransformed[i].z * g_vecProjected[i].w;
		}

		g_fFogCoord[i] = g_vecProjected[i].z;
		if( g_vecProjected[i].w < 0 || g_vecProjected[i].z < 0 || g_fFogCoord[i] < gRSPfFogMin )
			g_fFogCoord[i] = gRSPfFogMin;

#ifdef _DEBUG
		if( (pauseAtNext && (eventToPause == NEXT_VERTEX_CMD || eventToPause == NEXT_MATRIX_CMD || eventToPause == NEXT_FLUSH_TRI )) && logTriDetailsWithVertexMtx ) 
		{
			DWORD *dat = (DWORD*)(&vert);
			DebuggerAppendMsg("vtx %d: %08X %08X %08X %08X", i, dat[0],dat[1],dat[2],dat[3]); 
			DebuggerAppendMsg("      : %f, %f, %f, %f", 
				g_vtxTransformed[i].x,g_vtxTransformed[i].y,g_vtxTransformed[i].z,g_vtxTransformed[i].w);
			DebuggerAppendMsg("      : %f, %f, %f, %f", 
				g_vecProjected[i].x,g_vecProjected[i].y,g_vecProjected[i].z,g_vecProjected[i].w);
		}
#endif

		RSP_Vtx_Clipping(i);

		if( gRSP.bLightingEnable )
		{
			//if( gRSPnumLights == 1 )
			{
				DWORD r= ((gRSP.ambientLightColor>>16)&0xFF);
				DWORD g= ((gRSP.ambientLightColor>> 8)&0xFF);
				DWORD b= ((gRSP.ambientLightColor    )&0xFF);
				for( int k=1; k<=gRSPnumLights; k++)
				{
					r += gRSPlights[k].r;
					g += gRSPlights[k].g;
					b += gRSPlights[k].b;
				}
				if( r>255 ) r=255;
				if( g>255 ) g=255;
				if( b>255 ) b=255;
				r *= vert.rgba.r ;
				g *= vert.rgba.g ;
				b *= vert.rgba.b ;
				r >>= 8;
				g >>= 8;
				b >>= 8;
				g_dwVecDiffuseCol[i] = 0xFF000000;
				g_dwVecDiffuseCol[i] |= (r<<16);
				g_dwVecDiffuseCol[i] |= (g<< 8);
				g_dwVecDiffuseCol[i] |= (b    );			
			}
			/*
			else
			{
				g_normal.x = (float)vert.norma.nx;
				g_normal.y = (float)vert.norma.ny;
				g_normal.z = (float)vert.norma.nz;

				if( status.isSSESupported )
				{
					SSEVec3TransformNormal();
					g_dwVecDiffuseCol[i] = SSELightVert();
				}
				else
				{
					Vec3TransformNormal(g_normal, gRSPmodelViewTop);
					g_dwVecDiffuseCol[i] = LightVert(g_normal);
				}
			}
			*/


			*(((BYTE*)&(g_dwVecDiffuseCol[i]))+3) = vert.rgba.a;	// still use alpha from the vertex
		}
		else
		{
			if( (gRDP.geometryMode & G_SHADE) == 0 && gRSP.ucode < 5 )	//Shade is disabled
			{
				g_dwVecDiffuseCol[i] = gRDP.primitiveColor;
			}
			else	//FLAT shade
			{
				g_dwVecDiffuseCol[i] = DAEDALUS_COLOR_RGBA(vert.rgba.r, vert.rgba.g, vert.rgba.b, vert.rgba.a);
			}
		}

#ifdef _DEBUG
		if( options.bWinFrameMode )
		{
			//g_vecProjected[i].z = 0;
			g_dwVecDiffuseCol[i] = DAEDALUS_COLOR_RGBA(vert.rgba.r, vert.rgba.g, vert.rgba.b, vert.rgba.a);
		}
#endif

		if( gRDP.geometryMode & G_FOG )
		{
			// Use fog factor to replace vertex alpha
			*(((BYTE*)&(g_dwVecDiffuseCol[i]))+3) = (BYTE)(g_fFogCoord[i]*255);	
		}

		// Update texture coords n.b. need to divide tu/tv by bogus scale on addition to buffer
		VECTOR2 & t = g_vecTexture[i];

		// If the vert is already lit, then there is no normal (and hence we
		// can't generate tex coord)
		if (gRSP.bTextureGen && gRSP.bLightingEnable )
		{
			DaedalusMatrix & matWV = gRSP.modelviewMtxs[gRSP.modelViewMtxTop];

			// Assign the spheremap's texture coordinates
			g_vecTexture[i].x = (0.5f * ( 1.0f + ( g_normal.x*matWV._11 +
				g_normal.y*matWV._21 +
				g_normal.z*matWV._31 ) ));

			g_vecTexture[i].y = (0.5f * ( 1.0f - ( g_normal.x*matWV._12 +
				g_normal.y*matWV._22 +
				g_normal.z*matWV._32 ) ));

		}
		else
		{
			g_vecTexture[i].x = (float)vert.tu;
			g_vecTexture[i].y = (float)vert.tv; 
		}
	}

	StopProfiler(PROFILE_TRANSFORMATING);

#ifdef _DEBUG
	if( (pauseAtNext && (eventToPause == NEXT_TRIANGLE || eventToPause == NEXT_FLUSH_TRI )) && logTriDetailsWithVertexMtx ) 
	{
		DebuggerAppendMsg("Setting Vertexes: %d - %d\n", dwV0, dwV0+dwNum-1);
	}
	DEBUGGER_PAUSE_AND_DUMP(NEXT_VERTEX_CMD,{DebuggerAppendMsg("Paused at Vertex Cmd");});
#endif
}


typedef struct{
	s16 y;
	s16 x;
	s16 flag;
	s16 z;
} RS_Vtx_XYZ;

typedef union {
	struct {
		BYTE a;
		BYTE b;
		BYTE g;
		BYTE r;
	};
	struct {
		char na;
		char nz;	//b
		char ny;	//g
		char nx;	//r
	};
} RS_Vtx_Color;


void SetNewVertexInfo_Rogue_Squadron(DWORD dwXYZAddr, DWORD dwColorAddr, DWORD dwXYZCmd, DWORD dwColorCmd)
{
	u32 dwV0 = 0;
	u32 dwNum = (dwXYZCmd&0xFF00)>>10;

	RS_Vtx_XYZ * pVtxXYZBase = (RS_Vtx_XYZ*)(g_pu8RamBase + dwXYZAddr);
	RS_Vtx_Color * pVtxColorBase = (RS_Vtx_Color*)(g_pu8RamBase + dwColorAddr);

	DWORD i;
	for (i = dwV0; i < dwV0 + dwNum; i++)
	{
		RS_Vtx_XYZ & vertxyz = pVtxXYZBase[i - dwV0];
		RS_Vtx_Color & vertcolors = pVtxColorBase[i - dwV0];

		g_vtxNonTransformed[i].x = (float)vertxyz.x;
		g_vtxNonTransformed[i].y = (float)vertxyz.y;
		g_vtxNonTransformed[i].z = (float)vertxyz.z;

		if( status.isSSEEnabled )
			SSEVec3Transform(i);
		else
		{
			D3DXVec3Transform(&g_vtxTransformed[i], (D3DXVECTOR3*)&g_vtxNonTransformed[i], &gRSPworldProject);	// Convert to w=1
			g_vecProjected[i].w = 1.0f / g_vtxTransformed[i].w;
			g_vecProjected[i].x = g_vtxTransformed[i].x * g_vecProjected[i].w;
			g_vecProjected[i].y = g_vtxTransformed[i].y * g_vecProjected[i].w;
			g_vecProjected[i].z = g_vtxTransformed[i].z * g_vecProjected[i].w;
		}

		g_fFogCoord[i] = g_vecProjected[i].z;
		if( g_vecProjected[i].w < 0 || g_vecProjected[i].z < 0 || g_fFogCoord[i] < gRSPfFogMin )
			g_fFogCoord[i] = gRSPfFogMin;

#ifdef _DEBUG
		if( (pauseAtNext && (eventToPause == NEXT_VERTEX_CMD || eventToPause == NEXT_MATRIX_CMD || eventToPause == NEXT_FLUSH_TRI )) && logTriDetailsWithVertexMtx ) 
		{
			DebuggerAppendMsg("      : %f, %f, %f, %f", 
				g_vtxTransformed[i].x,g_vtxTransformed[i].y,g_vtxTransformed[i].z,g_vtxTransformed[i].w);
			DebuggerAppendMsg("      : %f, %f, %f, %f", 
				g_vecProjected[i].x,g_vecProjected[i].y,g_vecProjected[i].z,g_vecProjected[i].w);
		}
#endif

		RSP_Vtx_Clipping(i);

		if( gRSP.bLightingEnable )
		{
			g_normal.x = (float)vertcolors.nx;
			g_normal.y = (float)vertcolors.ny;
			g_normal.z = (float)vertcolors.nz;

			if( status.isSSESupported )
			{
				SSEVec3TransformNormal();
				g_dwVecDiffuseCol[i] = SSELightVert();
			}
			else
			{
				Vec3TransformNormal(g_normal, gRSPmodelViewTop);
				g_dwVecDiffuseCol[i] = LightVert(g_normal);
			}
			*(((BYTE*)&(g_dwVecDiffuseCol[i]))+3) = vertcolors.a;	// still use alpha from the vertex
		}
		else
		{
			if( (gRDP.geometryMode & G_SHADE) == 0 && gRSP.ucode < 5 )	//Shade is disabled
			{
				g_dwVecDiffuseCol[i] = gRDP.primitiveColor;
			}
			else	//FLAT shade
			{
				g_dwVecDiffuseCol[i] = DAEDALUS_COLOR_RGBA(vertcolors.r, vertcolors.g, vertcolors.b, vertcolors.a);
			}
		}

#ifdef _DEBUG
		if( options.bWinFrameMode )
		{
			g_dwVecDiffuseCol[i] = DAEDALUS_COLOR_RGBA(vertcolors.r, vertcolors.g, vertcolors.b, vertcolors.a);
		}
#endif

		if( gRDP.geometryMode & G_FOG )
		{
			// Use fog factor to replace vertex alpha
			*(((BYTE*)&(g_dwVecDiffuseCol[i]))+3) = (BYTE)(g_fFogCoord[i]*255);	
		}

		/*
		// Update texture coords n.b. need to divide tu/tv by bogus scale on addition to buffer
		VECTOR2 & t = g_vecTexture[i];

		// If the vert is already lit, then there is no normal (and hence we
		// can't generate tex coord)
		if (gRSP.bTextureGen && gRSP.bLightingEnable && g_textures[gRSP.curTile].m_bTextureEnable )
		{
			DaedalusMatrix & matWV = gRSP.modelviewMtxs[gRSP.modelViewMtxTop];

			// Assign the spheremap's texture coordinates
			t.x = (0.5f * ( 1.0f + ( g_normal.x*matWV._11 +
				g_normal.y*matWV._21 +
				g_normal.z*matWV._31 ) ));

			t.y = (0.5f * ( 1.0f - ( g_normal.x*matWV._12 +
				g_normal.y*matWV._22 +
				g_normal.z*matWV._32 ) ));

		}
		else
		{
			t.x = (float)vert.tu;
			t.y = (float)vert.tv; 
		}
		*/
	}

	StopProfiler(PROFILE_TRANSFORMATING);

#ifdef _DEBUG
	if( (pauseAtNext && (eventToPause == NEXT_TRIANGLE || eventToPause == NEXT_FLUSH_TRI )) && logTriDetailsWithVertexMtx ) 
	{
		DebuggerAppendMsg("Setting Vertexes: %d - %d\n", dwV0, dwV0+dwNum-1);
	}
	DEBUGGER_PAUSE_AND_DUMP(NEXT_VERTEX_CMD,{TRACE0("Paused at Vertex Cmd");});
#endif
}

void SetLightCol(DWORD dwLight, DWORD dwCol)
{
	gRSPlights[dwLight].r = (BYTE)((dwCol >> 24)&0xFF);
	gRSPlights[dwLight].g = (BYTE)((dwCol >> 16)&0xFF);
	gRSPlights[dwLight].b = (BYTE)((dwCol >>  8)&0xFF);
	gRSPlights[dwLight].a = 255;	// Ignore light alpha
	gRSPlights[dwLight].fr = (float)gRSPlights[dwLight].r;
	gRSPlights[dwLight].fg = (float)gRSPlights[dwLight].g;
	gRSPlights[dwLight].fb = (float)gRSPlights[dwLight].b;
	gRSPlights[dwLight].fa = 255;	// Ignore light alpha
}

void SetLightDirection(DWORD dwLight, float x, float y, float z)
{
	register float w = (float)sqrt(x*x+y*y+z*z);

	gRSPlights[dwLight].x = x/w;
	gRSPlights[dwLight].y = y/w;
	gRSPlights[dwLight].z = z/w;
}

static float maxS0, maxT0;
static float maxS1, maxT1;
static bool validS0, validT0;
static bool validS1, validT1;

void LogTextureCoords(float fTex0S, float fTex0T, float fTex1S, float fTex1T)
{
	if( validS0 )
	{
		if( fTex0S<0 || fTex0S>maxS0 )	validS0 = false;
	}
	if( validT0 )
	{
		if( fTex0T<0 || fTex0T>maxT0 )	validT0 = false;
	}
	if( validS1 )
	{
		if( fTex1S<0 || fTex1S>maxS1 )	validS1 = false;
	}
	if( validT1 )
	{
		if( fTex1T<0 || fTex1T>maxT1 )	validT1 = false;
	}
}

bool CheckTextureCoords(int tex)
{
	if( tex==0 )
	{
		return validS0&&validT0;
	}
	else
	{
		return validS1&&validT1;
	}
}

void ResetTextureCoordsLog(float maxs0, float maxt0, float maxs1, float maxt1)
{
	maxS0 = maxs0;
	maxT0 = maxt0;
	maxS1 = maxs1;
	maxT1 = maxt1;
	validS0 = validT0 = true;
	validS1 = validT1 = true;
}

void ForceMainTextureIndex(int dwTile) 
{
	if( dwTile == 1 && !(CDaedalusRender::g_pRender->IsTexel0Enable()) && CDaedalusRender::g_pRender->IsTexel1Enable() )
	{
		// Hack
		gRSP.curTile = 0;
	}
	else
		gRSP.curTile = dwTile;
}


#ifdef USING_INT_MATRIX
// N64 integer matrix class


N64IntMatrix& N64IntMatrix::operator *= ( const N64IntMatrix& mtx )
{
	int old_nums[16], old_decs[16];
	register int i,j;
	for(i=0; i<16; i++)
	{
		old_nums[i] = nums[i];
		old_nums[i] = decs[i];
	}

	for( i=0; i<4; i++ )
	{
		for( j=0; j<4; j++ )
		{
			nums[index(i,j)]  = old_nums[index(i,0)]*mtx.nums[index(0,j)]
							  + old_nums[index(i,1)]*mtx.nums[index(1,j)]
							  + old_nums[index(i,2)]*mtx.nums[index(2,j)]
							  + old_nums[index(i,3)]*mtx.nums[index(3,j)];
			nums[index(i,j)] += (old_nums[index(i,0)]*mtx.decs[index(0,j)]
							  + old_nums[index(i,1)]*mtx.decs[index(1,j)]
							  + old_nums[index(i,2)]*mtx.decs[index(2,j)]
							  + old_nums[index(i,3)]*mtx.decs[index(3,j)])>>16;
			nums[index(i,j)] += (old_decs[index(i,0)]*mtx.nums[index(0,j)]
							  + old_decs[index(i,1)]*mtx.nums[index(1,j)]
							  + old_decs[index(i,2)]*mtx.nums[index(2,j)]
							  + old_decs[index(i,3)]*mtx.nums[index(3,j)])>>16;

			decs[index(i,j)]  = (old_nums[index(i,0)]*mtx.decs[index(0,j)]
							  + old_nums[index(i,1)]*mtx.decs[index(1,j)]
							  + old_nums[index(i,2)]*mtx.decs[index(2,j)]
							  + old_nums[index(i,3)]*mtx.decs[index(3,j)])%65536;
			decs[index(i,j)] += (old_decs[index(i,0)]*mtx.nums[index(0,j)]
							  + old_decs[index(i,1)]*mtx.nums[index(1,j)]
							  + old_decs[index(i,2)]*mtx.nums[index(2,j)]
							  + old_decs[index(i,3)]*mtx.nums[index(3,j)])%65536;
			decs[index(i,j)] += (old_decs[index(i,0)]*mtx.decs[index(0,j)]
							  + old_decs[index(i,1)]*mtx.decs[index(1,j)]
							  + old_decs[index(i,2)]*mtx.decs[index(2,j)]
							  + old_decs[index(i,3)]*mtx.decs[index(3,j)]);
		}
	}

	return *this;
}

N64IntMatrix N64IntMatrix::operator * ( const N64IntMatrix& mtx ) const
{
	//DaedalusMatrix a = this->convert();
	//DaedalusMatrix b = mtx.convert();
	//DaedalusMatrix c = a*b;

	N64IntMatrix newMtx;
	register i,j;
	int res0;
	int res1;
	int res2;
	u32 res3;
	int dec;

	//float org, res, dif;

	for( i=0; i<4; i++ )
	{
		for( j=0; j<4; j++ )
		{
			res0	=	nums[i*4+0]*mtx.nums[0*4+j]
					+	nums[i*4+1]*mtx.nums[1*4+j]
					+	nums[i*4+2]*mtx.nums[2*4+j]
					+	nums[i*4+3]*mtx.nums[3*4+j];
			res1	=	decs[i*4+0]*mtx.nums[0*4+j]
					+	decs[i*4+1]*mtx.nums[1*4+j]
					+	decs[i*4+2]*mtx.nums[2*4+j]
					+	decs[i*4+3]*mtx.nums[3*4+j];
			res2	=	nums[i*4+0]*mtx.decs[0*4+j]
					+	nums[i*4+1]*mtx.decs[1*4+j]
					+	nums[i*4+2]*mtx.decs[2*4+j]
					+	nums[i*4+3]*mtx.decs[3*4+j];
			res3	=	((decs[i*4+0]*mtx.decs[0*4+j])>>16)
					+	((decs[i*4+1]*mtx.decs[1*4+j])>>16)
					+	((decs[i*4+2]*mtx.decs[2*4+j])>>16)
					+	((decs[i*4+3]*mtx.decs[3*4+j])>>16);

			newMtx.nums[i*4+j] = res0 + (res1+res2+(int)(res3))/65536;
			dec  = (res1+res2+(int)(res3))%65536;
			if( dec < 0 )
			{
				newMtx.nums[i*4+j]--;
				dec += 65536;
			}
			newMtx.decs[i*4+j]  = dec;

			//org = *((&(c._11))+i*4+j);
			//res = newMtx.nums[i*4+j] + newMtx.decs[i*4+j]/65536.0f;
			//dif = org-res;
			//if( dif > 0.001 || dif < -0.001 )
			//{
			//	dif = org-res;
			//}/

		}
	}

	//DaedalusMatrix d = newMtx.convert();
	//DaedalusMatrix e = c-d;

	return newMtx;
}

N64IntMatrix::N64IntMatrix( const N64IntMatrix& mtx )
{
	for(register int i=0; i<16; i++)
	{
		nums[i] = mtx.nums[i];
		decs[i] = mtx.decs[i];
	}
}

N64IntMatrix::N64IntMatrix( const short new_nums[16], const int new_decs[16])
{
	for(register int i=0; i<16; i++)
	{
		nums[i] = new_nums[i];
		decs[i] = new_decs[i];
	}
}

N64IntMatrix::N64IntMatrix(const WORD *new_nums, const WORD *new_decs)
{
	for(register int i=0; i<16; i++)
	{
		nums[i] = (short)new_nums[i];
		decs[i] = (int)new_decs[i];
	}
}

N64IntMatrix::N64IntMatrix()
{
	for( int i=0;i<16;i++ )
	{
		nums[i] = 0;
		decs[i] = 0;
	}
	nums[0]=nums[5]=nums[10]=nums[15]=1;
}

N64IntMatrix::N64IntMatrix(DWORD addr)
{
	register int i,j;
	for (i = 0; i < 4; i++) 
	{
		for (j = 0; j < 4; j++) 
		{
			short nDataHi = *(short *)(((addr+(i<<3)+(j<<1)     )^0x2));
			WORD  nDataLo = *(WORD  *)(((addr+(i<<3)+(j<<1) + 32)^0x2));
			nums[(i<<2)+j] = nDataHi;
			decs[(i<<2)+j] = nDataLo;
		}
	}
}

DaedalusMatrix N64IntMatrix::convert() const
{
	DaedalusMatrix mtx;
	for( int i=0; i<16; i++ )
	{
		*(&(mtx._11)+i) = nums[i]+decs[i]/65536.0f;
	}

	return mtx;
}
#endif
