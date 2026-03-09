#ifndef _UCODE_H_
#define _UCODE_H_


// I don't like to create a ucode mapping table for each ucode, but it seems I have to do it
// because the shared ucode mapping table is confusing and directing ucode incorrectly for ucode!=1
// games

// This Header should only be included in RDP_GFX.cpp, otherwise will get compiling error.




static void RSP_RDP_Nothing(uint32 word0, uint32 word1);
	   void RSP_GBI0_Mtx(uint32 word0, uint32 word1);
static void RSP_Mtx_DKR(uint32 word0, uint32 word1);
static void RSP_GBI0_DL(uint32 word0, uint32 word1);
static void RSP_DL_In_MEM_DKR(uint32 word0, uint32 word1);

static void RSP_GBI0_Vtx(uint32 word0, uint32 word1);
static void RSP_Vtx_DKR(uint32 word0, uint32 word1);
static void RSP_Vtx_WRUS(uint32 word0, uint32 word1);
static void RSP_Vtx_ShadowOfEmpire(uint32 word0, uint32 word1);

static void RSP_GBI0_Tri4(uint32 word0, uint32 word1);
static void RSP_DMA_Tri_DKR(uint32 word0, uint32 word1);
static void DLParser_Set_Addr_Ucode6(uint32 word0, uint32 word1);
static void RSP_MoveWord_DKR(uint32 word0, uint32 word1);

void 		RSP_Vtx_PD(uint32 word0, uint32 word1);
void 		RSP_Set_Vtx_CI_PD(uint32 word0, uint32 word1);
void 		RSP_Tri4_PD(uint32 word0, uint32 word1);

static void RSP_GBI0_Sprite2DBase(uint32 word0, uint32 word1);
static void RSP_GBI0_Sprite2DDraw(uint32 word0, uint32 word1);
static void RSP_GBI1_Sprite2DBase(uint32 word0, uint32 word1);
static void RSP_GBI1_Sprite2DScaleFlip(uint32 word0, uint32 word1);
void		RSP_GBI1_Sprite2DDraw(uint32 word0, uint32 word1);
static void RSP_GBI_Sprite2DBase(uint32 word0, uint32 word1);
static void RSP_GBI_Sprite2D_PuzzleMaster64(uint32 word0, uint32 word1);

static void RSP_GBI1_SpNoop(uint32 word0, uint32 word1);
static void RSP_GBI1_Reserved(uint32 word0, uint32 word1);
static void RSP_GBI1_Vtx(uint32 word0, uint32 word1);
static void RSP_GBI1_MoveMem(uint32 word0, uint32 word1);
static void RSP_GBI1_RDPHalf_Cont(uint32 word0, uint32 word1);
static void RSP_GBI1_RDPHalf_2(uint32 word0, uint32 word1);
static void RSP_GBI1_RDPHalf_1(uint32 word0, uint32 word1);
static void RSP_GBI1_Line3D(uint32 word0, uint32 word1);
static void RSP_GBI1_ClearGeometryMode(uint32 word0, uint32 word1);
static void RSP_GBI1_SetGeometryMode(uint32 word0, uint32 word1);
static void RSP_GBI1_EndDL(uint32 word0, uint32 word1);
static void RSP_GBI1_SetOtherModeL(uint32 word0, uint32 word1);
static void RSP_GBI1_SetOtherModeH(uint32 word0, uint32 word1);
static void RSP_GBI1_Texture(uint32 word0, uint32 word1);
static void RSP_GBI1_MoveWord(uint32 word0, uint32 word1);
static void RSP_GBI1_PopMtx(uint32 word0, uint32 word1);
static void RSP_GBI1_CullDL(uint32 word0, uint32 word1);
static void RSP_GBI1_Tri1(uint32 word0, uint32 word1);
	   void RSP_GBI1_Tri2(uint32 word0, uint32 word1);
static void RSP_GBI1_Noop(uint32 word0, uint32 word1);
	   void RSP_GBI1_ModifyVtx(uint32 word0, uint32 word1);
	   void RSP_GBI1_BranchZ(uint32 word0, uint32 word1);
	   void RSP_GBI1_LoadUCode(uint32 word0, uint32 word1);

	   void DLParser_TexRect(uint32 word0, uint32 word1);
