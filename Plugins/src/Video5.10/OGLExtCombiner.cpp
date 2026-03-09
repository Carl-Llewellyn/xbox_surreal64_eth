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
#include "glh_genext.h"

#define         GL_MODULATE_ADD_ATI                   0x8744
#define         GL_MODULATE_SUBTRACT_ATI              0x8746

//========================================================================
COGLExtColorCombiner::COGLExtColorCombiner(CDaedalusRender *pRender)
		:COGLColorCombiner(pRender), m_maxTexUnits(0), m_lastIndex(-1),
		m_dwLastMux0(0), m_dwLastMux1(0)
{
	m_bOGLExtCombinerSupported=false;
	m_bSupportModAdd_ATI = false;
	m_bSupportModSub_ATI = false;
	delete m_pDecodedMux;
	m_pDecodedMux = new COGLExtDecodedMux;
}

bool COGLExtColorCombiner::Initialize(void)
{
	m_bOGLExtCombinerSupported = false;
	m_bSupportModAdd_ATI = false;
	m_bSupportModSub_ATI = false;
	m_maxTexUnits = 1;

	if( COGLColorCombiner::Initialize() )
	{
		m_bSupportMultiTexture = true;
		COGLGraphicsContext *pcontext = (COGLGraphicsContext *)(CGraphicsContext::g_pGraphicsContext);

		if( pcontext->IsExtensionSupported("GL_EXT_texture_env_combine") || pcontext->IsExtensionSupported("GL_ARB_texture_env_combine") )
		{
			m_bOGLExtCombinerSupported = true;
			glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB,&m_maxTexUnits);
			if( m_maxTexUnits > 8 ) m_maxTexUnits = 8;

			TRACE0("Starting Ogl 1.2/1.3/1.4 multitexture combiner" );
			TRACE1("m_maxTexUnits = %d", m_maxTexUnits);
			if( pcontext->IsExtensionSupported("ATI_texture_env_combine3") )
			{
				m_bSupportModAdd_ATI = true;
				m_bSupportModSub_ATI = true;
			}
		}
		else
		{
			ErrorMsg("Your video card does not support OpenGL extension combiner, you can only use the basic OpenGL combiner functions");
		}
		m_supportedStages = m_maxTexUnits;
		return true;
	}

	return false;
}

//========================================================================
void COGLExtColorCombiner::InitCombinerCycleFill(void)
{
	glActiveTextureARB(GL_TEXTURE0_ARB);
	m_pOGLRender->EnableTexUnit(0,FALSE);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	m_pOGLRender->EnableTexUnit(1,FALSE);
}


void COGLExtColorCombiner::InitCombinerCycle12(void)
{
	if( !m_bOGLExtCombinerSupported )	COGLColorCombiner::InitCombinerCycle12();

#ifdef _DEBUG
	if( debuggerDropCombiners )
	{
		UpdateCombiner(m_pDecodedMux->m_dwMux0,m_pDecodedMux->m_dwMux1);
		m_vCompiledSettings.clear();
		m_dwLastMux0 = m_dwLastMux1 = 0;
		debuggerDropCombiners = false;
	}
#endif

	m_pOGLRender->EnableMultiTexture();

	bool combinerIsChanged = false;

	if( m_pDecodedMux->m_dwMux0 != m_dwLastMux0 || m_pDecodedMux->m_dwMux1 != m_dwLastMux1 || m_lastIndex < 0 )
	{
		combinerIsChanged = true;
		m_lastIndex = FindCompiledMux();
		if( m_lastIndex < 0 )		// Can not found
		{
			m_lastIndex = ParseDecodedMux();
		}

		m_dwLastMux0 = m_pDecodedMux->m_dwMux0;
		m_dwLastMux1 = m_pDecodedMux->m_dwMux1;
	}
	
	m_pOGLRender->SetAllTexelRepeatFlag();

	if( m_bCycleChanged || combinerIsChanged || gRDP.texturesAreReloaded )
	{
		if( gRDP.texturesAreReloaded )
		{
			gRDP.texturesAreReloaded = false;
		}

		if( m_bCycleChanged || combinerIsChanged )
		{
			GenerateCombinerSettingConstants(m_lastIndex);
			GenerateCombinerSetting(m_lastIndex);
		}
	}
}


