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

extern DWORD constFactorMap[2];

/*
 *	The lowest end video cards, such as some onboard crappy video cards
 *  only supports 1 texture, 1 stage, and only supports texture ops:
 *
 *		MODULATE
 *		SEL Arg1, SEL Arg2
 *		BLENDTextureAlpha
 *
 *	Very limited, only SEL, MOD, BlendTexAlpha, no factor, 1 stage only, 1 texture only
 *
 */
CDirectXColorCombinerLowestEnd::CDirectXColorCombinerLowestEnd(CDaedalusRender *pRender)
	:CDirectXColorCombiner(pRender)
{
	TRACE0("Create CDirectXColorCombinerLowestEnd");
}

bool CDirectXColorCombinerLowestEnd::GenerateD3DCombineInfo(SetCombineInfo &ci)
{
	DecodedMux newMux = *m_pDecodedMux;
	newMux.ReplaceVal(MUX_ENV, MUX_SHADE);
	newMux.ReplaceVal(MUX_PRIM, MUX_SHADE);
	newMux.ReplaceVal(MUX_TEXEL1, MUX_TEXEL0);
	newMux.Reformat();

	StageOperate *colocOp = &(ci.stages[0].colorOp);
	StageOperate *alphaOp = &(ci.stages[0].alphaOp);
	ci.nStages = 1;
	ci.blendingFunc = ENABLE_BOTH;
	ci.stages[0].dwTexture = TEX_0;
	ci.m_dwShadeColorChannelFlag = MUX_0;
	ci.m_dwShadeAlphaChannelFlag = MUX_0;
	ci.specularPostOp = MUX_0;
	ci.TFactor = MUX_0;
	colocOp->Arg0 = alphaOp->Arg0 = 0;

	for( int i=0; i<2; i++ )
	{
		N64CombinerType &m = newMux.m_n64Combiners[i];
		StageOperate *op = (i==0)?&(ci.stages[0].colorOp):&(ci.stages[0].alphaOp);
		switch( newMux.splitType[i] )
		{
		case CM_FMT_TYPE_D:
			op->op = D3DTOP_SELECTARG1;
			op->Arg1 = GetD3DArgument(m.d);
			op->Arg2 = D3DTA_IGNORE;
			break;
		case CM_FMT_TYPE_A_ADD_D:
			op->op = D3DTOP_MODULATE;
			op->Arg1 = GetD3DArgument(m.a);
			op->Arg2 = GetD3DArgument(m.d);
			break;
		case CM_FMT_TYPE_A_MOD_C:
			op->op = D3DTOP_MODULATE;
			op->Arg1 = GetD3DArgument(m.a);
			op->Arg2 = GetD3DArgument(m.c);
			break;
		case CM_FMT_TYPE_A_MOD_C_ADD_D:
			op->op = D3DTOP_MODULATE;
			op->Arg1 = GetD3DArgument(m.a);
			if( isTexel(m.c) )
				op->Arg2 = GetD3DArgument(m.c);
			else
				op->Arg2 = GetD3DArgument(m.d);
			break;
		case CM_FMT_TYPE_A_SUB_B:
			op->op = D3DTOP_MODULATE;
			op->Arg1 = GetD3DArgument(m.a);
			op->Arg2 = GetD3DArgument(m.b);
			break;
		case CM_FMT_TYPE_A_LERP_B_C:
		case CM_FMT_TYPE_A_B_C_D:
			if( isTexel(m.b) )
			{
				op->op = D3DTOP_BLENDTEXTUREALPHA;
				op->Arg1 = GetD3DArgument(m.a);
				op->Arg2 = GetD3DArgument(m.b);
			}
			else
			{
				op->op = D3DTOP_MODULATE;
				op->Arg1 = GetD3DArgument(m.a);
				if( isTexel(m.c) )
					op->Arg2 = GetD3DArgument(m.c);
				else
					op->Arg2 = GetD3DArgument(m.b);
			}
			break;
		case CM_FMT_TYPE_A_SUB_B_ADD_D:
			op->op = D3DTOP_MODULATE;
			op->Arg1 = GetD3DArgument(m.a);
			if( isTexel(m.b) )
				op->Arg2 = GetD3DArgument(m.b);
			else
				op->Arg2 = GetD3DArgument(m.d);
			break;
		case CM_FMT_TYPE_A_SUB_B_MOD_C:
			op->op = D3DTOP_MODULATE;
			op->Arg1 = GetD3DArgument(m.a);
			if( isTexel(m.b) )
				op->Arg2 = GetD3DArgument(m.b);
			else
				op->Arg2 = GetD3DArgument(m.c);
			break;
		default:
			op->op = D3DTOP_MODULATE;
			op->Arg1 = GetD3DArgument(m.a);
			op->Arg2 = GetD3DArgument(m.c);
			break;
		}
	}

	ci.stages[0].bTextureUsed = IsTextureUsedInStage(ci.stages[0]);

	return TRUE;
}

/*
 *	
 */
CDirectXColorCombinerLowerEnd::CDirectXColorCombinerLowerEnd(CDaedalusRender *pRender)
	:CDirectXColorCombinerLowestEnd(pRender)
{
	TRACE0("Create CDirectXColorCombinerLowerEnd");
}

bool CDirectXColorCombinerLowerEnd::GenerateD3DCombineInfo(SetCombineInfo & sci)
{
	return CDirectXColorCombiner::GenerateD3DCombineInfo(sci);
}

/*
 *	
 */
CDirectXColorCombinerMiddleLevel::CDirectXColorCombinerMiddleLevel(CDaedalusRender *pRender)
	:CDirectXColorCombinerLowerEnd(pRender)
{
	TRACE0("Create CDirectXColorCombinerMiddleLevel");
}

bool CDirectXColorCombinerMiddleLevel::GenerateD3DCombineInfo(SetCombineInfo & sci)
{
	return CDirectXColorCombiner::GenerateD3DCombineInfo(sci);
}

/*
 *	Higher end video cards includes ATI Rage 128, Voodoo 3/4/5 etc
 *  They can support 2 textures, and can support up to 3 combiner stages
 *  They don't support LERP, MULTIPLYADD, but they support all other texture ops
 */
CDirectXColorCombinerHigherEnd::CDirectXColorCombinerHigherEnd(CDaedalusRender *pRender)
	:CDirectXColorCombinerMiddleLevel(pRender)
{
	TRACE0("Create CDirectXColorCombinerHigherEnd");
}

bool CDirectXColorCombinerHigherEnd::GenerateD3DCombineInfo(SetCombineInfo & sci)
{
	return CDirectXColorCombiner::GenerateD3DCombineInfo(sci);
}



/*
 *	
 */
/*CDirectXColorCombinerNVidiaGeforcePlus::CDirectXColorCombinerNVidiaGeforcePlus(CDaedalusRender *pRender)
	:CNvTNTDirectXCombiner(pRender)
{
	TRACE0("Create CDirectXColorCombinerNVidiaGeforcePlus");
}*/

/*
 *	
 */
CDirectXColorCombinerATIDadeonPlus::CDirectXColorCombinerATIDadeonPlus(CDaedalusRender *pRender)
	:CDirectXColorCombinerHigherEnd(pRender)
{
	TRACE0("Create CDirectXColorCombinerATIDadeonPlus");
}