static void DLParser_TexRectFlip(uint32 word0, uint32 word1);
static void DLParser_RDPLoadSync(uint32 word0, uint32 word1);
static void DLParser_RDPPipeSync(uint32 word0, uint32 word1);
static void DLParser_RDPTileSync(uint32 word0, uint32 word1);
static void DLParser_RDPFullSync(uint32 word0, uint32 word1);
static void DLParser_SetKeyGB(uint32 word0, uint32 word1);
static void DLParser_SetKeyR(uint32 word0, uint32 word1);
static void DLParser_SetConvert(uint32 word0, uint32 word1);
static void DLParser_SetScissor(uint32 word0, uint32 word1);
static void DLParser_SetPrimDepth(uint32 word0, uint32 word1);
static void DLParser_RDPSetOtherMode(uint32 word0, uint32 word1);
static void DLParser_LoadTLut(uint32 word0, uint32 word1);
static void DLParser_SetTileSize(uint32 word0, uint32 word1);
static void DLParser_LoadBlock(uint32 word0, uint32 word1);
static void DLParser_LoadTile(uint32 word0, uint32 word1);
static void DLParser_SetTile(uint32 word0, uint32 word1);
static void DLParser_FillRect(uint32 word0, uint32 word1);
static void DLParser_SetFillColor(uint32 word0, uint32 word1);
static void DLParser_SetFogColor(uint32 word0, uint32 word1);
static void DLParser_SetBlendColor(uint32 word0, uint32 word1);
static void DLParser_SetPrimColor(uint32 word0, uint32 word1);
static void DLParser_SetEnvColor(uint32 word0, uint32 word1);
static void DLParser_SetCombine(uint32 word0, uint32 word1);
static void DLParser_SetTImg(uint32 word0, uint32 word1);
static void DLParser_SetZImg(uint32 word0, uint32 word1);
static void DLParser_SetCImg(uint32 word0, uint32 word1);

static void RSP_GBI2_DL(uint32 word0, uint32 word1);
static void RSP_GBI2_CullDL(uint32 word0, uint32 word1);
static void RSP_GBI2_EndDL(uint32 word0, uint32 word1);
static void RSP_GBI2_MoveWord(uint32 word0, uint32 word1);
static void RSP_GBI2_Texture(uint32 word0, uint32 word1);
static void RSP_GBI2_GeometryMode(uint32 word0, uint32 word1);
static void RSP_GBI2_SetOtherModeL(uint32 word0, uint32 word1);
static void RSP_GBI2_SetOtherModeH(uint32 word0, uint32 word1);
static void RSP_GBI2_MoveMem(uint32 word0, uint32 word1);
static void RSP_GBI2_Mtx(uint32 word0, uint32 word1);
static void RSP_GBI2_PopMtx(uint32 word0, uint32 word1);
static void RSP_GBI2_Vtx(uint32 word0, uint32 word1);
static void RSP_GBI2_Tri1(uint32 word0, uint32 word1);
static void RSP_GBI2_Tri2(uint32 word0, uint32 word1);
static void RSP_GBI2_Line3D(uint32 word0, uint32 word1);

static void RSP_GBI2_DL_Count(uint32 word0, uint32 word1);
static void RSP_GBI2_SubModule(uint32 word0, uint32 word1);
static void RSP_GBI2_0x8(uint32 word0, uint32 word1);
static void DLParser_Bomberman2TextRect(uint32 word0, uint32 word1);

void RSP_S2DEX_BG_1CYC_2(uint32 word0, uint32 word1);
void RSP_S2DEX_OBJ_RENDERMODE_2(uint32 word0, uint32 word1);

void RSP_S2DEX_SPObjLoadTxtr_Ucode1(uint32 word0, uint32 word1);

void RDP_TriFill(uint32 word0, uint32 word1);
void RDP_TriFillZ(uint32 word0, uint32 word1);
void RDP_TriTxtr(uint32 word0, uint32 word1);
void RDP_TriTxtrZ(uint32 word0, uint32 word1);
void RDP_TriShade(uint32 word0, uint32 word1);
void RDP_TriShadeZ(uint32 word0, uint32 word1);
void RDP_TriShadeTxtr(uint32 word0, uint32 word1);
void RDP_TriShadeTxtrZ(uint32 word0, uint32 word1);