int COGLExtColorCombiner::ParseDecodedMux()
{
#define nextUnit()	{unitNo++;}
	if( m_maxTexUnits<3) 
		return  ParseDecodedMux2Units();

	OGLExtCombinerSaveType res;
	COGLDecodedMux &mux = *(COGLDecodedMux*)m_pDecodedMux;

	int unitNos[2];
	for( int rgbalpha = 0; rgbalpha<2; rgbalpha++ )
	{
		unitNos[rgbalpha] = 0;
		for( int cycle = 0; cycle<2; cycle++ )
		{
			int &unitNo = unitNos[rgbalpha];
			OGLExtCombinerType &unit = res.units[unitNo];
			OGLExt1CombType &comb = unit.Combs[rgbalpha];
			CombinerFormatType type = m_pDecodedMux->splitType[cycle*2+rgbalpha];
			N64CombinerType &m = m_pDecodedMux->m_n64Combiners[cycle*2+rgbalpha];
			comb.arg0 = comb.arg1 = comb.arg2 = MUX_0;

			switch( type )
			{
			case CM_FMT_TYPE_NOT_USED:
				comb.arg0 = MUX_COMBINED;
				unit.ops[rgbalpha] = GL_REPLACE;
				nextUnit();
				break;
			case CM_FMT_TYPE_D:				// = A
				comb.arg0 = m.d;
				unit.ops[rgbalpha] = GL_REPLACE;
				nextUnit();
				break;
			case CM_FMT_TYPE_A_ADD_D:			// = A+D
				comb.arg0 = m.a;
				comb.arg1 = m.d;
				unit.ops[rgbalpha] = GL_ADD;
				nextUnit();
				break;
			case CM_FMT_TYPE_A_SUB_B:			// = A-B
				comb.arg0 = m.a;
				comb.arg1 = m.b;
				unit.ops[rgbalpha] = GL_SUBTRACT_ARB;
				nextUnit();
				break;
			case CM_FMT_TYPE_A_MOD_C:			// = A*C
				comb.arg0 = m.a;
				comb.arg1 = m.c;
				unit.ops[rgbalpha] = GL_MODULATE;
				nextUnit();
				break;
			case CM_FMT_TYPE_A_MOD_C_ADD_D:	// = A*C+D
				if( m_bSupportModAdd_ATI )
				{
					comb.arg0 = m.a;
					comb.arg2 = m.c;
					comb.arg1 = m.d;
					unit.ops[rgbalpha] = GL_MODULATE_ADD_ATI;
					nextUnit();
				}
				else
				{
					if( unitNo < m_maxTexUnits-1 )
					{
						comb.arg0 = m.a;
						comb.arg1 = m.c;
						unit.ops[rgbalpha] = GL_MODULATE;
						nextUnit();
						res.units[unitNo].Combs[rgbalpha].arg0 = MUX_COMBINED;
						res.units[unitNo].Combs[rgbalpha].arg1 = m.d;
						res.units[unitNo].ops[rgbalpha] = GL_ADD;
						nextUnit();
					}
					else
					{
						comb.arg0 = m.a;
						comb.arg1 = m.c;
						comb.arg2 = m.d;
						unit.ops[rgbalpha] = GL_INTERPOLATE_ARB;
						nextUnit();
					}
				}
				break;
			case CM_FMT_TYPE_A_LERP_B_C:		// = (A-B)*C+B
				comb.arg0 = m.a;
				comb.arg1 = m.b;
				comb.arg2 = m.c;
				unit.ops[rgbalpha] = GL_INTERPOLATE_ARB;
				nextUnit();
				break;
			case CM_FMT_TYPE_A_SUB_B_ADD_D:	// = A-B+D
				if( unitNo < m_maxTexUnits-1 )
				{
					comb.arg0 = m.a;
					comb.arg1 = m.b;
					unit.ops[rgbalpha] = GL_SUBTRACT_ARB;
					nextUnit();
					res.units[unitNo].Combs[rgbalpha].arg0 = MUX_COMBINED;
					res.units[unitNo].Combs[rgbalpha].arg1 = m.d;
					res.units[unitNo].ops[rgbalpha] = GL_ADD;
					nextUnit();
				}
				else
				{
					comb.arg0 = m.a;
					comb.arg1 = m.c;
					comb.arg2 = m.d;
					unit.ops[rgbalpha] = GL_INTERPOLATE_ARB;
					nextUnit();
				}
				break;
			case CM_FMT_TYPE_A_SUB_B_MOD_C:	// = (A-B)*C
				if( unitNo < m_maxTexUnits-1 )
				{
					comb.arg0 = m.a;
					comb.arg1 = m.b;
					unit.ops[rgbalpha] = GL_SUBTRACT_ARB;
					nextUnit();
					res.units[unitNo].Combs[rgbalpha].arg0 = MUX_COMBINED;
					res.units[unitNo].Combs[rgbalpha].arg1 = m.c;
					res.units[unitNo].ops[rgbalpha] = GL_MODULATE;
					nextUnit();
				}
				else
				{
					comb.arg0 = m.a;
					comb.arg1 = m.c;
					comb.arg2 = m.d;
					unit.ops[rgbalpha] = GL_INTERPOLATE_ARB;
					nextUnit();
				}
				break;
			case CM_FMT_TYPE_A_B_C_D:			// = (A-B)*C+D
			default:
				if( unitNo < m_maxTexUnits-1 )
				{
					comb.arg0 = m.a;
					comb.arg1 = m.b;
					unit.ops[rgbalpha] = GL_SUBTRACT_ARB;
					nextUnit();
					if( m_bSupportModAdd_ATI )
					{
						res.units[unitNo].Combs[rgbalpha].arg0 = MUX_COMBINED;
						res.units[unitNo].Combs[rgbalpha].arg2 = m.c;
						res.units[unitNo].Combs[rgbalpha].arg1 = m.d;
						res.units[unitNo].ops[rgbalpha] = GL_MODULATE_ADD_ATI;
						nextUnit();
					}
					else
					{
						res.units[unitNo].Combs[rgbalpha].arg0 = m.a;
						res.units[unitNo].Combs[rgbalpha].arg1 = m.b;
						res.units[unitNo].Combs[rgbalpha].arg2 = m.c;
						res.units[unitNo].ops[rgbalpha] = GL_INTERPOLATE_ARB;
						nextUnit();
					}
				}
				else
				{
					comb.arg0 = m.a;
					comb.arg1 = m.c;
					comb.arg2 = m.d;
					unit.ops[rgbalpha] = GL_INTERPOLATE_ARB;
					nextUnit();
				}
				break;
			}
		}
	}
		
	res.numOfUnits = min(m_maxTexUnits, max(unitNos[0],unitNos[1]));

	if( unitNos[0]>m_maxTexUnits || unitNos[1]>m_maxTexUnits ) 
	{
		TRACE0("Unit overflows");
	}

	for( int j=0; j<2; j++ )
	{
		if( unitNos[j]<res.numOfUnits )
		{
			for( int i=unitNos[j]; i<res.numOfUnits; i++ )
			{
				res.units[i].Combs[j].arg0 = MUX_COMBINED;
				res.units[i].ops[j] = GL_REPLACE;
			}
		}
	}

	return SaveParsedResult(res);
}

