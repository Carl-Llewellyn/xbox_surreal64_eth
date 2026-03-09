#ifndef _UCODE_H_
#define _UCODE_H_


// I don't like to create a ucode mapping table for each ucode, but it seems I have to do it
// because the shared ucode mapping table is confusing and directing ucode incorrectly for ucode!=1
// games

// This Header should only be included in RDP_GFX.cpp, otherwise will get compiling error.




static void DLParser_Nothing(DWORD dwCmd0, DWORD dwCmd1);
	   void DLParser_GBI0_Mtx(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_Mtx_DKR(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI0_DL(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_DL_In_MEM_DKR(DWORD dwCmd0, DWORD dwCmd1);

static void DLParser_GBI0_Vtx(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_Vtx_DKR(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_Vtx_WRUS(DWORD dwCmd0, DWORD dwCmd1);

static void DLParser_GBI0_Tri4(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_DMA_Tri_DKR(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_Set_Addr_Ucode6(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_MoveWord_DKR(DWORD dwCmd0, DWORD dwCmd1);

void DLParser_Vtx_PD(DWORD dwCmd0, DWORD dwCmd1);
void DLParser_Set_Vtx_CI_PD(DWORD dwCmd0, DWORD dwCmd1);
void DLParser_Tri4_PD(DWORD dwCmd0, DWORD dwCmd1);

static void DLParser_GBI0_Sprite2DBase(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI0_Sprite2DDraw(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_Sprite2DBase(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_Sprite2DScaleFlip(DWORD dwCmd0, DWORD dwCmd1);
void DLParser_GBI1_Sprite2DDraw(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI_Sprite2DBase(DWORD dwCmd0, DWORD dwCmd1);

static void DLParser_GBI1_SpNoop(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_Reserved(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_Vtx(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_MoveMem(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_RDPHalf_Cont(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_RDPHalf_2(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_RDPHalf_1(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_Line3D(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_ClearGeometryMode(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_SetGeometryMode(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_EndDL(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_SetOtherModeL(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_SetOtherModeH(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_Texture(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_MoveWord(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_PopMtx(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_CullDL(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_Tri1(DWORD dwCmd0, DWORD dwCmd1);
	   void DLParser_GBI1_Tri2(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI1_Noop(DWORD dwCmd0, DWORD dwCmd1);
	   void DLParser_GBI1_ModifyVtx(DWORD dwCmd0, DWORD dwCmd1);
	   void DLParser_GBI1_BranchZ(DWORD dwCmd0, DWORD dwCmd1);
	   void DLParser_GBI1_LoadUCode(DWORD dwCmd0, DWORD dwCmd1);

	   void DLParser_TexRect(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_TexRectFlip(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_RDPLoadSync(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_RDPPipeSync(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_RDPTileSync(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_RDPFullSync(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_SetKeyGB(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_SetKeyR(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_SetConvert(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_SetScissor(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_SetPrimDepth(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_RDPSetOtherMode(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_LoadTLut(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_SetTileSize(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_LoadBlock(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_LoadTile(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_SetTile(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_FillRect(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_SetFillColor(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_SetFogColor(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_SetBlendColor(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_SetPrimColor(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_SetEnvColor(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_SetCombine(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_SetTImg(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_SetZImg(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_SetCImg(DWORD dwCmd0, DWORD dwCmd1);

static void DLParser_GBI2_DL(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI2_CullDL(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI2_EndDL(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI2_MoveWord(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI2_Texture(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI2_GeometryMode(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI2_SetOtherModeL(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI2_SetOtherModeH(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI2_MoveMem(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI2_Mtx(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI2_PopMtx(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI2_Vtx(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI2_Tri1(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI2_Tri2(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI2_Line3D(DWORD dwCmd0, DWORD dwCmd1);

static void DLParser_GBI2_DL_Count(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI2_SubModule(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_GBI2_0x8(DWORD dwCmd0, DWORD dwCmd1);
static void DLParser_Bomberman2TextRect(DWORD dwCmd0, DWORD dwCmd1);

void RDP_S2DEX_BG_1CYC_2(DWORD dwCmd0, DWORD dwCmd1);
void RDP_S2DEX_OBJ_RENDERMODE_2(DWORD dwCmd0, DWORD dwCmd1);

void RDP_S2DEX_SPObjLoadTxtr_Ucode1(DWORD dwCmd0, DWORD dwCmd1);

void RDP_TriFill(DWORD dwCmd0, DWORD dwCmd1);
void RDP_TriFillZ(DWORD dwCmd0, DWORD dwCmd1);
void RDP_TriTxtr(DWORD dwCmd0, DWORD dwCmd1);
void RDP_TriTxtrZ(DWORD dwCmd0, DWORD dwCmd1);
void RDP_TriShade(DWORD dwCmd0, DWORD dwCmd1);
void RDP_TriShadeZ(DWORD dwCmd0, DWORD dwCmd1);
void RDP_TriShadeTxtr(DWORD dwCmd0, DWORD dwCmd1);
void RDP_TriShadeTxtrZ(DWORD dwCmd0, DWORD dwCmd1);

LPCSTR g_szRDPInstrName[256] =
{
	"G_SPNOOP",	 "G_MTX",     "Reserved0", "G_MOVEMEM",
	"G_VTX",	 "Reserved1",  "G_DL",     "Reserved2",
	"G_RESERVED3", "G_SPRITE2D", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
//10
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
//20
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
//30
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
//40
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
//50
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
//60
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
//70
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",

//80
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
//90
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
//A0
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_LOAD_UCODE",
//B0
	"G_BRANCH_Z", "G_TRI2",    "G_MODIFY_VERTEX", "G_RDPHALF_2",
	"G_RDPHALF_1", "G_LINE3D", "G_CLEARGEOMETRYMODE", "G_SETGEOMETRYMODE",
	"G_ENDDL", "G_SETOTHERMODE_L", "G_SETOTHERMODE_H", "G_TEXTURE",
	"G_MOVEWORD", "G_POPMTX", "G_CULLDL", "G_TRI1",

//C0
	"G_NOOP",    "G_NOTHING", "G_YS_UNK1", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"RDP_TriFill", "RDP_TriFillZ", "RDP_TriTxtr", "RDP_TriTxtrZ",
	"RDP_TriShade", "RDP_TriShadeZ", "RDP_TriShadeTxtr", "RDP_TriShadeTxtrZ",
//D0
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
//E0
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "G_NOTHING",
	"G_TEXRECT", "G_TEXRECTFLIP", "G_RDPLOADSYNC", "G_RDPPIPESYNC",
	"G_RDPTILESYNC", "G_RDPFULLSYNC", "G_SETKEYGB", "G_SETKEYR",
	"G_SETCONVERT", "G_SETSCISSOR", "G_SETPRIMDEPTH", "G_RDPSETOTHERMODE",
//F0
	"G_LOADTLUT", "G_NOTHING", "G_SETTILESIZE", "G_LOADBLOCK", 
	"G_LOADTILE", "G_SETTILE", "G_FILLRECT", "G_SETFILLCOLOR",
	"G_SETFOGCOLOR", "G_SETBLENDCOLOR", "G_SETPRIMCOLOR", "G_SETENVCOLOR",
	"G_SETCOMBINE", "G_SETTIMG", "G_SETZIMG", "G_SETCIMG"


};


LPCSTR g_szRDPInstrName_GBI2[256] =
{
	"NOOP",	"GBI2_Vtx", "ModifyVtx", "GBI2_CullDL",
	"BranchZ", "GBI2_Tri1", "GBI2_Tri2","GBI2_Line3D",
	"Nothing", "ObjBG1CYC", "ObjBGCopy", "OBJ_RenderMode",
	"Nothing", "Nothing", "Nothing", "Nothing",
//10
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
//20
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
//30
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
//40
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
//50
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
//60
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
//70
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",

//80
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
//90
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
//a0
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Load_Ucode",
//b0
	"BranchZ", "Tri2_Goldeneye", "ModifyVtx", "RDPHalf_2",
	"RDPHalf_1", "Line3D", "ClearGeometryMode", "SetGeometryMode",
	"EndDL", "SetOtherMode_L", "SetOtherMode_H", "Texture",
	"MoveWord", "PopMtx", "CullDL", "Tri1",

//c0
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "Nothing", "Nothing", "Nothing",
	"RDP_TriFill", "RDP_TriFillZ", "RDP_TriTxtr", "RDP_TriTxtrZ",
	"RDP_TriShade", "RDP_TriShadeZ", "RDP_TriShadeTxtr", "RDP_TriShadeTxtrZ",
//d0
	"Nothing", "Nothing", "Nothing", "Nothing",
	"Nothing", "GBI2_DL_N", "GBI2_SubModule", "GBI2_Texture",
	"GBI2_PopMtx", "GBI2_SetGeometryMode", "GBI2_Mtx", "GBI2_MoveWord",
	"GBI2_MoveMem", "Load_Ucode", "GBI2_DL", "GBI2_EndDL",
//e0
	"SPNOOP", "RDPHalf_1", "GBI2_SetOtherMode_L", "GBI2_SetOtherMode_H",
	"TexRect", "TexRectFlip", "RDPLoadSync", "RDPPipeSync",
	"RDPTileSync", "RDPFullSync", "SetKeyGB", "SetKeyR",
	"SetConvert", "SetScissor", "SetPrimDepth", "RDPSetOtherMode",
//f0
	"LoadTLut", "Nothing", "SetTileSize", "LoadBlock",
	"LoadTile", "SetTile", "FillRect", "SetFillColor",
	"SetFogColor", "SetBlendColor", "SetPrimColor", "SetEnvColor",
	"SetCombine", "SetTImg", "SetZImg", "SetCImg",
};


typedef RDPInstruction UcodeMap[256] ;

// Ucode: F3DEX, for most games
UcodeMap GFXInstructionUcode1 =
{
	DLParser_GBI1_SpNoop,	 DLParser_GBI0_Mtx,     DLParser_GBI1_Reserved, DLParser_GBI1_MoveMem,
	DLParser_GBI1_Vtx, DLParser_GBI1_Reserved, DLParser_GBI0_DL,	DLParser_GBI1_Reserved,
	DLParser_GBI1_Reserved, DLParser_GBI1_Sprite2DBase, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//10
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//20
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//30
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//40
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//50
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//60
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//70
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,

	// The next 64 commands (-65 .. -128, or 128..191) are "Immediate" commands,
	//  in that they can be executed immediately with no further memory transfers
//80
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//90
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//a0
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_GBI1_LoadUCode,
//b0
	DLParser_GBI1_BranchZ, DLParser_GBI1_Tri2, DLParser_GBI1_ModifyVtx, DLParser_GBI1_RDPHalf_2,
	DLParser_GBI1_RDPHalf_1, DLParser_GBI1_Line3D, DLParser_GBI1_ClearGeometryMode, DLParser_GBI1_SetGeometryMode,
	DLParser_GBI1_EndDL, DLParser_GBI1_SetOtherModeL, DLParser_GBI1_SetOtherModeH, DLParser_GBI1_Texture,
	DLParser_GBI1_MoveWord, DLParser_GBI1_PopMtx, DLParser_GBI1_CullDL, DLParser_GBI1_Tri1,

	// The last 64 commands are "RDP" commands; they are passed through the
	//  RSP and sent to the RDP directly.
//c0
	DLParser_GBI1_Noop,    RDP_S2DEX_SPObjLoadTxtr_Ucode1, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	RDP_TriFill, RDP_TriFillZ, RDP_TriTxtr, RDP_TriTxtrZ,
	RDP_TriShade, RDP_TriShadeZ, RDP_TriShadeTxtr, RDP_TriShadeTxtrZ,
//d0
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//e0
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_TexRect, DLParser_TexRectFlip, DLParser_RDPLoadSync, DLParser_RDPPipeSync,
	DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB, DLParser_SetKeyR,
	DLParser_SetConvert, DLParser_SetScissor, DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
//f0
	DLParser_LoadTLut, DLParser_Nothing, DLParser_SetTileSize, DLParser_LoadBlock, 
	DLParser_LoadTile, DLParser_SetTile, DLParser_FillRect, DLParser_SetFillColor,
	DLParser_SetFogColor, DLParser_SetBlendColor, DLParser_SetPrimColor, DLParser_SetEnvColor,
	DLParser_SetCombine, DLParser_SetTImg, DLParser_SetZImg, DLParser_SetCImg
};


//Ucode 0, Mario 64 and Demos
UcodeMap GFXInstructionUcode0=
{
	// The First 128 Instructions (0..127) are "DMA" commands (i.e. they
	//  require a DMA transfer to move the required data info position)
	//               Zelda_LoadVtx                            Zelda_CullDL
	DLParser_GBI1_SpNoop,	 DLParser_GBI0_Mtx,     DLParser_GBI1_Reserved, DLParser_GBI1_MoveMem,
	DLParser_GBI0_Vtx, DLParser_GBI1_Reserved, DLParser_GBI0_DL,	DLParser_GBI1_Reserved,
	DLParser_GBI1_Reserved, DLParser_GBI0_Sprite2DBase, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//10
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//20
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//30
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//40
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//50
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//60
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//70
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,

	// The next 64 commands (-65 .. -128, or 128..191) are "Immediate" commands,
	//  in that they can be executed immediately with no further memory transfers
//80
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//90
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//a0
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//b0
	DLParser_Nothing, DLParser_GBI0_Tri4, DLParser_GBI1_RDPHalf_Cont, DLParser_GBI1_RDPHalf_2,
	DLParser_GBI1_RDPHalf_1, DLParser_GBI1_Line3D, DLParser_GBI1_ClearGeometryMode, DLParser_GBI1_SetGeometryMode,
	DLParser_GBI1_EndDL, DLParser_GBI1_SetOtherModeL, DLParser_GBI1_SetOtherModeH, DLParser_GBI1_Texture,
	DLParser_GBI1_MoveWord, DLParser_GBI1_PopMtx, DLParser_GBI1_CullDL, DLParser_GBI1_Tri1,

	// The last 64 commands are "RDP" commands; they are passed through the
	//  RSP and sent to the RDP directly.
//c0
	DLParser_GBI1_Noop,    DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	RDP_TriFill, RDP_TriFillZ, RDP_TriTxtr, RDP_TriTxtrZ,
	RDP_TriShade, RDP_TriShadeZ, RDP_TriShadeTxtr, RDP_TriShadeTxtrZ,
//d0
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//e0
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_TexRect, DLParser_TexRectFlip, DLParser_RDPLoadSync, DLParser_RDPPipeSync,
	DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB, DLParser_SetKeyR,
	DLParser_SetConvert, DLParser_SetScissor, DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
//f0
	DLParser_LoadTLut, DLParser_Nothing, DLParser_SetTileSize, DLParser_LoadBlock, 
	DLParser_LoadTile, DLParser_SetTile, DLParser_FillRect, DLParser_SetFillColor,
	DLParser_SetFogColor, DLParser_SetBlendColor, DLParser_SetPrimColor, DLParser_SetEnvColor,
	DLParser_SetCombine, DLParser_SetTImg, DLParser_SetZImg, DLParser_SetCImg
};

// Zelda and new games, F3DEX_GBI_2
UcodeMap GFXInstructionUcode5=
{
	// The First 128 Instructions (0..127) are "DMA" commands (i.e. they
	//  require a DMA transfer to move the required data info position)
	//               Zelda_LoadVtx                            Zelda_CullDL
	DLParser_GBI1_Noop,	 DLParser_GBI2_Vtx,     DLParser_GBI1_ModifyVtx, DLParser_GBI2_CullDL,
	DLParser_GBI1_BranchZ, DLParser_GBI2_Tri1, DLParser_GBI2_Tri2,	DLParser_GBI2_Line3D,
	DLParser_GBI2_0x8, RDP_S2DEX_BG_1CYC, RDP_S2DEX_BG_COPY, RDP_S2DEX_OBJ_RENDERMODE,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//10
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//20
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//30
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//40
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//50
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//60
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//70
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,

	// The next 64 commands (-65 .. -128, or 128..191) are "Immediate" commands,
	//  in that they can be executed immediately with no further memory transfers
//80
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//90
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//a0
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_GBI1_LoadUCode,
//b0
	DLParser_GBI1_BranchZ, DLParser_GBI0_Tri4, DLParser_GBI1_ModifyVtx, DLParser_GBI1_RDPHalf_2,
	DLParser_GBI1_RDPHalf_1, DLParser_GBI1_Line3D, DLParser_GBI1_ClearGeometryMode, DLParser_GBI1_SetGeometryMode,
	DLParser_GBI1_EndDL, DLParser_GBI1_SetOtherModeL, DLParser_GBI1_SetOtherModeH, DLParser_GBI1_Texture,
	DLParser_GBI1_MoveWord, DLParser_GBI1_PopMtx, DLParser_GBI1_CullDL, DLParser_GBI1_Tri1,

	// The last 64 commands are "RDP" commands; they are passed through the
	//  RSP and sent to the RDP directly.
//c0
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	RDP_TriFill, RDP_TriFillZ, RDP_TriTxtr, RDP_TriTxtrZ,
	RDP_TriShade, RDP_TriShadeZ, RDP_TriShadeTxtr, RDP_TriShadeTxtrZ,
//d0
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_GBI2_DL_Count, DLParser_GBI2_SubModule, DLParser_GBI2_Texture,
	DLParser_GBI2_PopMtx, DLParser_GBI2_GeometryMode, DLParser_GBI2_Mtx, DLParser_GBI2_MoveWord,
	DLParser_GBI2_MoveMem, DLParser_GBI1_LoadUCode, DLParser_GBI2_DL, DLParser_GBI2_EndDL,
//e0
	DLParser_GBI1_SpNoop, DLParser_GBI1_RDPHalf_1, DLParser_GBI2_SetOtherModeL, DLParser_GBI2_SetOtherModeH,
	DLParser_TexRect, DLParser_TexRectFlip, DLParser_RDPLoadSync, DLParser_RDPPipeSync,
	DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB, DLParser_SetKeyR,
	DLParser_SetConvert, DLParser_SetScissor, DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
//f0
	DLParser_LoadTLut, DLParser_Nothing, DLParser_SetTileSize, DLParser_LoadBlock, 
	DLParser_LoadTile, DLParser_SetTile, DLParser_FillRect, DLParser_SetFillColor,
	DLParser_SetFogColor, DLParser_SetBlendColor, DLParser_SetPrimColor, DLParser_SetEnvColor,
	DLParser_SetCombine, DLParser_SetTImg, DLParser_SetZImg, DLParser_SetCImg
};


// S2DEX 1.--
UcodeMap GFXInstructionUcode7=
{
	// The First 128 Instructions (0..127) are "DMA" commands (i.e. they
	//  require a DMA transfer to move the required data info position)
	//               Zelda_LoadVtx                            Zelda_CullDL
	DLParser_GBI1_SpNoop,	 RDP_S2DEX_BG_1CYC_2,     RDP_S2DEX_BG_COPY, RDP_S2DEX_OBJ_RECTANGLE,
	RDP_S2DEX_OBJ_SPRITE, RDP_S2DEX_OBJ_MOVEMEM, DLParser_GBI0_DL,	DLParser_GBI1_Reserved,
	DLParser_GBI1_Reserved, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,

//10
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//20
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//30
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//40
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//50
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//60
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//70
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,

	// The next 64 commands (-65 .. -128, or 128..191) are "Immediate" commands,
	//  in that they can be executed immediately with no further memory transfers
//80
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//90
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//a0
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_GBI1_LoadUCode,
//b0
	RDP_S2DEX_SELECT_DL, RDP_S2DEX_OBJ_RENDERMODE_2, RDP_S2DEX_OBJ_RECTANGLE_R, DLParser_GBI1_RDPHalf_2,
	DLParser_GBI1_RDPHalf_1, DLParser_GBI1_Line3D, DLParser_GBI1_ClearGeometryMode, DLParser_GBI1_SetGeometryMode,
	DLParser_GBI1_EndDL, DLParser_GBI1_SetOtherModeL, DLParser_GBI1_SetOtherModeH, DLParser_GBI1_Texture,
	DLParser_GBI1_MoveWord, DLParser_GBI1_PopMtx, DLParser_GBI1_CullDL, DLParser_GBI1_Tri1,

	// The last 64 commands are "RDP" commands; they are passed through the
	//  RSP and sent to the RDP directly.
//c0
	DLParser_GBI1_Noop,    RDP_S2DEX_SPObjLoadTxtr, RDP_S2DEX_SPObjLoadTxSprite, RDP_S2DEX_SPObjLoadTxRect,
	RDP_S2DEX_SPObjLoadTxRectR, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	RDP_TriFill, RDP_TriFillZ, RDP_TriTxtr, RDP_TriTxtrZ,
	RDP_TriShade, RDP_TriShadeZ, RDP_TriShadeTxtr, RDP_TriShadeTxtrZ,
//d0
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
//e0
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	RDP_S2DEX_RDPHALF_0, DLParser_TexRectFlip, DLParser_RDPLoadSync, DLParser_RDPPipeSync,
	DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB, DLParser_SetKeyR,
	DLParser_SetConvert, DLParser_SetScissor, DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
//f0
	DLParser_LoadTLut, DLParser_Nothing, DLParser_SetTileSize, DLParser_LoadBlock, 
	DLParser_LoadTile, DLParser_SetTile, DLParser_FillRect, DLParser_SetFillColor,
	DLParser_SetFogColor, DLParser_SetBlendColor, DLParser_SetPrimColor, DLParser_SetEnvColor,
	DLParser_SetCombine, DLParser_SetTImg, DLParser_SetZImg, DLParser_SetCImg
};

// Ucode 3 - S2DEX GBI2
UcodeMap GFXInstructionUcode3=
{
	DLParser_GBI1_Noop,	 RDP_S2DEX_OBJ_RECTANGLE,     RDP_S2DEX_OBJ_SPRITE, DLParser_GBI2_CullDL,
	RDP_S2DEX_SELECT_DL, RDP_S2DEX_SPObjLoadTxtr, RDP_S2DEX_SPObjLoadTxSprite,	RDP_S2DEX_SPObjLoadTxRect,
	RDP_S2DEX_SPObjLoadTxRectR, RDP_S2DEX_BG_1CYC, RDP_S2DEX_BG_COPY, RDP_S2DEX_OBJ_RENDERMODE,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//10
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//20
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//30
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//40
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//50
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//60
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//70
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,

	// The next 64 commands (-65 .. -128, or 128..191) are "Immediate" commands,
	//  in that they can be executed immediately with no further memory transfers
	//80
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//90
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	//a0
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_GBI1_LoadUCode,
	//b0
	DLParser_GBI1_BranchZ, DLParser_GBI0_Tri4, DLParser_GBI1_ModifyVtx, DLParser_GBI1_RDPHalf_2,
	DLParser_GBI1_RDPHalf_1, DLParser_GBI1_Line3D, DLParser_GBI1_ClearGeometryMode, DLParser_GBI1_SetGeometryMode,
	DLParser_GBI1_EndDL, DLParser_GBI1_SetOtherModeL, DLParser_GBI1_SetOtherModeH, DLParser_GBI1_Texture,
	DLParser_GBI1_MoveWord, DLParser_GBI1_PopMtx, DLParser_GBI1_CullDL, DLParser_GBI1_Tri1,

	// The last 64 commands are "RDP" commands; they are passed through the
	//  RSP and sent to the RDP directly.
	//c0
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	RDP_TriFill, RDP_TriFillZ, RDP_TriTxtr, RDP_TriTxtrZ,
	RDP_TriShade, RDP_TriShadeZ, RDP_TriShadeTxtr, RDP_TriShadeTxtrZ,
	//d0
	DLParser_Nothing, DLParser_Nothing, DLParser_Nothing, DLParser_Nothing,
	DLParser_Nothing, DLParser_GBI2_DL_Count, DLParser_GBI2_SubModule, DLParser_GBI2_Texture,
	DLParser_GBI2_PopMtx, DLParser_GBI2_GeometryMode, DLParser_GBI2_Mtx, DLParser_GBI2_MoveWord,
	DLParser_GBI2_MoveMem, DLParser_GBI1_LoadUCode, DLParser_GBI2_DL, DLParser_GBI2_EndDL,
	//e0
	DLParser_GBI1_SpNoop, DLParser_GBI1_RDPHalf_1, DLParser_GBI2_SetOtherModeL, DLParser_GBI2_SetOtherModeH,
	DLParser_TexRect, DLParser_TexRectFlip, DLParser_RDPLoadSync, DLParser_RDPPipeSync,
	DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB, DLParser_SetKeyR,
	DLParser_SetConvert, DLParser_SetScissor, DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
	//f0
	DLParser_LoadTLut, DLParser_Nothing, DLParser_SetTileSize, DLParser_LoadBlock, 
	DLParser_LoadTile, DLParser_SetTile, DLParser_FillRect, DLParser_SetFillColor,
	DLParser_SetFogColor, DLParser_SetBlendColor, DLParser_SetPrimColor, DLParser_SetEnvColor,
	DLParser_SetCombine, DLParser_SetTImg, DLParser_SetZImg, DLParser_SetCImg
};

RDPInstruction *currentUcodeMap = GFXInstructionUcode1;

#endif