LPCSTR ucodeNames_GBI1[256] =
{
	"RSP_SPNOOP",	 "RSP_MTX",     "Reserved0", "RSP_MOVEMEM",
	"RSP_VTX",	 "Reserved1",  "RSP_DL",     "Reserved2",
	"RSP_RESERVED3", "RSP_SPRITE2D", "G_NOTHING", "G_NOTHING",
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
	"G_NOTHING", "G_NOTHING", "G_NOTHING", "RSP_LOAD_UCODE",
//B0
	"RSP_BRANCH_Z", "RSP_TRI2",    "G_MODIFY_VERTEX", "RSP_RDPHALF_2",
	"RSP_RDPHALF_1", "RSP_LINE3D", "RSP_CLEARGEOMETRYMODE", "RSP_SETGEOMETRYMODE",
	"RSP_ENDDL", "RSP_SETOTHERMODE_L", "RSP_SETOTHERMODE_H", "RSP_TEXTURE",
	"RSP_MOVEWORD", "RSP_POPMTX", "RSP_CULLDL", "RSP_TRI1",

//C0
	"RDP_NOOP",    "G_NOTHING", "G_YS_UNK1", "G_NOTHING",
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
	"RDP_TEXRECT", "RDP_TEXRECT_FLIP", "RDP_LOADSYNC", "RDP_PIPESYNC",
	"RDP_TILESYNC", "RDP_FULLSYNC", "RDP_SETKEYGB", "RDP_SETKEYR",
	"RDP_SETCONVERT", "RDP_SETSCISSOR", "RDP_SETPRIMDEPTH", "RDP_RDPSETOTHERMODE",
//F0
	"RDP_LOADTLUT", "G_NOTHING", "RDP_SETTILESIZE", "RDP_LOADBLOCK", 
	"RDP_LOADTILE", "RDP_SETTILE", "RDP_FILLRECT", "RDP_SETFILLCOLOR",
	"RDP_SETFOGCOLOR", "RDP_SETBLENDCOLOR", "RDP_SETPRIMCOLOR", "RDP_SETENVCOLOR",
	"RDP_SETCOMBINE", "RDP_SETTIMG", "RDP_SETZIMG", "RDP_SETCIMG"


};


LPCSTR ucodeNames_GBI2[256] =
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
	RSP_GBI1_SpNoop,   RSP_GBI0_Mtx,		  RSP_GBI1_Reserved, RSP_GBI1_MoveMem,
	RSP_GBI1_Vtx,	   RSP_GBI1_Reserved,	  RSP_GBI0_DL,		 RSP_GBI1_Reserved,
	RSP_GBI1_Reserved, RSP_GBI1_Sprite2DBase, RSP_RDP_Nothing,	 RSP_RDP_Nothing,
	RSP_RDP_Nothing,   RSP_RDP_Nothing,		  RSP_RDP_Nothing,	 RSP_RDP_Nothing,
//10
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//20
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//30
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//40
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//50
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//60
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//70
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,

	// The next 64 commands (-65 .. -128, or 128..191) are "Immediate" commands,
	//  in that they can be executed immediately with no further memory transfers
//80
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//90
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//a0
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_GBI1_LoadUCode,
//b0
	RSP_GBI1_BranchZ, RSP_GBI1_Tri2, RSP_GBI1_ModifyVtx, RSP_GBI1_RDPHalf_2,
	RSP_GBI1_RDPHalf_1, RSP_GBI1_Line3D, RSP_GBI1_ClearGeometryMode, RSP_GBI1_SetGeometryMode,
	RSP_GBI1_EndDL, RSP_GBI1_SetOtherModeL, RSP_GBI1_SetOtherModeH, RSP_GBI1_Texture,
	RSP_GBI1_MoveWord, RSP_GBI1_PopMtx, RSP_GBI1_CullDL, RSP_GBI1_Tri1,

	// The last 64 commands are "RDP" commands; they are passed through the
	//  RSP and sent to the RDP directly.