int COGLExtColorCombiner::ParseDecodedMux2Units()
{
	OGLExtCombinerSaveType res;
	res.numOfUnits = 2;
		COGLDecodedMux &mux = *(COGLDecodedMux*)m_pDecodedMux;
		
		for( int i=0; i<res.numOfUnits*2; i++ )	// Set combiner for each texture unit
		{
			// For each texture unit, set both RGB and Alpha channel
			// Keep in mind that the m_pDecodeMux has been reformated and simplified very well

			OGLExtCombinerType &unit = res.units[i/2];
			OGLExt1CombType &comb = unit.Combs[i%2];

			CombinerFormatType type = m_pDecodedMux->splitType[i];
			N64CombinerType &m = m_pDecodedMux->m_n64Combiners[i];

			comb.arg0 = comb.arg1 = comb.arg2 = MUX_0;
			
			switch( type )
			{
			case CM_FMT_TYPE_NOT_USED:
				comb.arg0 = MUX_COMBINED;
				unit.ops[i%2] = GL_REPLACE;
				break;
			case CM_FMT_TYPE_D:				// = A
				comb.arg0 = m.d;
				unit.ops[i%2] = GL_REPLACE;
				break;
			case CM_FMT_TYPE_A_ADD_D:			// = A+D
				comb.arg0 = m.a;
				comb.arg1 = m.d;
				unit.ops[i%2] = GL_ADD;
				break;
			case CM_FMT_TYPE_A_SUB_B:			// = A-B
				comb.arg0 = m.a;
				comb.arg1 = m.b;
				unit.ops[i%2] = GL_SUBTRACT_ARB;
				break;
			case CM_FMT_TYPE_A_MOD_C:			// = A*C
				comb.arg0 = m.a;
				comb.arg1 = m.c;
				unit.ops[i%2] = GL_MODULATE;
				break;
			case CM_FMT_TYPE_A_MOD_C_ADD_D:	// = A*C+D
				comb.arg0 = m.a;
				comb.arg1 = m.c;
				comb.arg2 = m.d;
				unit.ops[i%2] = GL_INTERPOLATE_ARB;
				break;
			case CM_FMT_TYPE_A_LERP_B_C:		// = (A-B)*C+B
				comb.arg0 = m.a;
				comb.arg1 = m.b;
				comb.arg2 = m.c;
				unit.ops[i%2] = GL_INTERPOLATE_ARB;
				break;
			case CM_FMT_TYPE_A_SUB_B_ADD_D:	// = A-B+D
				// fix me, to use 2 texture units
				comb.arg0 = m.a;
				comb.arg1 = m.b;
				unit.ops[i%2] = GL_SUBTRACT_ARB;
				break;
			case CM_FMT_TYPE_A_SUB_B_MOD_C:	// = (A-B)*C
				// fix me, to use 2 texture units
				comb.arg0 = m.a;
				comb.arg1 = m.c;
				unit.ops[i%2] = GL_MODULATE;
				break;
				break;
			case CM_FMT_TYPE_A_B_C_D:			// = (A-B)*C+D
			default:
				comb.arg0 = m.a;
				comb.arg1 = m.b;
				comb.arg2 = m.c;
				unit.ops[i%2] = GL_INTERPOLATE_ARB;
				break;
			}
		}

	return SaveParsedResult(res);
}