//c0
	RSP_GBI1_Noop,    RSP_S2DEX_SPObjLoadTxtr_Ucode1, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RDP_TriFill, RDP_TriFillZ, RDP_TriTxtr, RDP_TriTxtrZ,
	RDP_TriShade, RDP_TriShadeZ, RDP_TriShadeTxtr, RDP_TriShadeTxtrZ,
//d0
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//e0
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	DLParser_TexRect, DLParser_TexRectFlip, DLParser_RDPLoadSync, DLParser_RDPPipeSync,
	DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB, DLParser_SetKeyR,
	DLParser_SetConvert, DLParser_SetScissor, DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
//f0
	DLParser_LoadTLut, RSP_RDP_Nothing, DLParser_SetTileSize, DLParser_LoadBlock, 
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
	RSP_GBI1_SpNoop,	 RSP_GBI0_Mtx,     RSP_GBI1_Reserved, RSP_GBI1_MoveMem,
	RSP_GBI0_Vtx, RSP_GBI1_Reserved, RSP_GBI0_DL,	RSP_GBI1_Reserved,
	RSP_GBI1_Reserved, RSP_GBI0_Sprite2DBase, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//10
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//20
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//30
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//40
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//50
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//60
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//70
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,

	// The next 64 commands (-65 .. -128, or 128..191) are "Immediate" commands,
	//  in that they can be executed immediately with no further memory transfers
//80
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//90
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//a0
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//b0
	RSP_RDP_Nothing, RSP_GBI0_Tri4, RSP_GBI1_RDPHalf_Cont, RSP_GBI1_RDPHalf_2,
	RSP_GBI1_RDPHalf_1, RSP_GBI1_Line3D, RSP_GBI1_ClearGeometryMode, RSP_GBI1_SetGeometryMode,
	RSP_GBI1_EndDL, RSP_GBI1_SetOtherModeL, RSP_GBI1_SetOtherModeH, RSP_GBI1_Texture,
	RSP_GBI1_MoveWord, RSP_GBI1_PopMtx, RSP_GBI1_CullDL, RSP_GBI1_Tri1,

	// The last 64 commands are "RDP" commands; they are passed through the
	//  RSP and sent to the RDP directly.
//c0
	RSP_GBI1_Noop,    RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RDP_TriFill, RDP_TriFillZ, RDP_TriTxtr, RDP_TriTxtrZ,
	RDP_TriShade, RDP_TriShadeZ, RDP_TriShadeTxtr, RDP_TriShadeTxtrZ,
//d0
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//e0
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	DLParser_TexRect, DLParser_TexRectFlip, DLParser_RDPLoadSync, DLParser_RDPPipeSync,
	DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB, DLParser_SetKeyR,
	DLParser_SetConvert, DLParser_SetScissor, DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
//f0
	DLParser_LoadTLut, RSP_RDP_Nothing, DLParser_SetTileSize, DLParser_LoadBlock, 
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
	RSP_GBI1_Noop,	 RSP_GBI2_Vtx,     RSP_GBI1_ModifyVtx, RSP_GBI2_CullDL,
	RSP_GBI1_BranchZ, RSP_GBI2_Tri1, RSP_GBI2_Tri2,	RSP_GBI2_Line3D,
	RSP_GBI2_0x8, RSP_S2DEX_BG_1CYC, RSP_S2DEX_BG_COPY, RSP_S2DEX_OBJ_RENDERMODE,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//10
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//20
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//30
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//40
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//50
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//60
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//70
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,

	// The next 64 commands (-65 .. -128, or 128..191) are "Immediate" commands,
	//  in that they can be executed immediately with no further memory transfers
//80
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//90
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//a0
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_GBI1_LoadUCode,
//b0
	RSP_GBI1_BranchZ, RSP_GBI0_Tri4, RSP_GBI1_ModifyVtx, RSP_GBI1_RDPHalf_2,
	RSP_GBI1_RDPHalf_1, RSP_GBI1_Line3D, RSP_GBI1_ClearGeometryMode, RSP_GBI1_SetGeometryMode,
	RSP_GBI1_EndDL, RSP_GBI1_SetOtherModeL, RSP_GBI1_SetOtherModeH, RSP_GBI1_Texture,
	RSP_GBI1_MoveWord, RSP_GBI1_PopMtx, RSP_GBI1_CullDL, RSP_GBI1_Tri1,

	// The last 64 commands are "RDP" commands; they are passed through the
	//  RSP and sent to the RDP directly.
//c0
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RDP_TriFill, RDP_TriFillZ, RDP_TriTxtr, RDP_TriTxtrZ,
	RDP_TriShade, RDP_TriShadeZ, RDP_TriShadeTxtr, RDP_TriShadeTxtrZ,
//d0
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_GBI2_DL_Count, RSP_GBI2_SubModule, RSP_GBI2_Texture,
	RSP_GBI2_PopMtx, RSP_GBI2_GeometryMode, RSP_GBI2_Mtx, RSP_GBI2_MoveWord,
	RSP_GBI2_MoveMem, RSP_GBI1_LoadUCode, RSP_GBI2_DL, RSP_GBI2_EndDL,
//e0
	RSP_GBI1_SpNoop, RSP_GBI1_RDPHalf_1, RSP_GBI2_SetOtherModeL, RSP_GBI2_SetOtherModeH,
	DLParser_TexRect, DLParser_TexRectFlip, DLParser_RDPLoadSync, DLParser_RDPPipeSync,
	DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB, DLParser_SetKeyR,
	DLParser_SetConvert, DLParser_SetScissor, DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
//f0
	DLParser_LoadTLut, RSP_RDP_Nothing, DLParser_SetTileSize, DLParser_LoadBlock, 
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
	RSP_GBI1_SpNoop,	 RSP_S2DEX_BG_1CYC_2,     RSP_S2DEX_BG_COPY, RSP_S2DEX_OBJ_RECTANGLE,
	RSP_S2DEX_OBJ_SPRITE, RSP_S2DEX_OBJ_MOVEMEM, RSP_GBI0_DL,	RSP_GBI1_Reserved,
	RSP_GBI1_Reserved, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,

//10
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//20
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//30
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//40
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//50
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//60
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//70
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,

	// The next 64 commands (-65 .. -128, or 128..191) are "Immediate" commands,
	//  in that they can be executed immediately with no further memory transfers
//80
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//90
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//a0
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_GBI1_LoadUCode,
//b0
	RSP_S2DEX_SELECT_DL, RSP_S2DEX_OBJ_RENDERMODE_2, RSP_S2DEX_OBJ_RECTANGLE_R, RSP_GBI1_RDPHalf_2,
	RSP_GBI1_RDPHalf_1, RSP_GBI1_Line3D, RSP_GBI1_ClearGeometryMode, RSP_GBI1_SetGeometryMode,
	RSP_GBI1_EndDL, RSP_GBI1_SetOtherModeL, RSP_GBI1_SetOtherModeH, RSP_GBI1_Texture,
	RSP_GBI1_MoveWord, RSP_GBI1_PopMtx, RSP_GBI1_CullDL, RSP_GBI1_Tri1,

	// The last 64 commands are "RDP" commands; they are passed through the
	//  RSP and sent to the RDP directly.
//c0
	RSP_GBI1_Noop,    RSP_S2DEX_SPObjLoadTxtr, RSP_S2DEX_SPObjLoadTxSprite, RSP_S2DEX_SPObjLoadTxRect,
	RSP_S2DEX_SPObjLoadTxRectR, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RDP_TriFill, RDP_TriFillZ, RDP_TriTxtr, RDP_TriTxtrZ,
	RDP_TriShade, RDP_TriShadeZ, RDP_TriShadeTxtr, RDP_TriShadeTxtrZ,
//d0
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
//e0
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_S2DEX_RDPHALF_0, DLParser_TexRectFlip, DLParser_RDPLoadSync, DLParser_RDPPipeSync,
	DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB, DLParser_SetKeyR,
	DLParser_SetConvert, DLParser_SetScissor, DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