const char* COGLExtColorCombiner::GetOpStr(GLenum op)
{
	switch( op )
	{
	case GL_REPLACE:
		return "REPLACE";
	case GL_MODULATE:
		return "MOD";
	case GL_ADD:
		return "ADD";
	case GL_ADD_SIGNED_ARB:
		return "ADD_SIGNED";
	case GL_INTERPOLATE_ARB:
		return "INTERPOLATE";
	case GL_SUBTRACT_ARB:
	default:
		return "SUB";
	}
}

int COGLExtColorCombiner::SaveParsedResult(OGLExtCombinerSaveType &result)
{
	result.dwMux0 = m_pDecodedMux->m_dwMux0;
	result.dwMux1 = m_pDecodedMux->m_dwMux1;

	m_vCompiledSettings.push_back(result);
	m_lastIndex = m_vCompiledSettings.size()-1;

#ifdef _DEBUG
	if( logUsedMux )
	{
		DisplaySimpleMuxString();
	}
#endif

	return m_lastIndex;
}

#ifdef _DEBUG
extern const char *translatedCombTypes[];
void COGLExtColorCombiner::DisplaySimpleMuxString(void)
{
	char buf0[30], buf1[30], buf2[30];
	OGLExtCombinerSaveType &result = m_vCompiledSettings[m_lastIndex];

	COGLColorCombiner::DisplaySimpleMuxString();
	DebuggerAppendMsg("OpenGL 1.2: %d Stages", result.numOfUnits);		
	for( int i=0; i<result.numOfUnits; i++ )
	{
		DebuggerAppendMsg("//aRGB%d:\t%s: %s, %s, %s\n", i,GetOpStr(result.units[i].rgbOp), DecodedMux::FormatStr(result.units[i].rgbArg0,buf0), DecodedMux::FormatStr(result.units[i].rgbArg1,buf1), DecodedMux::FormatStr(result.units[i].rgbArg2,buf2));		
		DebuggerAppendMsg("//aAlpha%d:\t%s: %s, %s, %s\n", i,GetOpStr(result.units[i].alphaOp), DecodedMux::FormatStr(result.units[i].alphaArg0,buf0), DecodedMux::FormatStr(result.units[i].alphaArg1,buf1), DecodedMux::FormatStr(result.units[i].alphaArg2,buf2));		
	}
	TRACE0("\n\n");
}
#endif