//f0
	DLParser_LoadTLut, RSP_RDP_Nothing, DLParser_SetTileSize, DLParser_LoadBlock, 
	DLParser_LoadTile, DLParser_SetTile, DLParser_FillRect, DLParser_SetFillColor,
	DLParser_SetFogColor, DLParser_SetBlendColor, DLParser_SetPrimColor, DLParser_SetEnvColor,
	DLParser_SetCombine, DLParser_SetTImg, DLParser_SetZImg, DLParser_SetCImg
};

// Ucode 3 - S2DEX GBI2
UcodeMap GFXInstructionUcode3=
{
	RSP_GBI1_Noop,	 RSP_S2DEX_OBJ_RECTANGLE,     RSP_S2DEX_OBJ_SPRITE, RSP_GBI2_CullDL,
	RSP_S2DEX_SELECT_DL, RSP_S2DEX_SPObjLoadTxtr, RSP_S2DEX_SPObjLoadTxSprite,	RSP_S2DEX_SPObjLoadTxRect,
	RSP_S2DEX_SPObjLoadTxRectR, RSP_S2DEX_BG_1CYC, RSP_S2DEX_BG_COPY, RSP_S2DEX_OBJ_RENDERMODE,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	//10
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	//20
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	//30
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	//40
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	//50
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	//60
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	//70
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,

	// The next 64 commands (-65 .. -128, or 128..191) are "Immediate" commands,
	//  in that they can be executed immediately with no further memory transfers
	//80
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	//90
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	//a0
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_GBI1_LoadUCode,
	//b0
	RSP_GBI1_BranchZ, RSP_GBI0_Tri4, RSP_GBI1_ModifyVtx, RSP_GBI1_RDPHalf_2,
	RSP_GBI1_RDPHalf_1, RSP_GBI1_Line3D, RSP_GBI1_ClearGeometryMode, RSP_GBI1_SetGeometryMode,
	RSP_GBI1_EndDL, RSP_GBI1_SetOtherModeL, RSP_GBI1_SetOtherModeH, RSP_GBI1_Texture,
	RSP_GBI1_MoveWord, RSP_GBI1_PopMtx, RSP_GBI1_CullDL, RSP_GBI1_Tri1,

	// The last 64 commands are "RDP" commands; they are passed through the
	//  RSP and sent to the RDP directly.
	//c0
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RDP_TriFill, RDP_TriFillZ, RDP_TriTxtr, RDP_TriTxtrZ,
	RDP_TriShade, RDP_TriShadeZ, RDP_TriShadeTxtr, RDP_TriShadeTxtrZ,
	//d0
	RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing, RSP_RDP_Nothing,
	RSP_RDP_Nothing, RSP_GBI2_DL_Count, RSP_GBI2_SubModule, RSP_GBI2_Texture,
	RSP_GBI2_PopMtx, RSP_GBI2_GeometryMode, RSP_GBI2_Mtx, RSP_GBI2_MoveWord,
	RSP_GBI2_MoveMem, RSP_GBI1_LoadUCode, RSP_GBI2_DL, RSP_GBI2_EndDL,
	//e0
	RSP_GBI1_SpNoop, RSP_GBI1_RDPHalf_1, RSP_GBI2_SetOtherModeL, RSP_GBI2_SetOtherModeH,
	DLParser_TexRect, DLParser_TexRectFlip, DLParser_RDPLoadSync, DLParser_RDPPipeSync,
	DLParser_RDPTileSync, DLParser_RDPFullSync, DLParser_SetKeyGB, DLParser_SetKeyR,
	DLParser_SetConvert, DLParser_SetScissor, DLParser_SetPrimDepth, DLParser_RDPSetOtherMode,
	//f0
	DLParser_LoadTLut, RSP_RDP_Nothing, DLParser_SetTileSize, DLParser_LoadBlock, 
	DLParser_LoadTile, DLParser_SetTile, DLParser_FillRect, DLParser_SetFillColor,
	DLParser_SetFogColor, DLParser_SetBlendColor, DLParser_SetPrimColor, DLParser_SetEnvColor,
	DLParser_SetCombine, DLParser_SetTImg, DLParser_SetZImg, DLParser_SetCImg
};

RDPInstruction *currentUcodeMap = GFXInstructionUcode1;

#endif