int COGLExtColorCombiner::FindCompiledMux()
{
	for( uint16 i=0; i<m_vCompiledSettings.size(); i++ )
	{
		if( m_vCompiledSettings[i].dwMux0 == m_pDecodedMux->m_dwMux0 && m_vCompiledSettings[i].dwMux1 == m_pDecodedMux->m_dwMux1 )
			return i;
	}

	return -1;
}

//========================================================================

GLenum COGLExtColorCombiner::RGBArgsMap[] =
{
	{GL_PRIMARY_COLOR_ARB,		},	//MUX_0
	{GL_PRIMARY_COLOR_ARB,		},	//MUX_1
	{GL_PREVIOUS_ARB,			},	//MUX_COMBINED,
	{GL_TEXTURE0_ARB,			},	//MUX_TEXEL0,
	{GL_TEXTURE1_ARB,			},	//MUX_TEXEL1,
	{GL_CONSTANT_ARB,			},	//MUX_PRIM,
	{GL_PRIMARY_COLOR_ARB,		},	//MUX_SHADE,
	{GL_CONSTANT_ARB,			},	//MUX_ENV,
	{GL_PREVIOUS_ARB,			},	//MUX_COMBALPHA,
	{GL_TEXTURE0_ARB,			},	//MUX_T0_ALPHA,
	{GL_TEXTURE1_ARB,			},	//MUX_T1_ALPHA,
	{GL_CONSTANT_ARB,			},	//MUX_PRIM_ALPHA,
	{GL_PRIMARY_COLOR_ARB,		},	//MUX_SHADE_ALPHA,
	{GL_CONSTANT_ARB,			},	//MUX_ENV_ALPHA,
	{GL_CONSTANT_ARB,			},	//MUX_LODFRAC,
	{GL_CONSTANT_ARB,			},	//MUX_PRIMLODFRAC,
};


//========================================================================

GLenum COGLExtColorCombiner::MapRGBArgs(BYTE arg)
{
	return RGBArgsMap[arg&MUX_MASK];
}

GLenum COGLExtColorCombiner::MapRGBArgFlags(BYTE arg)
{
	if( (arg & MUX_ALPHAREPLICATE) && (arg & MUX_COMPLEMENT) )
	{
		return GL_ONE_MINUS_SRC_ALPHA;
	}
	else if( (arg & MUX_ALPHAREPLICATE) )
	{
		return GL_SRC_ALPHA;
	}
	else if(arg & MUX_COMPLEMENT) 
	{
		return GL_ONE_MINUS_SRC_COLOR;
	}
	else
		return GL_SRC_COLOR;
}

GLenum COGLExtColorCombiner::MapAlphaArgs(BYTE arg)
{
	return RGBArgsMap[arg&MUX_MASK];
}

GLenum COGLExtColorCombiner::MapAlphaArgFlags(BYTE arg)
{
	if(arg & MUX_COMPLEMENT) 
	{
		return GL_ONE_MINUS_SRC_ALPHA;
	}
	else
		return GL_SRC_ALPHA;
}

//========================================================================

void COGLExtColorCombiner::GenerateCombinerSetting(int index)
{
	OGLExtCombinerSaveType &res = m_vCompiledSettings[index];

	// Texture unit 0
	COGLTexture* pTexture = g_textures[gRSP.curTile].m_pCOGLTexture;
	COGLTexture* pTexture1 = g_textures[(gRSP.curTile+1)&7].m_pCOGLTexture;

	if( pTexture )	m_pOGLRender->BindTexture(pTexture->m_dwTextureName, 0);
	if( pTexture1 )	m_pOGLRender->BindTexture(pTexture1->m_dwTextureName, 1);

	for( int i=0; i<res.numOfUnits; i++ )
	{
		glActiveTextureARB(GL_TEXTURE0_ARB+i);
		m_pOGLRender->EnableTexUnit(i,TRUE);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, res.units[i].rgbOp);

		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, MapRGBArgs(res.units[i].rgbArg0));
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, MapRGBArgFlags(res.units[i].rgbArg0));

		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, MapRGBArgs(res.units[i].rgbArg1));
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, MapRGBArgFlags(res.units[i].rgbArg1));

		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, MapRGBArgs(res.units[i].rgbArg2));
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, MapRGBArgFlags(res.units[i].rgbArg2));

		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, MapRGBArgs(res.units[i].alphaArg0));
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, MapAlphaArgFlags(res.units[i].alphaArg0));

		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, MapRGBArgs(res.units[i].alphaArg1));
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, MapAlphaArgFlags(res.units[i].alphaArg1));

		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_ARB, MapRGBArgs(res.units[i].alphaArg2));
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_ARB, MapAlphaArgFlags(res.units[i].alphaArg2));
	}

	if( res.numOfUnits < m_maxTexUnits )
	{
		for( int i=res.numOfUnits; i<m_maxTexUnits; i++ )
		{
			glActiveTextureARB(GL_TEXTURE0_ARB+i);
			m_pOGLRender->EnableTexUnit(i,FALSE);
		}
	}
}


void COGLExtColorCombiner::GenerateCombinerSettingConstants(int index)
{
	float *fv;
	float tempf[4];

	if( m_pDecodedMux->isUsed(MUX_PRIM) )
	{
		fv = GetPrimitiveColorfv();	// CONSTANT COLOR
	}
	else if( m_pDecodedMux->isUsed(MUX_ENV) )
	{
		fv = GetEnvColorfv();	// CONSTANT COLOR
	}
	else
	{
		float frac = gRDP.primLODFrac / 255.0f;
		tempf[0] = tempf[1] = tempf[2] = tempf[3] = frac;
		fv = &tempf[0];
	}

	OGLExtCombinerSaveType &res = m_vCompiledSettings[index];
	for( int i=0; i<res.numOfUnits; i++ )
	{
		glActiveTextureARB(GL_TEXTURE0_ARB+i);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,fv);
	}
}

