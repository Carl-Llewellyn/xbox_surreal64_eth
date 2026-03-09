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
#include "ucode.h"
//#include "menu/menumain.h"
extern void RunIngameMenu();

// Lkb added mirroring support for cards that don't support it. Thanks Lkb!
/*
Description:
	N64 games often use texture mirroring to display semi-transparent circles.
	Examples:
	- Shadow under Mario and other characters
	- Transition that happens when Mario opens the castle door
	- The cannon in Mario
	- The telescope in Majora's Mask (seen in TR64)

	However, some 3D cards do not support texture mirroring and mirrored textures are displayed incorrectly.
	For the first two cases, this is not a big problem, but it's annoying in the last two.

	This patch emulates mirroring in software when the 3D card doesn't support it (eg. Matrox G400)

	Problems:
	- This patch doesn't redefine any texture coordinate (maybe it's correct, maybe no, I failed to understand this :) )
	- This patch always assumes 2x2 mirroring
	- This patch computes the mirroring at every frame and does not cache the texture
*/


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//                    uCode Config                      //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
#define MAX_UCODE_INFO	16
UcodeInfo ucodeInfo[MAX_UCODE_INFO];

RDPInstruction LoadedUcodeMap[256];
LPCSTR LoadedUcodeNameMap[256];

// This is the multiplier applied to vertex indices. 
// For Mario 64, it is 10.
// For Starfox, Mariokart etc it is 2.
OSTask *g_pOSTask = NULL;
UcodeInfo lastUcodeInfo;
UcodeInfo UsedUcodes[MAX_UCODE_INFO];
const uint32 maxUsedUcodes = sizeof(UsedUcodes)/sizeof(UcodeInfo);

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//                     Ucodes                           //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

UcodeMap *ucodeMaps[] = 
{
	&GFXInstructionUcode0,	// ucode 0 - Mario
	&GFXInstructionUcode1,	// ucode 1 - GBI1
	NULL,					// ucode 2 - Golden Eye
	&GFXInstructionUcode3,	// ucode 3 - S2DEX GBI2
	NULL,					// ucode 4 - Wave Racer
	&GFXInstructionUcode5,	// ucode 5 - BGI2
	NULL,					// ucode 6 - DKR
	&GFXInstructionUcode7,	// ucode 7 - S2DEX
	NULL,					// ucode 8 - Star Wars - Rogue Squadron, 
	NULL,					// ucode 9 - Perfect Dark
	NULL,					// ucode 10 - Conker
	NULL,					// ucode 11 - Gemini
	NULL,					// ucode 12 - Silicon Valley, Spacestation
	NULL,					// ucode 13 - modified ucode S2DEX
	NULL,					// ucode 14 - OgreBattle Background
	NULL,					// ucode 15 - ucode 0 with sprite2D
	NULL,					// ucode 16 - Star War, Shadow of Empire
	NULL,					// ucode 17 - ucode 0 with sprite2D, for Demo Puzzle Master 64
};

uint32 vertexMultVals[] =
{
	10, // ucode 0 - Mario
	2,	// ucode 1 - GBI1
	10,	// ucode 2 - Golden Eye
	2,	// ucode 3 - S2DEX GBI2
	5,	// ucode 4 - Wave Racer
	2,	// ucode 5 - BGI2
	10, // ucode 6 - DKR
	2,	// ucode 7 - S2DEX
	2,	// ucode 8 - Star Wars - Rogue Squadron, 
	10, // ucode 9 - Perfect Dark
	2,	// ucode 10 - Conker
	10, // ucode 11 - Gemini
	2,	// ucode 12 - Silicon Valley, Spacestation
	2,	// ucode 13 - modified ucode S2DEX
	2,	// ucode 14 - OgreBattle Background
	10,	// ucode 15 - ucode 0 with sprite2D
	5,	// ucode 16 - Star War, Shadow of Empire
	10,	// ucode 17 - ucode 0 with sprite2D, for Demo Puzzle Master 64
};


// F3DEX		Fast3D EX
// F3DLX		Improved performance version of F3DEX, GBI compatibility. Can turn clipping on/off
// F3DLP		? Line ?
// F3DZEX		? Zelda ?

// NoN			No Near clipping
// Rej			Reject polys with one or more points outside screenspace

//F3DEX: Extended fast3d. Vertex cache is 32, up to 18 DL links
//F3DLX: Compatible with F3DEX, GBI, but not sub-pixel accurate. Clipping can be explicitly enabled/disabled
//F3DLX.Rej: No clipping, rejection instead. Vertex cache is 64
//F3FLP.Rej: Like F3FLX.Rej. Vertex cache is 80
//L3DEX: Line processing, Vertex cache is 32.

#define RSP_DMATRI	0x05
#define G_DLINMEM	0x07

// THE LEGEND OF ZELDA CZLE
// GOLDENEYE       NGEE
// TETRISPHERE     .......NTPP.
CHAR gLastMicrocodeString[ 300 ] = "";


// All star Tennis 99
// ArmyMen - Air Combat - Conflicts with Xena??
// Duke Nukem 64
// International Superstar Soccer '98
// Mischief Makers
// Testrisphere
// I S S 64

//Wipeout:
//Ucode is: 0xc705c37c, RSP Gfx ucode F3DLX         1.21 Yoshitaka Yasumoto Nintendo.
//Ucode is: 0xb65aa2da, RSP Gfx ucode L3DEX         1.21 Yoshitaka Yasumoto Nintendo.
//Ucode is: 0x700de42e, RSP SW Version: 2.0H, 02-12-97

//Flying Dragon:
//Ucode is: 0x1b304a74, RSP SW Version: 2.0H, 02-12-97
//Ucode is: 0xa56cf996, RSP Gfx ucode L3DEX         1.23 Yoshitaka Yasumoto Nintendo.
//Ucode is: 0xfc6529aa, RSP Gfx ucode F3DEX         1.23 Yoshitaka Yasumoto Nintendo.
//Ucode is: 0xca8927a0, RSP Gfx ucode F3DLX.Rej     1.23 Yoshitaka Yasumoto Nintendo.

//*****************************************************************************
//
//*****************************************************************************
static UcodeData g_UcodeData[] = 
{
	//crc_size,	crc_800;

	// SGI U64 GFX SW TEAM: S Anderson, S Carr, H Cheng, K Luster, R Moore, N Pooley, A Srinivasan
	{0, 0x150c3ce8, 0x150c3ce8, "RSP SW Version: 2.0D, 04-01-96",}, // Super Mario 64
		// Note ucode 4 - no idea why this is so different!
	{4, 0x2b94276f, 0x2b94276f, "RSP SW Version: 2.0D, 04-01-96",}, // Wave Race 64 (v1.0)
	{16,0xb1870454, 0xb1870454, "RSP SW Version: 2.0D, 04-01-96",}, // Star Wars - Shadows of the Empire (v1.0), 
	{0, 0x51671ae4, 0x51671ae4, "RSP SW Version: 2.0D, 04-01-96",}, // Pilot Wings 64, 
	{0, 0x67b5ac55, 0x67b5ac55, "RSP SW Version: 2.0D, 04-01-96",}, // Wibble, 
	{0, 0x64dc8104, 0x64dc8104, "RSP SW Version: 2.0D, 04-01-96",}, // Dark Rift, 
	{0, 0x309f363d, 0x309f363d, "RSP SW Version: 2.0D, 04-01-96",}, // Killer Instinct Gold, 
	{0, 0xfcb57e57, 0xfcb57e57, "RSP SW Version: 2.0D, 04-01-96",}, // Blast Corps, 
	{0, 0xb420f35a, 0xb420f35a, "RSP SW Version: 2.0D, 04-01-96",}, // Blast Corps, 
	{0, 0x6e26c1df, 0x7c98e9c2, "RSP SW Version: 2.0D, 04-01-96",}, 
	{2, 0xc02ac7bc, 0xc02ac7bc, "RSP SW Version: 2.0G, 09-30-96",}, // GoldenEye 007, 
	{0, 0xe5fee3bc, 0xe5fee3bc, "RSP SW Version: 2.0G, 09-30-96",}, // Aero Fighters Assault, 
	{17, 0xe4bb5ad8, 0x80129845, "RSP SW Version: 2.0G, 09-30-96",}, // Puzzle Master 64, 
	{0, 0x72109ec6, 0x72109ec6, "RSP SW Version: 2.0H, 02-12-97",}, // Duke Nukem 64, 
	{0, 0xf24a9a04, 0xf24a9a04, "RSP SW Version: 2.0H, 02-12-97",}, // Tetrisphere, 
	{15,0x700de42e, 0x700de42e, "RSP SW Version: 2.0H, 02-12-97",}, // Wipeout 64 (uses GBI1 too!), 
	{15,0x1b304a74, 0x1b304a74, "RSP SW Version: 2.0H, 02-12-97",}, // Flying Dragon, 
	{15,0xe4bb5ad8, 0xa7b2f704, "RSP SW Version: 2.0H, 02-12-97",}, // Silicon Valley, 
	{15,0xe4bb5ad8, 0x88202781, "RSP SW Version: 2.0H, 02-12-97",}, // Glover, 
	{0, 0xe466b5bd, 0xe466b5bd, "Unknown 0xe466b5bd, 0xe466b5bd",}, // Dark Rift, 
	{9, 0x7064a163, 0x7064a163, "Unknown 0x7064a163, 0x7064a163",}, // Perfect Dark (v1.0), 
	{0, 0x6522df69, 0x71bd078d, "Unknown 0x6522df69, 0x71bd078d",}, // Tetris 

	// GBI1
	{1, 0x6d2a01b1, 0x6d2a01b1, "RSP Gfx ucode ZSortp 0.33 Yoshitaka Yasumoto Nintendo.",}, // Mia Hamm Soccer 64, 
	{1, 0x45ca328e, 0x45ca328e, "RSP Gfx ucode F3DLX         0.95 Yoshitaka Yasumoto Nintendo.",}, // Mario Kart 64, 
	{1, 0x98e3b909, 0x98e3b909, "RSP Gfx ucode F3DEX         0.95 Yoshitaka Yasumoto Nintendo.",},	// Mario Kart 64
	{1, 0x5d446090, 0x5d446090, "RSP Gfx ucode F3DLP.Rej     0.96 Yoshitaka Yasumoto Nintendo.",0,1}, // Jikkyou J. League Perfect Striker, 
	{1, 0x244f5ca3, 0x244f5ca3, "RSP Gfx ucode F3DEX         1.00 Yoshitaka Yasumoto Nintendo.",}, // F-1 Pole Position 64, 
	{1, 0x6a022585, 0x6a022585, "RSP Gfx ucode F3DEX.NoN     1.00 Yoshitaka Yasumoto Nintendo.",1}, // Turok - The Dinosaur Hunter (v1.0), 
	{1, 0x150706be, 0x150706be, "RSP Gfx ucode F3DLX.NoN     1.00 Yoshitaka Yasumoto Nintendo.",1}, // Extreme-G, 
	{1, 0x503f2c53, 0x503f2c53, "RSP Gfx ucode F3DEX.NoN     1.21 Yoshitaka Yasumoto Nintendo.",1}, // Bomberman 64, 
	{1, 0xc705c37c, 0xc705c37c, "RSP Gfx ucode F3DLX         1.21 Yoshitaka Yasumoto Nintendo.",}, // Fighting Force 64, Wipeout 64
	{1, 0xa2146075, 0xa2146075, "RSP Gfx ucode F3DLX.NoN     1.21 Yoshitaka Yasumoto Nintendo.",1}, // San Francisco Rush - Extreme Racing, 
	{1, 0xb65aa2da, 0xb65aa2da, "RSP Gfx ucode L3DEX         1.21 Yoshitaka Yasumoto Nintendo.",}, // Wipeout 64, 
	{1, 0x0c8e5ec9, 0x0c8e5ec9, "RSP Gfx ucode F3DEX         1.21 Yoshitaka Yasumoto Nintendo.",}, // 
	{1, 0xe30795f2, 0xa53df3c4, "RSP Gfx ucode F3DLP.Rej     1.21 Yoshitaka Yasumoto Nintendo.",0,1},
	{12, 0x6b519381, 0xfebacfd8, "Unknown in Toukan Road",},	// I don't know which ucode

	{1, 0xaebeda7d, 0xaebeda7d, "RSP Gfx ucode F3DLX.Rej     1.21 Yoshitaka Yasumoto Nintendo.",0,1}, // Jikkyou World Soccer 3, 
	{1, 0x0c8e5ec9, 0x0c8e5ec9, "RSP Gfx ucode F3DEX         1.23 Yoshitaka Yasumoto Nintendo" ,},  // Wave Race 64 (Rev. 2) - Shindou Rumble Edition (JAP) 
	{1, 0xc705c37c, 0xc705c37c, "RSP Gfx ucode F3DLX         1.23 Yoshitaka Yasumoto Nintendo.",}, // GT
	{1, 0x2a61350d, 0x2a61350d, "RSP Gfx ucode F3DLX         1.23 Yoshitaka Yasumoto Nintendo.",}, // Toy Story2
	{1, 0x0c8e5ec9, 0x0c8e5ec9, "RSP Gfx ucode F3DEX         1.23 Yoshitaka Yasumoto Nintendo.",}, // Wave Race 64 Shindou Edition
	{12,0xfc6529aa, 0xfc6529aa, "RSP Gfx ucode F3DEX         1.23 Yoshitaka Yasumoto Nintendo.",}, // Superman - The Animated Series, 
	{1, 0xa56cf996, 0xa56cf996, "RSP Gfx ucode L3DEX         1.23 Yoshitaka Yasumoto Nintendo.",}, // Flying Dragon, 
	{1, 0xcc83b43f, 0xcc83b43f, "RSP Gfx ucode F3DEX.NoN     1.23 Yoshitaka Yasumoto Nintendo.",1}, // AeroGauge, 
	{1, 0xca8927a0, 0xca8927a0, "RSP Gfx ucode F3DLX.Rej     1.23 Yoshitaka Yasumoto Nintendo.",0,1},	// Puzzle Bobble 64, 
	{1, 0x25689c75, 0xbe481ae8, "RSP Gfx ucode F3DLP.Rej     1.23 Yoshitaka Yasumoto Nintendo.",0,1},
	{1, 0xd2d747b7, 0xd2d747b7, "RSP Gfx ucode F3DLX.NoN     1.23 Yoshitaka Yasumoto Nintendo.",1}, // Penny Racers, 

	// Sprite! See also VRally '99
	{7, 0xecd8b772, 0xecd8b772, "RSP Gfx ucode S2DEX  1.06 Yoshitaka Yasumoto Nintendo.",}, // Yoshi's Story, 
	{7, 0xf59132f5, 0xf59132f5, "RSP Gfx ucode S2DEX  1.07 Yoshitaka Yasumoto Nintendo.",}, // Bakuretsu Muteki Bangaioh, 
	{7, 0x961dd811, 0x961dd811, "RSP Gfx ucode S2DEX  1.03 Yoshitaka Yasumoto Nintendo.",}, // GT

	{5, 0x3e083afa, 0x722f97cc, "RSP Gfx ucode F3DEX.NoN   fifo 2.03  Yoshitaka Yasumoto 1998 Nintendo.",1}, // F-Zero X, 
	{5, 0xa8050bd1, 0xa8050bd1, "RSP Gfx ucode F3DEX       fifo 2.03  Yoshitaka Yasumoto 1998 Nintendo.",}, // F-Zero X, 
	{5, 0x4e8055f0, 0x4e8055f0, "RSP Gfx ucode F3DLX.Rej   fifo 2.03  Yoshitaka Yasumoto 1998 Nintendo.",0,1}, // F-Zero X, 
	{5, 0xabf001f5, 0xabf001f5, "RSP Gfx ucode F3DFLX.Rej  fifo 2.03F Yoshitaka Yasumoto 1998 Nintendo.",0,1}, // F-Zero X, 
	{5, 0xadb4b686, 0xadb4b686, "RSP Gfx ucode F3DEX       fifo 2.04  Yoshitaka Yasumoto 1998 Nintendo.",}, // Top Gear Rally 2, 
	{5, 0x779e2a9b, 0x779e2a9b, "RSP Gfx ucode F3DEX.NoN   fifo 2.04  Yoshitaka Yasumoto 1998 Nintendo.",1}, // California Speed, 
	{5, 0xa8cb3e09, 0xa8cb3e09, "RSP Gfx ucode L3DEX       fifo 2.04  Yoshitaka Yasumoto 1998 Nintendo.",}, // In-Fisherman Bass Hunter 64, 
	{5, 0x2a1341d6, 0x2a1341d6, "RSP Gfx ucode F3DEX       fifo 2.04H Yoshitaka Yasumoto 1998 Nintendo.",}, // Kirby 64 - The Crystal Shards, 
	{5, 0x3e083afa, 0x89a8e0ed, "RSP Gfx ucode F3DEX.NoN   fifo 2.05  Yoshitaka Yasumoto 1998 Nintendo.",1}, // Carmageddon 64 (uncensored), 
	{5, 0x4964b75d, 0x4964b75d, "RSP Gfx ucode F3DEX.NoN   fifo 2.05  Yoshitaka Yasumoto 1998 Nintendo.",1}, 
	{5, 0x39e3e95a, 0x39e3e95a, "RSP Gfx ucode F3DEX       fifo 2.05  Yoshitaka Yasumoto 1998 Nintendo."}, // Knife Edge - Nose Gunner, 
	{5, 0xd2913522, 0xd2913522, "RSP Gfx ucode F3DAM       fifo 2.05  Yoshitaka Yasumoto 1998 Nintendo."}, // Hey You, Pikachu!, 
	{5, 0x3e083afa, 0xc998443f, "RSP Gfx ucode F3DEX       xbus 2.05  Yoshitaka Yasumoto 1998 Nintendo."}, //Triple play
	{5, 0xf4184a7d, 0xf4184a7d, "RSP Gfx ucode F3DEX       fifo 2.06  Yoshitaka Yasumoto 1998 Nintendo.",}, // Hey You, Pikachu!, 
	{5, 0x595a88de, 0x595a88de, "RSP Gfx ucode F3DEX.Rej   fifo 2.06  Yoshitaka Yasumoto 1998 Nintendo.",0,1}, // Bio Hazard 2, 
	{5, 0x0259f764, 0x0259f764, "RSP Gfx ucode F3DLX.Rej   fifo 2.06  Yoshitaka Yasumoto 1998 Nintendo.",0,1}, // Mario Party, 
	{5, 0xe1a5477a, 0xe1a5477a, "RSP Gfx ucode F3DEX.NoN   xbus 2.06  Yoshitaka Yasumoto 1998 Nintendo.",1}, // Command & Conquer, 
	{5, 0x4cfa0a19, 0x4cfa0a19, "RSP Gfx ucode F3DZEX.NoN  fifo 2.06H Yoshitaka Yasumoto 1998 Nintendo.",1}, // The Legend of Zelda - Ocarina of Time (v1.0), 
	{5, 0x2cbd9514, 0x5f40b9f5, "RSP Gfx ucode F3DZEX.NoN  fifo 2.06H Yoshitaka Yasumoto 1998 Nintendo.",1}, 

	// What to do for this L3DEX GBI2?
	{5, 0x3e083afa, 0x882680f4, "RSP Gfx ucode L3DEX       fifo 2.07  Yoshitaka Yasumoto 1998 Nintendo."},	// Polaris Sno

	{5, 0xdeb1cac0, 0xdeb1cac0, "RSP Gfx ucode F3DEX.NoN   fifo 2.07  Yoshitaka Yasumoto 1998 Nintendo.",1}, // Knockout Kings 2000, 
	{5, 0xf4184a7d, 0xf4184a7d, "RSP Gfx ucode F3DEX       fifo 2.07  Yoshitaka Yasumoto 1998 Nintendo.",}, // Xena Warrior Princess - Talisman of Fate, Army Men - Air Combat, Destruction Derby
	{5, 0x4b013e60, 0x4b013e60, "RSP Gfx ucode F3DEX       xbus 2.07  Yoshitaka Yasumoto 1998 Nintendo.",}, // Lode Runner 3-D, 
	{5, 0xd1a63836, 0xd1a63836, "RSP Gfx ucode L3DEX       fifo 2.08  Yoshitaka Yasumoto 1999 Nintendo.",}, // Hey You, Pikachu!, 
	{5, 0x97193667, 0x97193667, "RSP Gfx ucode F3DEX       fifo 2.08  Yoshitaka Yasumoto 1999 Nintendo.",}, // Top Gear Hyper-Bike, 
	{5, 0x92149ba8, 0x92149ba8, "RSP Gfx ucode F3DEX       fifo 2.08  Yoshitaka Yasumoto/Kawasedo 1999.",}, // Paper Mario, 
	{5, 0xae0fb88f, 0xae0fb88f, "RSP Gfx ucode F3DEX       xbus 2.08  Yoshitaka Yasumoto 1999 Nintendo.",}, // WWF WrestleMania 2000, 
	{5, 0xc572f368, 0xc572f368, "RSP Gfx ucode F3DLX.Rej   xbus 2.08  Yoshitaka Yasumoto 1999 Nintendo.",}, // WWF No Mercy, 
	{5, 0x3e083afa, 0x74252492, "RSP Gfx ucode F3DEX.NoN   xbus 2.08  Yoshitaka Yasumoto 1999 Nintendo.",1}, 

	{5, 0x9c2edb70, 0xea98e740, "RSP Gfx ucode F3DEX.NoN   fifo 2.08  Yoshitaka Yasumoto 1999 Nintendo.",1}, // LEGO Racers, 
	{5, 0x79e004a6, 0x79e004a6, "RSP Gfx ucode F3DLX.Rej   fifo 2.08  Yoshitaka Yasumoto 1999 Nintendo.",0,1}, // Mario Party 2, 
	{5, 0xaa6ab3ca, 0xaa6ab3ca, "RSP Gfx ucode F3DEX.Rej   fifo 2.08  Yoshitaka Yasumoto 1999 Nintendo.",0,1}, // V-Rally Edition 99, 
	{5, 0x2c597e0f, 0x2c597e0f, "RSP Gfx ucode F3DEX       fifo 2.08  Yoshitaka Yasumoto 1999 Nintendo.",}, // Cruis'n Exotica,
	{10, 0x4e5f3e3b, 0x4e5f3e3b,"RSP Gfx ucode F3DEXBG.NoN fifo 2.08  Yoshitaka Yasumoto 1999 Nintendo.",1}, // Conker The Bad Fur Day 
	{5, 0x61f31862, 0x61f31862, "RSP Gfx ucode F3DEX.NoN   fifo 2.08H Yoshitaka Yasumoto 1999 Nintendo.",1}, // Pokemon Snap, 
	{5, 0x005f5b71, 0x005f5b71, "RSP Gfx ucode F3DZEX.NoN  fifo 2.08I Yoshitaka Yasumoto/Kawasedo 1999.",1}, // The Legend of Zelda 2 - Majora's Mask, 
	{5, 0x7b685972, 0x57b8095a, "Unknown 0x7b685972, 0x57b8095a",}, // Stunt Racer 64, 

	{3, 0x41839d1e, 0x41839d1e, "RSP Gfx ucode S2DEX       fifo 2.05  Yoshitaka Yasumoto 1998 Nintendo.",}, // Chou Snobow Kids, 
	{3, 0x2cbd9514, 0xc639dbb9, "RSP Gfx ucode S2DEX       xbus 2.06  Yoshitaka Yasumoto 1998 Nintendo.",},
	{3, 0xec89e273, 0xec89e273, "RSP Gfx ucode S2DEX       fifo 2.08  Yoshitaka Yasumoto 1999 Nintendo.",}, // V-Rally Edition 99, 
	{3, 0x9429b7d6, 0x9429b7d6, "RSP Gfx ucode S2DEX       xbus 2.08  Yoshitaka Yasumoto 1999 Nintendo.",}, // Star Craft, 
	//{14,0x5a72397b, 0xec89e273, "RSP Gfx ucode S2DEX       fifo 2.08  Yoshitaka Yasumoto 1999 Nintendo.",}, // OgreBattle Background, 
	{3, 0x2cbd9514, 0xec89e273, "RSP Gfx ucode S2DEX       fifo 2.08  Yoshitaka Yasumoto 1999 Nintendo.",}, // Zelda MM, 

	{6, 0x6aef74f8, 0x6aef74f8, "Unknown 0x6aef74f8, 0x6aef74f8",}, // Diddy Kong Racing (v1.0), 
	{6, 0x4c4eead8, 0x4c4eead8, "Unknown 0x4c4eead8, 0x4c4eead8",}, // Diddy Kong Racing (v1.1), 

	{1, 0xed421e9a, 0xed421e9a, "Unknown 0xed421e9a, 0xed421e9a",}, // Kuiki Uhabi Suigo, 
	{5, 0x37751932, 0x55c0fd25, "Unknown 0x37751932, 0x55c0fd25",}, // Bio Hazard 2, 
	{1, 0xa486bed3, 0xa486bed3, "Unknown 0xa486bed3, 0xa486bed3",}, // Last Legion UX, 
	{11,0xbe0b83e7, 0xbe0b83e7,"Unknown 0xbe0b83e7, 0xbe0b83e7",}, // Jet Force Gemini, 

	{8, 0x02e882cf, 0x2ad17281, "Unknown 0x02e882cf, 0x2ad17281",}, // Indiana Jones, 
	{8, 0x1f7d9118, 0xdab2199b, "Unknown 0x1f7d9118, 0xdab2199b",}, // Battle Naboo, 
	{8, 0x74583614, 0x74583614, "Unknown 0x74583614, 0x74583614",}, // Star Wars - Rogue Squadron, 

	//{5, 0x0564a917, 0x0e7aa3cf, "Unknown 0x0564a917, 0x0e7aa3cf",}, // Bio Harzard 2(J), 
	//{3, 0x6522df69, 0xf1e8ba9e, "Unknown 0x6522df69, 0xf1e8ba9e",}, // Bio Harzard 2(J), 
	//{3, 0x61c5ddcd, 0x76e7428f, "Unknown 0x61c5ddcd, 0x76e7428f",}, // Bio Harzard 2(J), 
	//{3, 0x251eea65, 0x69a7162d, "Unknown 0x61c5ddcd, 0x76e7428f",}, // Bio Harzard 2(J), 
};


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//                     GFX State                        //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Multithread protection for our renderer
FiddledVtx * g_pVtxBase=NULL;

SetImgInfo g_TI = { TXT_FMT_RGBA, TXT_SIZE_16b, 1, 0 };
SetImgInfo g_CI = { TXT_FMT_RGBA, TXT_SIZE_16b, 1, 0 };
SetImgInfo g_ZI = { TXT_FMT_RGBA, TXT_SIZE_16b, 1, 0 };
TextureBufferInfo g_ZI_saves[2];

DList	g_dwPCStack[MAX_DL_STACK_SIZE];
int		g_dwPCindex= -1;

TMEMLoadMapInfo g_tmemLoadAddrMap[0x200];	// Totally 4KB TMEM
TMEMLoadMapInfo g_tmemInfo0;				// Info for Tmem=0
TMEMLoadMapInfo g_tmemInfo1;				// Info for Tmem=0x100

char *pszImgSize[4] = {"4", "8", "16", "32"};
const char *textluttype[4] = {"RGB16", "I16?", "RGBA16", "IA16"};
WORD	g_wRDPTlut[0x200];
uint32	g_dwRDPPalCrc[16];

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//                      Ucodes                          //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

#include "FrameBuffer.h"
#include "RDP_GBI0.h"
#include "RDP_GBI1.h"
#include "RDP_GBI2.h"
#include "RDP_GBI2_ext.h"
#include "RDP_GBI_Others.h"
#include "RDP_GBI_Sprite2D.h"
#include "RDP_GBI_Texture.h"

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//                  Init and Reset                      //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void DLParser_Init()
{
	int i;

	status.gRDPTime = 0;
	status.gDlistCount = 0;
	status.gUcodeCount = 0;
	status.frameReadByCPU = FALSE;
	status.frameWriteByCPU = FALSE;
	status.SPCycleCount = 0;
	status.DPCycleCount = 0;
	status.bN64IsDrawingTextureBuffer = false;
	status.bDirectWriteIntoRDRAM = false;
	status.bHandleN64TextureBuffer = false;

	status.bUcodeIsKnown = FALSE;
	status.lastPurgeTimeTime = status.gRDPTime;

	status.curRenderBuffer = NULL;
	status.curDisplayBuffer = NULL;
	status.curVIOriginReg = NULL;

	status.primitiveType = PRIM_TRI1;

	status.lastPurgeTimeTime = 0;		// Time textures were last purged

	status.UseLargerTile[0] = status.UseLargerTile[0] = false;
	status.LargerTileRealLeft[0] = status.LargerTileRealLeft[1] = 0;

	for( i=0; i<8; i++ )
	{
		memset(&gRDP.tiles[i], 0, sizeof(Tile));
	}
	memset(g_tmemLoadAddrMap, 0, sizeof(g_tmemLoadAddrMap));

	for( i=0; i<MAX_UCODE_INFO; i++ )
	{
		memset(&ucodeInfo[i], 0, sizeof(UcodeInfo));
	}

	status.bUseModifiedUcodeMap = false;
	status.ucodeHasBeenSet = false;
	status.bAllowLoadFromTMEM = true;
	
	// Check DKR for ucode 6
	char name[200];
	strcpy(name, g_curRomInfo.szGameName);

	memset(&lastUcodeInfo, 0, sizeof(UcodeInfo));
	memset(&UsedUcodes, 0, sizeof(UsedUcodes));
	memset(&g_TmemFlag, 0, sizeof(g_TmemFlag));
	memset(&g_RecentCIInfo, 0, sizeof(RecentCIInfo)*numOfRecentCIInfos);
	memset(&g_RecentVIOriginInfo, 0, sizeof(RecentViOriginInfo)*numOfRecentCIInfos);
	memset(&g_ZI_saves, 0, sizeof(TextureBufferInfo)*2);

	status.UseLargerTile[0] = status.UseLargerTile[1] = false;
	status.LargerTileRealLeft[0] = status.LargerTileRealLeft[1] = 0;
}


void RDP_GFX_Reset()
{
	g_dwPCindex=-1;
	status.bUcodeIsKnown = FALSE;
	gTextureManager.DropTextures();
}


void RDP_Cleanup()
{
	if( status.bHandleN64TextureBuffer )
	{
		CGraphicsContext::g_pGraphicsContext->CloseTextureBuffer(false);
	}
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//                   Task Handling                      //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void RDP_SetUcodeMap(int ucode)
{
	status.bUseModifiedUcodeMap = false;
	switch( ucode )
	{
	case 0:	// Mario and demos
		break;
	case 1:	// F3DEX GBI1
		break;
	case 2: // Golden Eye
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode0, sizeof(UcodeMap));
		//LoadedUcodeMap[9]=RSP_GBI1_Sprite2DBase;
		//LoadedUcodeMap[0xaf]=RSP_GBI1_LoadUCode;
		//LoadedUcodeMap[0xb0]=RSP_GBI1_BranchZ;
		LoadedUcodeMap[0xb4]=DLParser_RDPHalf_1_0xb4_GoldenEye;
		status.bUseModifiedUcodeMap = true;
		break;
	case 3:	// S2DEX GBI2
		break;
	case 4:
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode0, sizeof(UcodeMap));
		LoadedUcodeMap[4]=RSP_Vtx_WRUS;
		LoadedUcodeMap[0xb1]=RSP_GBI1_Tri2;
		//LoadedUcodeMap[9]=RSP_GBI1_Sprite2DBase;
		//LoadedUcodeMap[0xaf]=RSP_GBI1_LoadUCode;
		//LoadedUcodeMap[0xb0]=RSP_GBI1_BranchZ;
		//LoadedUcodeMap[0xb2]=RSP_GBI1_ModifyVtx;
		status.bUseModifiedUcodeMap = true;
		break;
	case 5:	// F3DEX GBI2
		break;
	case 6: // DKR, Jet Force Gemini, Mickey
	case 11: // DKR, Jet Force Gemini, Mickey
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode0, sizeof(UcodeMap));
		LoadedUcodeMap[1]=RSP_Mtx_DKR;
		LoadedUcodeMap[4]=RSP_Vtx_DKR;
		if( ucode == 11 )	LoadedUcodeMap[4]=RSP_Vtx_Gemini;
		LoadedUcodeMap[5]=RSP_DMA_Tri_DKR;
		LoadedUcodeMap[7]=RSP_DL_In_MEM_DKR;
		LoadedUcodeMap[0xbc]=RSP_MoveWord_DKR;
		LoadedUcodeMap[0xbf]=DLParser_Set_Addr_Ucode6;
		//LoadedUcodeMap[9]=RSP_GBI1_Sprite2DBase;
		//LoadedUcodeMap[0xb0]=RSP_GBI1_BranchZ;
		//LoadedUcodeMap[0xb2]=RSP_GBI1_ModifyVtx;
		status.bUseModifiedUcodeMap = true;
		break;
	case 7: // S2DEX GBI1
		break;
	case 8:	//Indiana Jones, does not work anyway
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode1, sizeof(UcodeMap));
		LoadedUcodeMap[0]=DLParser_Ucode8_0x0;
		//LoadedUcodeMap[1]=DLParser_Ucode8_0x1;
		LoadedUcodeMap[2]=DLParser_RS_Color_Buffer;
		LoadedUcodeMap[3]=DLParser_RS_MoveMem;
		LoadedUcodeMap[4]=DLParser_RS_Vtx_Buffer;
		LoadedUcodeMap[5]=DLParser_Ucode8_0x05;
		LoadedUcodeMap[6]=DLParser_Ucode8_DL;
		LoadedUcodeMap[7]=DLParser_Ucode8_JUMP;
		LoadedUcodeMap[8]=RSP_RDP_Nothing;
		LoadedUcodeMap[9]=RSP_RDP_Nothing;
		LoadedUcodeMap[10]=RSP_RDP_Nothing;
		LoadedUcodeMap[11]=RSP_RDP_Nothing;
		LoadedUcodeMap[0x80]=DLParser_RS_Block;
		LoadedUcodeMap[0xb4]=DLParser_Ucode8_0xb4;
		LoadedUcodeMap[0xb5]=DLParser_Ucode8_0xb5;
		LoadedUcodeMap[0xb8]=DLParser_Ucode8_EndDL;
		LoadedUcodeMap[0xbc]=DLParser_Ucode8_0xbc;
		LoadedUcodeMap[0xbd]=DLParser_Ucode8_0xbd;
		LoadedUcodeMap[0xbe]=DLParser_RS_0xbe;
		LoadedUcodeMap[0xbF]=DLParser_Ucode8_0xbf;
		status.bUseModifiedUcodeMap = true;
		break;
	case 9:	// Perfect Dark
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode0, sizeof(UcodeMap));
		LoadedUcodeMap[4]=RSP_Vtx_PD;
		LoadedUcodeMap[7]=RSP_Set_Vtx_CI_PD;
		LoadedUcodeMap[0xb1]=RSP_Tri4_PD;
		LoadedUcodeMap[0xb4]=DLParser_RDPHalf_1_0xb4_GoldenEye; // Add ske to PD
		status.bUseModifiedUcodeMap = true;
		break;
	case 10: // Conker BFD
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode5, sizeof(UcodeMap));
		LoadedUcodeMap[1]=RSP_Vtx_Conker;
		LoadedUcodeMap[0x10]=DLParser_Tri4_Conker;
		LoadedUcodeMap[0x11]=DLParser_Tri4_Conker;
		LoadedUcodeMap[0x12]=DLParser_Tri4_Conker;
		LoadedUcodeMap[0x13]=DLParser_Tri4_Conker;
		LoadedUcodeMap[0x14]=DLParser_Tri4_Conker;
		LoadedUcodeMap[0x15]=DLParser_Tri4_Conker;
		LoadedUcodeMap[0x16]=DLParser_Tri4_Conker;
		LoadedUcodeMap[0x17]=DLParser_Tri4_Conker;
		LoadedUcodeMap[0x18]=DLParser_Tri4_Conker;
		LoadedUcodeMap[0x19]=DLParser_Tri4_Conker;
		LoadedUcodeMap[0x1a]=DLParser_Tri4_Conker;
		LoadedUcodeMap[0x1b]=DLParser_Tri4_Conker;
		LoadedUcodeMap[0x1c]=DLParser_Tri4_Conker;
		LoadedUcodeMap[0x1d]=DLParser_Tri4_Conker;
		LoadedUcodeMap[0x1e]=DLParser_Tri4_Conker;
		LoadedUcodeMap[0x1f]=DLParser_Tri4_Conker;
		LoadedUcodeMap[0xdb]=DLParser_MoveWord_Conker;
		LoadedUcodeMap[0xdc]=DLParser_MoveMem_Conker;
		status.bUseModifiedUcodeMap = true;
		break;
	case 12: // Silicon Velley, Space Station
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode1, sizeof(UcodeMap));
		LoadedUcodeMap[0x01]=RSP_Mtx_SiliconValley;
		status.bUseModifiedUcodeMap = true;
		break;
	case 13: // modified S2DEX
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode7, sizeof(UcodeMap));
		//LoadedUcodeMap[S2DEX_BG_1CYC] = GFXInstructionUcode1[S2DEX_BG_1CYC];
		LoadedUcodeMap[S2DEX_OBJ_RECTANGLE] = GFXInstructionUcode1[S2DEX_OBJ_RECTANGLE];
		LoadedUcodeMap[S2DEX_OBJ_SPRITE] = GFXInstructionUcode1[S2DEX_OBJ_SPRITE];
		//LoadedUcodeMap[S2DEX_OBJ_RENDERMODE] = GFXInstructionUcode1[S2DEX_OBJ_RENDERMODE];
		//LoadedUcodeMap[S2DEX_OBJ_RECTANGLE_R] = GFXInstructionUcode1[S2DEX_OBJ_RECTANGLE_R];
		LoadedUcodeMap[S2DEX_RDPHALF_0] = GFXInstructionUcode1[S2DEX_RDPHALF_0];
		status.bUseModifiedUcodeMap = true;
		break;
	case 14: // OgreBattle Background
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode5, sizeof(UcodeMap));
		LoadedUcodeMap[0xda] = DLParser_OgreBatter64BG;
		LoadedUcodeMap[0xdc] = RSP_S2DEX_OBJ_MOVEMEM;
		status.bUseModifiedUcodeMap = true;
		break;
	case 15: // Ucode 0 with Sprite2D
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode0, sizeof(UcodeMap));
		LoadedUcodeMap[RSP_SPRITE2D_BASE] = RSP_GBI_Sprite2DBase;
		LoadedUcodeMap[RSP_SPRITE2D_SCALEFLIP] = RSP_GBI1_Sprite2DScaleFlip;
		LoadedUcodeMap[RSP_SPRITE2D_DRAW] = RSP_GBI0_Sprite2DDraw;
		status.bUseModifiedUcodeMap = true;
		break;
	case 16: // Star War, Shadow Of Empire
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode0, sizeof(UcodeMap));
		LoadedUcodeMap[4]=RSP_Vtx_ShadowOfEmpire;
		status.bUseModifiedUcodeMap = true;
		break;
	case 17: // Ucode 0 with Sprite2D, Puzzle Master 64
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode0, sizeof(UcodeMap));
		LoadedUcodeMap[RSP_SPRITE2D_BASE] = RSP_GBI_Sprite2D_PuzzleMaster64;
		LoadedUcodeMap[RSP_SPRITE2D_SCALEFLIP] = RSP_GBI1_Sprite2DScaleFlip;
		LoadedUcodeMap[RSP_SPRITE2D_DRAW] = RSP_GBI0_Sprite2DDraw;
		status.bUseModifiedUcodeMap = true;
		break;
	default:
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode5, sizeof(UcodeMap));
		status.bUseModifiedUcodeMap = true;
		break;
	}

#ifdef _DEBUG
	if( logMicrocode )
		TRACE1("Using ucode %d", ucode);
#endif
}

void RSP_SetUcode(int ucode, uint32 ucStart, uint32 ucDStart, uint32 ucSize)
{
	if( status.ucodeHasBeenSet && gRSP.ucode == ucode )
		return;

	status.ucodeHasBeenSet = true;

	if( ucode < 0 )
		ucode = 5;

	RDP_SetUcodeMap(ucode);
	if( status.bUseModifiedUcodeMap )
	{
		currentUcodeMap = &LoadedUcodeMap[0];
	}
	else
	{
		currentUcodeMap = *ucodeMaps[ucode];
	}

	gRSP.vertexMult = vertexMultVals[ucode];

	//if( gRSP.ucode != ucode )	DebuggerAppendMsg("Set to ucode: %d", ucode);
	gRSP.ucode = ucode;

	lastUcodeInfo.used = true;
	if( ucStart == 0 )
	{
		lastUcodeInfo.ucStart = g_pOSTask->t.ucode;
		lastUcodeInfo.ucDStart = g_pOSTask->t.ucode_data;
		lastUcodeInfo.ucSize = g_pOSTask->t.ucode_size;
	}
	else
	{
		lastUcodeInfo.ucStart = ucStart;
		lastUcodeInfo.ucDStart = ucDStart;
		lastUcodeInfo.ucSize = ucSize;
	}
}

//*****************************************************************************
//
//
//
//
//*****************************************************************************

//*****************************************************************************
//
//*****************************************************************************
static uint32 DLParser_IdentifyUcodeFromString( const CHAR * str_ucode )
{
	const CHAR str_ucode0[] = "RSP SW Version: 2.0";
	const CHAR str_ucode1[] = "RSP Gfx ucode ";

	if ( strnicmp( str_ucode, str_ucode0, strlen(str_ucode0) ) == 0 )
	{
		return 0;
	}

	if ( strnicmp( str_ucode, str_ucode1, strlen(str_ucode1) ) == 0 )
	{
		if( strstr(str_ucode,"1.") != 0 )
		{
			if( strstr(str_ucode,"S2DEX") != 0 )
			{
				return 7;
			}
			else
				return 1;
		}
		else if( strstr(str_ucode,"2.") != 0 )
		{
			if( strstr(str_ucode,"S2DEX") != 0 )
			{
				return 3;
			}
			else
				return 5;
		}
	}

	return 5;
}

//*****************************************************************************
//
//*****************************************************************************
static uint32 DLParser_IdentifyUcode( uint32 crc_size, uint32 crc_800, char* str )
{
	for ( uint32 i = 0; i < sizeof(g_UcodeData)/sizeof(UcodeData); i++ )
	{
#ifdef _DEBUG
		if ( crc_800 == g_UcodeData[i].crc_800 )
		{
			if( strlen(str)==0 || strcmp(g_UcodeData[i].ucode_name, str) == 0 ) 
			{
				TRACE0(g_UcodeData[i].ucode_name);
			}
			else
			{
				DebuggerAppendMsg("Incorrect description for this ucode:\n%x, %x, %s",crc_800, crc_size, str);
			}
			status.bUcodeIsKnown = TRUE;
			gRSP.bNearClip = !g_UcodeData[i].non_nearclip;
			gRSP.bRejectVtx = g_UcodeData[i].reject;
			DebuggerAppendMsg("Identify ucode = %d, crc = %08X, %s", g_UcodeData[i].ucode, crc_800, str);
			return g_UcodeData[i].ucode;
		}
#else
		if ( crc_800 == g_UcodeData[i].crc_800 )
		{
			status.bUcodeIsKnown = TRUE;
			gRSP.bNearClip = !g_UcodeData[i].non_nearclip;
			gRSP.bRejectVtx = g_UcodeData[i].reject;
			return g_UcodeData[i].ucode;
		}
#endif
	}

#ifdef _DEBUG
	{
		static bool warned = false;
		if( warned == false )
		{
			warned = true;
			TRACE0("Can not identify ucode for this game");
		}
	}
#endif
	gRSP.bNearClip = false;
	gRSP.bRejectVtx = false;
	status.bUcodeIsKnown = FALSE;
	return ~0;
}

uint32 DLParser_CheckUcode(uint32 ucStart, uint32 ucDStart, uint32 ucSize, uint32 ucDSize)
{
	// Check the used ucode table first
	int usedUcodeIndex = 0;
	for( usedUcodeIndex=0; usedUcodeIndex<maxUsedUcodes; usedUcodeIndex++ )
	{
		if( UsedUcodes[usedUcodeIndex].used == false )
		{
			break;
		}

		if( UsedUcodes[usedUcodeIndex].ucStart == ucStart && UsedUcodes[usedUcodeIndex].ucSize == ucSize &&
			UsedUcodes[usedUcodeIndex].ucDStart == ucDStart /*&& UsedUcodes[usedUcodeIndex].ucDSize == ucDSize*/ )
		{
#ifdef _DEBUG
			if( gRSP.ucode != UsedUcodes[usedUcodeIndex].ucode && logMicrocode)
			{
				DebuggerAppendMsg("Check, ucode = %d, crc = %08X, %s", UsedUcodes[usedUcodeIndex].ucode, 
					UsedUcodes[usedUcodeIndex].crc_800 , UsedUcodes[usedUcodeIndex].rspstr);
			}
#endif
			lastUcodeInfo.ucStart = ucStart;
			lastUcodeInfo.used = true;
			lastUcodeInfo.ucDStart = ucDStart;
			lastUcodeInfo.ucSize = ucSize;
			return UsedUcodes[usedUcodeIndex].ucode;
		}
	}

	uint32 base = ucDStart & 0x1fffffff;
	CHAR str[300] = "";
	for ( uint32 i = 0; i < 0x1000; i++ )
	{
		
		if ( g_pRDRAMs8[ base + ((i+0) ^ 3) ] == 'R' &&
			 g_pRDRAMs8[ base + ((i+1) ^ 3) ] == 'S' &&
			 g_pRDRAMs8[ base + ((i+2) ^ 3) ] == 'P' )
		{
			CHAR * p = str;
			while ( g_pRDRAMs8[ base + (i ^ 3) ] >= ' ')
			{
				*p++ = g_pRDRAMs8[ base + (i ^ 3) ];
				i++;
			}
			*p++ = 0;
			break;
		}
	}

	//if ( strcmp( str, gLastMicrocodeString ) != 0 )
	{
		uint32 size = ucDSize;
		base = ucStart & 0x1fffffff;

		uint32 crc_size = ComputeCRC32( 0, &g_pRDRAMu8[ base ], 8);//size );
		uint32 crc_800 = ComputeCRC32( 0, &g_pRDRAMu8[ base ], 0x800 );
		uint32 ucode;
		ucode = DLParser_IdentifyUcode( crc_size, crc_800, str );
		if ( ucode == ~0 )
		{
#ifdef _DEBUG
			static bool warned=false;
			//if( warned == false )
			{
				CHAR message[300];

				sprintf(message, "Unable to find ucode to use for\n\n"
								  "%s\n"
								  "CRCSize: 0x%08x\n\n"
								  "CRC800: 0x%08x\n"
								  "Please mail rice1964@yahoo.com with the contents of c:\\ucodes.txt",
						str, crc_size, crc_800);
				TRACE0(message);
				ErrorMsg(message);
				warned = true;
			}
#endif
			ucode = DLParser_IdentifyUcodeFromString(str);
			if ( ucode == ~0 )
			{
				ucode=5;
			}
		}

		//DLParser_SetuCode( ucode );
		
#ifdef _DEBUG
		{
			static bool warned=false;
			if( warned == false )
			{
				warned = true;
				if( strlen(str) == 0 )
					DebuggerAppendMsg("Can not find RSP string in the DLIST, CRC800: 0x%08x, CRCSize: 0x%08x", crc_800, crc_size);
				else
					TRACE0(str);
			}
		}
#endif
		strcpy( gLastMicrocodeString, str );

		if( usedUcodeIndex >= MAX_UCODE_INFO )
		{
			usedUcodeIndex = rand()%MAX_UCODE_INFO;
		}

		UsedUcodes[usedUcodeIndex].ucStart = ucStart;
		UsedUcodes[usedUcodeIndex].ucSize = ucSize;
		UsedUcodes[usedUcodeIndex].ucDStart = ucDStart;
		UsedUcodes[usedUcodeIndex].ucDSize = ucDSize;
		UsedUcodes[usedUcodeIndex].ucode = ucode;
		UsedUcodes[usedUcodeIndex].crc_800 = crc_800;
		UsedUcodes[usedUcodeIndex].crc_size = crc_size;
		UsedUcodes[usedUcodeIndex].used = true;
		strcpy( UsedUcodes[usedUcodeIndex].rspstr, str );

		TRACE2("New ucode has been detected:\n%s, ucode=%d", str, ucode);
	
		return ucode;
	}
	//else
	//{
	//	return 1;	//fix me, return current ucode
	//}
}

extern int dlistMtxCount;
extern bool bHalfTxtScale;
extern BOOL _INPUT_IsIngameMenuWaiting();
extern BOOL _INPUT_UpdatePaks();
extern BOOL _INPUT_UpdateControllerStates();
extern void _INPUT_RumblePause(bool bPause);
extern "C" void ReInitVirtualDynaMemory(boolean charge);
extern int TextureMode;
extern bool FrameSkip;

void DLParser_Process(OSTask * pTask)
{
	static int skipframe=0;

	dlistMtxCount = 0;
	bHalfTxtScale = false;

	if( FrameSkip )
	{
		skipframe++;
		if(skipframe%2)
		{
			TriggerDPInterrupt();
			return;
		}
	}

	if( currentRomOptions.N64RenderToTextureEmuType != TXT_BUF_NONE && defaultRomOptions.bSaveVRAM )
	{
		CGraphicsContext::g_pGraphicsContext->CheckTxtrBufsCRCInRDRAM();
	}

	StartProfiler(PROFILE_ALL);
	g_pOSTask = pTask;
	
	DebuggerPauseCountN( NEXT_DLIST );
	status.bRDPHalted = FALSE;
	status.gRDPTime = timeGetTime();
	status.gDlistCount++;

	uint32 dwPC;
	uint32 word0;
	uint32 word1;

	//if (status.bUcodeIsKnown == FALSE)
	if ( lastUcodeInfo.ucStart != (uint32)(pTask->t.ucode) )
	{
		//uint32 ucode = DLParser_CheckUcode(pTask->t.ucode, pTask->t.ucode_data, pTask->t.ucode_size, pTask->t.data_size);
		uint32 ucode = DLParser_CheckUcode(pTask->t.ucode, pTask->t.ucode_data, pTask->t.ucode_size, pTask->t.ucode_data_size);
		RSP_SetUcode(ucode, pTask->t.ucode, pTask->t.ucode_data, pTask->t.ucode_size);
		DEBUGGER_PAUSE_AND_DUMP(NEXT_SWITCH_UCODE,{DebuggerAppendMsg("Pause at switching ucode");});
	}

	// Initialize stack
	status.bN64FrameBufferIsUsed = false;
	g_dwPCindex=0;
	g_dwPCStack[g_dwPCindex].addr = (uint32)pTask->t.data_ptr;
	g_dwPCStack[g_dwPCindex].limit = ~0;
	DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((g_dwPCStack[g_dwPCindex].addr == 0 && pauseAtNext && eventToPause==NEXT_UNKNOWN_OP),
			{DebuggerAppendMsg("Start Task without DLIST: ucode=%08X, data=%08X", (uint32)pTask->t.ucode, (uint32)pTask->t.ucode_data);});


	// Check if we need to purge
	if (status.gRDPTime - status.lastPurgeTimeTime > 5000)
	{
		gTextureManager.PurgeOldTextures();
		status.lastPurgeTimeTime = status.gRDPTime;
	}

	status.dwNumDListsCulled = 0;
	status.dwNumTrisRendered = 0;
	status.dwNumTrisClipped = 0;
	status.dwNumVertices = 0;

	LOG_DL("DP: Firing up RDP!");

	try
	{
		if ( CRender::g_pRender != NULL)
		{
			if( g_curRomInfo.bForceScreenClear && CGraphicsContext::needCleanScene )
			{
				CRender::g_pRender->ClearBuffer(true,true);
				CGraphicsContext::needCleanScene = false;
			}
			//CRender::g_pRender->ClearBuffer(true, true);

			SetVIScales();
			CRender::g_pRender->RenderReset();

			//for( int i=0; i<8; i++ )
			//{
			//	memset(&gRDP.tiles[i], 0, sizeof(Tile));
			//}

			CRender::g_pRender->BeginRendering();
			CRender::g_pRender->SetViewport(0, 0, windowSetting.uViWidth, windowSetting.uViHeight, 0x3FF);

#ifdef _DEBUG
			if( debuggerEnableZBuffer )
			{
				CRender::g_pRender->ZBufferEnable( TRUE );
			}
			else
			{
				CRender::g_pRender->ZBufferEnable( FALSE );
			}
#endif

			// Process the entire display list in one go
			while (!status.bRDPHalted)
			{
				if( options.bWinFrameMode )
				{
					CRender::g_pRender->SetFillMode(RICE_FILLMODE_WINFRAME );
				}
				else
				{
					CRender::g_pRender->SetFillMode(RICE_FILLMODE_SOLID);
				}
#ifdef _DEBUG
				DEBUGGER_PAUSE_COUNT_N(NEXT_UCODE);
				if( debuggerPause )
				{
					DebuggerPause();
				}
#endif
				// Current PC is the last value on the stack
				dwPC = g_dwPCStack[g_dwPCindex].addr;
				word0 = g_pRDRAMu32[(dwPC>>2)+0];
				word1 = g_pRDRAMu32[(dwPC>>2)+1];

	#ifdef _DEBUG
				if (dwPC > g_dwRamSize)
				{
					DebuggerAppendMsg("Error: dwPC is %08X", dwPC );
					break;
				}
	#endif

				g_dwPCStack[g_dwPCindex].addr += 8;

				StartProfiler(PROFILE_UCODE_PARSER);
				LOG_DL("0x%08x: %08x %08x %-10s", 
					dwPC, word0, word1, (gRSP.ucode!=5&&gRSP.ucode!=10)?ucodeNames_GBI1[(word0>>24)]:ucodeNames_GBI2[(word0>>24)]);

				status.gUcodeCount++;
				currentUcodeMap[word0>>24](word0, word1);
				StopProfiler(PROFILE_UCODE_PARSER);

				// Check limit
				if (!status.bRDPHalted)
				{
					g_dwPCStack[g_dwPCindex].limit--;
					if (g_dwPCStack[g_dwPCindex].limit == ~0)
					{
						LOG_DL("**EndDLInMem");
						// If we're here, then we musn't have finished with the display list yet
						// Check if this is the lasy display list in the sequence
						if(g_dwPCindex == 0)
						{
							status.bRDPHalted = TRUE;
						}
						else
						{
							g_dwPCindex--;
						}
					}	

				}
			}

			CRender::g_pRender->EndRendering();
		}
	}
	catch(...)
	{
		TRACE0("Unknown exception happens in ProcessDList");
	}

	TriggerDPInterrupt();
	StopProfiler(PROFILE_ALL);

	if (_INPUT_IsIngameMenuWaiting())
	{
		_INPUT_RumblePause(true);
		
		ReInitVirtualDynaMemory(false);
		RunIngameMenu();
		options.forceTextureFilter=TextureMode;
		_INPUT_UpdatePaks();//added by freakdave
		_INPUT_UpdateControllerStates();//added by freakdave
		ReInitVirtualDynaMemory(true);
		
		_INPUT_RumblePause(false);
	}
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//                   Util Functions                     //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void RDP_NOIMPL_Real(LPCTSTR op, uint32 word0, uint32 word1) 
{
#ifdef _DEBUG
	if( logUnknownUcode )
	{
		TRACE0("Stack Trace");
		for( int i=0; i<g_dwPCindex; i++ )
		{
			DebuggerAppendMsg("  %08X", g_dwPCStack[i].addr);
		}
		uint32 dwPC = g_dwPCStack[g_dwPCindex].addr-8;
		DebuggerAppendMsg("PC=%08X",dwPC);
		DebuggerAppendMsg(op, word0, word1);
	}
	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_UNKNOWN_OP, {TRACE0("Paused at unimplemented ucode\n");})
#endif
}

void RDP_NOIMPL_WARN(LPCTSTR op)
{
#ifdef _DEBUG
	if(logUnknownUcode)
	{
		TRACE0(op);
	}
#endif
}


void RSP_GBI1_Noop(uint32 word0, uint32 word1)
{
}


void RDP_GFX_PopDL()
{
	LOG_DL("Returning from DisplayList: level=%d", g_dwPCindex+1);
	LOG_DL("############################################");
	LOG_DL("/\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\");
	LOG_DL("");

	if (g_dwPCindex < 0)
	{
		LOG_DL("EndDisplayList: Too many EndDL calls - ignoring");
		status.bRDPHalted = TRUE;			// Stop, damn it!
		return;
	}

	// Check if this is the last display list in the sequence
	if(g_dwPCindex == 0)
	{
		LOG_DL("EndDisplayList: The last one, halt RSP");
		status.bRDPHalted = TRUE;
		g_dwPCindex=-1;
		return;
	}

	// If we're here, then we musn't have finished with the display list yet
	g_dwPCindex--;
}

uint32 CalcalateCRC(uint32* srcPtr, uint32 srcSize)
{
	uint32 crc=0;
	for( uint32 i=0; i<srcSize; i++ )
	{
		crc += srcPtr[i];
	}
	return crc;
}


void RSP_GFX_InitGeometryMode()
{
	bool bCullFront		= (gRDP.geometryMode & G_CULL_FRONT) ? true : false;
	bool bCullBack		= (gRDP.geometryMode & G_CULL_BACK) ? true : false;
	
	BOOL bShade			= (gRDP.geometryMode & G_SHADE) ? TRUE : FALSE;
	BOOL bShadeSmooth	= (gRDP.geometryMode & G_SHADING_SMOOTH) ? TRUE : FALSE;
	
	BOOL bFog			= (gRDP.geometryMode & G_FOG) ? TRUE : FALSE;
	bool bTextureGen	= (gRDP.geometryMode & G_TEXTURE_GEN) ? true : false;

	bool bLighting      = (gRDP.geometryMode & G_LIGHTING) ? true : false;
	BOOL bZBuffer		= (gRDP.geometryMode & G_ZBUFFER)	? TRUE : FALSE;	

	if( bCullFront && bCullBack ) // should never cull front
	{
		//TRACE0("Warning: both front and back face are culled, don't do for front face");
		bCullFront = false;
	}
	CRender::g_pRender->SetCullMode(bCullFront, bCullBack);
	
	if (bShade && bShadeSmooth)		CRender::g_pRender->SetShadeMode( SHADE_SMOOTH );
	else							CRender::g_pRender->SetShadeMode( SHADE_FLAT );
	
	CRender::g_pRender->SetFogEnable( bFog );
	SetTextureGen(bTextureGen);

	SetLighting( bLighting );
	CRender::g_pRender->ZBufferEnable( bZBuffer );
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//                   DP Ucodes                          //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void DLParser_SetKeyGB(uint32 word0, uint32 word1)
{
	DP_Timing(DLParser_SetKeyGB);
	static BOOL bWarned = FALSE;

	LOG_DL("    SetKeyGB (not implemented)");
	gRDP.keyB = (word1>>8)&0xFF;
	gRDP.keyG = (word1>>24)&0xFF;
	gRDP.keyA = (gRDP.keyR+gRDP.keyG+gRDP.keyB)/3;
	gRDP.fKeyA = gRDP.keyA/255.0f;
	if (!bWarned)
	{
		RSP_RDP_NOIMPL("RDP: SetKeyGB 0x%08x 0x%08x", word0, word1);
		bWarned = TRUE;
	}

}
void DLParser_SetKeyR(uint32 word0, uint32 word1)
{
	DP_Timing(DLParser_SetKeyR);
	static BOOL bWarned = FALSE;

	LOG_DL("    SetKeyR (not implemented)");
	gRDP.keyR = (word1>>8)&0xFF;
	gRDP.keyA = (gRDP.keyR+gRDP.keyG+gRDP.keyB)/3;
	gRDP.fKeyA = gRDP.keyA/255.0f;
	if (!bWarned)
	{
		RSP_RDP_NOIMPL("RDP: SetKeyR 0x%08x 0x%08x", word0, word1);
		bWarned = TRUE;
	}
}

int g_convk0,g_convk1,g_convk2,g_convk3,g_convk4,g_convk5;
float g_convc0,g_convc1,g_convc2,g_convc3,g_convc4,g_convc5;
void DLParser_SetConvert(uint32 word0, uint32 word1)
{
	DP_Timing(DLParser_SetConvert);

	int temp;

	temp = (word0>>13)&0x1FF;
	g_convk0 = temp>0xFF ? -(temp-0x100) : temp;

	temp = (word0>>4)&0x1FF;
	g_convk1 = temp>0xFF ? -(temp-0x100) : temp;

	temp = word0&0xF;
	temp = (temp<<5)|((word1>>27)&0x1F);
	g_convk2 = temp>0xFF ? -(temp-0x100) : temp;

	temp = (word1>>18)&0x1FF;
	g_convk3 = temp>0xFF ? -(temp-0x100) : temp;

	temp = (word1>>9)&0x1FF;
	g_convk4 = temp>0xFF ? -(temp-0x100) : temp;

	temp = word1&0x1FF;
	g_convk5 = temp>0xFF ? -(temp-0x100) : temp;

	g_convc0 = g_convk5/255.0f+1.0f;
	g_convc1 = g_convk0/255.0f*g_convc0;
	g_convc2 = g_convk1/255.0f*g_convc0;
	g_convc3 = g_convk2/255.0f*g_convc0;
	g_convc4 = g_convk3/255.0f*g_convc0;
}
void DLParser_SetPrimDepth(uint32 word0, uint32 word1)
{
	DP_Timing(DLParser_SetPrimDepth);
	uint32 dwZ  = (word1 >> 16) & 0xFFFF;
	uint32 dwDZ = (word1      ) & 0xFFFF;

	LOG_DL("SetPrimDepth: 0x%08x 0x%08x - z: 0x%04x dz: 0x%04x",
		word0, word1, dwZ, dwDZ);
	
	SetPrimitiveDepth(dwZ, dwDZ);
	DEBUGGER_PAUSE(NEXT_SET_PRIM_COLOR);
}

void DLParser_RDPSetOtherMode(uint32 word0, uint32 word1)
{
	DP_Timing(DLParser_RDPSetOtherMode);
	gRDP.otherMode._u32[1] = word0;	// High
	gRDP.otherMode._u32[0] = word1;	// Low

	if( gRDP.otherModeH != (word0 & 0x0FFFFFFF) )
	{
		gRDP.otherModeH = (word0 & 0x0FFFFFFF);

		uint32 dwTextFilt  = (gRDP.otherModeH>>RSP_SETOTHERMODE_SHIFT_TEXTFILT)&0x3;
		CRender::g_pRender->SetTextureFilter(dwTextFilt<<RSP_SETOTHERMODE_SHIFT_TEXTFILT);
	}

	if( gRDP.otherModeL != word1 )
	{
		if( (gRDP.otherModeL&ZMODE_DEC) != (word1&ZMODE_DEC) )
		{
			if( (word1&ZMODE_DEC) )
				CRender::g_pRender->SetZBias( 2 );
			else
				CRender::g_pRender->SetZBias( 0 );
		}

		gRDP.otherModeL = word1;

		BOOL bZCompare		= (gRDP.otherModeL & Z_COMPARE)			? TRUE : FALSE;
		BOOL bZUpdate		= (gRDP.otherModeL & Z_UPDATE)			? TRUE : FALSE;

		CRender::g_pRender->SetZCompare( bZCompare );
		CRender::g_pRender->SetZUpdate( bZUpdate );

		uint32 dwAlphaTestMode = (gRDP.otherModeL >> RSP_SETOTHERMODE_SHIFT_ALPHACOMPARE) & 0x3;

		if ((dwAlphaTestMode) != 0)
			CRender::g_pRender->SetAlphaTestEnable( TRUE );
		else
			CRender::g_pRender->SetAlphaTestEnable( FALSE );
	}

	uint16 blender = gRDP.otherMode.blender;
	RDP_BlenderSetting &bl = *(RDP_BlenderSetting*)(&(blender));
	if( bl.c1_m1a==3 || bl.c1_m2a == 3 || bl.c2_m1a == 3 || bl.c2_m2a == 3 )
	{
		gRDP.bFogEnableInBlender = true;
	}
	else
	{
		gRDP.bFogEnableInBlender = false;
	}
}



void DLParser_RDPLoadSync(uint32 word0, uint32 word1)	
{ 
	DP_Timing(DLParser_RDPLoadSync);
	LOG_DL("LoadSync: (Ignored)"); 
}

void DLParser_RDPPipeSync(uint32 word0, uint32 word1)	
{ 
	DP_Timing(DLParser_RDPPipeSync);
	LOG_DL("PipeSync: (Ignored)"); 
}
void DLParser_RDPTileSync(uint32 word0, uint32 word1)	
{ 
	DP_Timing(DLParser_RDPTileSync);
	LOG_DL("TileSync: (Ignored)"); 
}

void DLParser_RDPFullSync(uint32 word0, uint32 word1)
{ 
	DP_Timing(DLParser_RDPFullSync);
	TriggerDPInterrupt();
}

void DLParser_SetScissor(uint32 word0, uint32 word1)
{
	DP_Timing(DLParser_SetScissor);

	ScissorType tempScissor;
	// The coords are all in 8:2 fixed point
	tempScissor.x0   = (word0>>12)&0xFFF;
	tempScissor.y0   = (word0>>0 )&0xFFF;
	tempScissor.mode = (word1>>24)&0x03;
	tempScissor.x1   = (word1>>12)&0xFFF;
	tempScissor.y1   = (word1>>0 )&0xFFF;

	tempScissor.left	= tempScissor.x0/4;
	tempScissor.top		= tempScissor.y0/4;
	tempScissor.right	= tempScissor.x1/4;
	tempScissor.bottom	= tempScissor.y1/4;

	if( options.bEnableHacks )
	{
		if( g_CI.dwWidth == 0x200 && tempScissor.right == 0x200 )
		{
			uint32 width = *g_GraphicsInfo.VI_WIDTH_REG & 0xFFF;

			if( width != 0x200 )
			{
				// Hack for RE2
				tempScissor.bottom = tempScissor.right*tempScissor.bottom/width;
				tempScissor.right = width;
			}

		}
	}

	if( gRDP.scissor.left != tempScissor.left || gRDP.scissor.top != tempScissor.top ||
		gRDP.scissor.right != tempScissor.right || gRDP.scissor.bottom != tempScissor.bottom )
	{
		memcpy(&(gRDP.scissor), &tempScissor, sizeof(ScissorType) );
		if( !status.bHandleN64TextureBuffer )
			SetVIScales();

		if(  options.enableHackForGames == HACK_FOR_SUPER_BOWLING && g_CI.dwAddr%0x100 != 0 )
		{
			// right half screen
			gRDP.scissor.left += 160;
			gRDP.scissor.right += 160;
			CRender::g_pRender->SetViewport(160, 0, 320, 240, 0xFFFF);
		}

		CRender::g_pRender->UpdateClipRectangle();
		CRender::g_pRender->UpdateScissor();
		CRender::g_pRender->SetViewportRender();
	}

	LOG_DL("SetScissor: x0=%d y0=%d x1=%d y1=%d mode=%d",
		gRDP.scissor.left, gRDP.scissor.top,
		gRDP.scissor.right, gRDP.scissor.bottom,
		gRDP.scissor.mode);

	DEBUGGER_IF_DUMP((logTextureBuffer) ,
	{DebuggerAppendMsg("SetScissor: x0=%d y0=%d x1=%d y1=%d mode=%d",
	gRDP.scissor.left, gRDP.scissor.top,
	gRDP.scissor.right, gRDP.scissor.bottom,
	gRDP.scissor.mode);}
	);
}


void DLParser_FillRect(uint32 word0, uint32 word1)
{ 
	DP_Timing(DLParser_FillRect);	// fix me
	status.primitiveType = PRIM_FILLRECT;

	if( status.bN64IsDrawingTextureBuffer && frameBufferOptions.bIgnore )
	{
		return;
	}

	if( options.enableHackForGames == HACK_FOR_MARIO_TENNIS )
	{
		DWORD dwPC = g_dwPCStack[g_dwPCindex].addr;		// This points to the next instruction
		DWORD word0 = *(DWORD *)(g_pRDRAMu8 + dwPC);
		DWORD word1 = *(DWORD *)(g_pRDRAMu8 + dwPC + 4);
		if( (word0>>24) == RDP_FILLRECT )
		{
			// Mario Tennis, a lot of FillRect ucodes, skip all of them
			while( (word0>>24) == RDP_FILLRECT )
			{
				dwPC += 8;
				word0 = *(DWORD *)(g_pRDRAMu8 + dwPC);
			}

			g_dwPCStack[g_dwPCindex].addr = dwPC;
			return;
		}
	}

	uint32 x0   = ((word1>>12)&0xFFF)/4;
	uint32 y0   = ((word1>>0 )&0xFFF)/4;
	uint32 x1   = ((word0>>12)&0xFFF)/4;
	uint32 y1   = ((word0>>0 )&0xFFF)/4;

	// Note, in some modes, the right/bottom lines aren't drawn

	LOG_DL("    (%d,%d) (%d,%d)", x0, y0, x1, y1);

	// TODO - In 1/2cycle mode, skip bottom/right edges!?
	if( gRDP.otherMode.cycle_type >= CYCLE_TYPE_COPY )
	{
		x1++;
		y1++;
	}

	DEBUGGER_IF_DUMP((logTextureBuffer) ,
	{
		DebuggerAppendMsg("FillRect: X0=%d, Y0=%d, X1=%d, Y1=%d, Color=0x%08X", x0, y0, x1, y1, gRDP.originalFillColor);
	});

	if( status.bHandleN64TextureBuffer && options.enableHackForGames == HACK_FOR_BANJO_TOOIE )
	{
		// Skip this
		return;
	}

	if (IsUsedAsDI(g_CI.dwAddr))
	{
		// Clear the Z Buffer
		if( x0!=0 || y0!=0 || windowSetting.uViWidth-x1>1 || windowSetting.uViHeight-y1>1)
		{
			if( options.enableHackForGames == HACK_FOR_GOLDEN_EYE )
			{
				// GoldenEye is using double zbuffer
				if( g_CI.dwAddr == g_ZI.dwAddr )
				{
					// The zbuffer is the upper screen
					D3DRECT rect={int(x0*windowSetting.fMultX),int(y0*windowSetting.fMultY),int(x1*windowSetting.fMultX),int(y1*windowSetting.fMultY)};
					CRender::g_pRender->ClearBuffer(false,true,rect);	//Check me
					LOG_DL("    Clearing ZBuffer");
				}
				else
				{
					// The zbuffer is the lower screen
					int h = (g_CI.dwAddr-g_ZI.dwAddr)/g_CI.dwWidth/2;
					D3DRECT rect={int(x0*windowSetting.fMultX),int((y0+h)*windowSetting.fMultY),int(x1*windowSetting.fMultX),int((y1+h)*windowSetting.fMultY)};
					CRender::g_pRender->ClearBuffer(false,true,rect);	//Check me
					LOG_DL("    Clearing ZBuffer");
				}
			}
			else
			{
				D3DRECT rect={x0,y0,int(x1*windowSetting.fMultX),int(y1*windowSetting.fMultY)};
				CRender::g_pRender->ClearBuffer(false,true,rect);	//Check me
				LOG_DL("    Clearing ZBuffer");
			}
		}
		else
		{
			CRender::g_pRender->ClearBuffer(false,true);	//Check me
			LOG_DL("    Clearing ZBuffer");
		}

		DEBUGGER_PAUSE_AND_DUMP_COUNT_N( NEXT_FLUSH_TRI,{TRACE0("Pause after FillRect: ClearZbuffer\n");});
		DEBUGGER_PAUSE_AND_DUMP_COUNT_N( NEXT_FILLRECT, {DebuggerAppendMsg("ClearZbuffer: X0=%d, Y0=%d, X1=%d, Y1=%d, Color=0x%08X", x0, y0, x1, y1, gRDP.originalFillColor);
		DebuggerAppendMsg("Pause after ClearZbuffer: Color=%08X\n", gRDP.originalFillColor);});

		if( g_curRomInfo.bEmulateClear )
		{
			// Emulating Clear, by write the memory in RDRAM
			WORD color = (WORD)gRDP.originalFillColor;
			uint32 pitch = g_CI.dwWidth<<1;
			uint32 base = (uint32)(g_pRDRAMu8 + g_CI.dwAddr);
			for( uint32 i =y0; i<y1; i++ )
			{
				for( uint32 j=x0; j<x1; j++ )
				{
					*(WORD*)((base+pitch*i+j)^2) = color;
				}
			}
		}
	}
	else if( status.bHandleN64TextureBuffer )
	{
		status.bCIBufferIsRendered = true;
		status.leftRendered = status.leftRendered<0 ? x0 : min((int)x0,status.leftRendered);
		status.topRendered = status.topRendered<0 ? y0 : min((int)y0,status.topRendered);
		status.rightRendered = status.rightRendered<0 ? x1 : max((int)x1,status.rightRendered);
		status.bottomRendered = status.bottomRendered<0 ? y1 : max((int)y1,status.bottomRendered);

		g_pTextureBufferInfo->maxUsedHeight = max(g_pTextureBufferInfo->maxUsedHeight,(int)y1);

		if( status.bDirectWriteIntoRDRAM || ( x0==0 && y0==0 && (x1 == g_pTextureBufferInfo->N64Width || x1 == g_pTextureBufferInfo->N64Width-1 ) ) )
		{
			if( g_pTextureBufferInfo->CI_Info.dwSize == TXT_SIZE_16b )
			{
				WORD color = (WORD)gRDP.originalFillColor;
				uint32 pitch = g_pTextureBufferInfo->N64Width<<1;
				uint32 base = (uint32)(g_pRDRAMu8 + g_pTextureBufferInfo->CI_Info.dwAddr);
				for( uint32 i =y0; i<y1; i++ )
				{
					for( uint32 j=x0; j<x1; j++ )
					{
						*(WORD*)((base+pitch*i+j)^2) = color;
					}
				}
			}
			else
			{
				BYTE color = (BYTE)gRDP.originalFillColor;
				uint32 pitch = g_pTextureBufferInfo->N64Width;
				uint32 base = (uint32)(g_pRDRAMu8 + g_pTextureBufferInfo->CI_Info.dwAddr);
				for( uint32 i=y0; i<y1; i++ )
				{
					for( uint32 j=x0; j<x1; j++ )
					{
						*(BYTE*)((base+pitch*i+j)^3) = color;
					}
				}
			}

			status.bFrameBufferDrawnByTriangles = false;
		}
		else
		{
			status.bFrameBufferDrawnByTriangles = true;
		}
		status.bFrameBufferDrawnByTriangles = true;

		if( !status.bDirectWriteIntoRDRAM )
		{
			status.bFrameBufferIsDrawn = true;

			//if( x0==0 && y0==0 && (x1 == g_pTextureBufferInfo->N64Width || x1 == g_pTextureBufferInfo->N64Width-1 ) && gRDP.fillColor == 0)
			//{
			//	CRender::g_pRender->ClearBuffer(true,false);
			//}
			//else
			{
				if( gRDP.otherMode.cycle_type == CYCLE_TYPE_FILL )
				{
					CRender::g_pRender->FillRect(x0, y0, x1, y1, gRDP.fillColor);
				}
				else
				{
					D3DCOLOR primColor = GetPrimitiveColor();
					CRender::g_pRender->FillRect(x0, y0, x1, y1, primColor);
				}
			}
		}

		DEBUGGER_PAUSE_AND_DUMP_COUNT_N( NEXT_FLUSH_TRI,{TRACE0("Pause after FillRect\n");});
		DEBUGGER_PAUSE_AND_DUMP_COUNT_N( NEXT_FILLRECT, {DebuggerAppendMsg("FillRect: X0=%d, Y0=%d, X1=%d, Y1=%d, Color=0x%08X", x0, y0, x1, y1, gRDP.originalFillColor);
		DebuggerAppendMsg("Pause after FillRect: Color=%08X\n", gRDP.originalFillColor);});
	}
	else
	{
		LOG_DL("    Filling Rectangle");
		if( frameBufferOptions.bSupportTxtBufs || frameBufferOptions.bCheckBackBufs )
		{
			status.bCIBufferIsRendered = true;
			status.leftRendered = status.leftRendered<0 ? x0 : min((int)x0,status.leftRendered);
			status.topRendered = status.topRendered<0 ? y0 : min((int)y0,status.topRendered);
			status.rightRendered = status.rightRendered<0 ? x1 : max((int)x1,status.rightRendered);
			status.bottomRendered = status.bottomRendered<0 ? y1 : max((int)y1,status.bottomRendered);
			CGraphicsContext::g_pGraphicsContext->CheckTxtrBufsWithNewCI(g_CI,gRDP.scissor.bottom,false);
		}

		if( gRDP.otherMode.cycle_type == CYCLE_TYPE_FILL )
		{
			if( !status.bHandleN64TextureBuffer || g_pTextureBufferInfo->CI_Info.dwSize == TXT_SIZE_16b )
			{
				CRender::g_pRender->FillRect(x0, y0, x1, y1, gRDP.fillColor);
			}
		}
		else
		{
			D3DCOLOR primColor = GetPrimitiveColor();
			//if( RGBA_GETALPHA(primColor) != 0 )
			{
				CRender::g_pRender->FillRect(x0, y0, x1, y1, primColor);
			}
		}
		DEBUGGER_PAUSE_AND_DUMP_COUNT_N( NEXT_FLUSH_TRI,{TRACE0("Pause after FillRect\n");});
		DEBUGGER_PAUSE_AND_DUMP_COUNT_N( NEXT_FILLRECT, {DebuggerAppendMsg("FillRect: X0=%d, Y0=%d, X1=%d, Y1=%d, Color=0x%08X", x0, y0, x1, y1, gRDP.originalFillColor);
		DebuggerAppendMsg("Pause after FillRect: Color=%08X\n", gRDP.originalFillColor);});
	}
}


void DLParser_SetZImg(uint32 word0, uint32 word1)
{
	DP_Timing(DLParser_SetZImg);
	LOG_DL("    Image: 0x%08x", RSPSegmentAddr(word1));

	uint32 dwFmt   = (word0>>21)&0x7;
	uint32 dwSiz   = (word0>>19)&0x3;
	uint32 dwWidth = (word0&0x0FFF) + 1;
	uint32 dwAddr = RSPSegmentAddr(word1);

	if( dwAddr != g_ZI_saves[0].CI_Info.dwAddr )
	{
		g_ZI_saves[1].CI_Info.dwAddr	= g_ZI.dwAddr;
		g_ZI_saves[1].CI_Info.dwFormat	= g_ZI.dwFormat;
		g_ZI_saves[1].CI_Info.dwSize	= g_ZI.dwSize;
		g_ZI_saves[1].CI_Info.dwWidth	= g_ZI.dwWidth;
		g_ZI_saves[1].updateAtFrame = g_ZI_saves[0].updateAtFrame;

		g_ZI_saves[0].CI_Info.dwAddr	= g_ZI.dwAddr	= dwAddr;
		g_ZI_saves[0].CI_Info.dwFormat	= g_ZI.dwFormat = dwFmt;
		g_ZI_saves[0].CI_Info.dwSize	= g_ZI.dwSize	= dwSiz;
		g_ZI_saves[0].CI_Info.dwWidth	= g_ZI.dwWidth	= dwWidth;
		g_ZI_saves[0].updateAtFrame		= status.gDlistCount;
	}
	else
	{
		g_ZI.dwAddr	= dwAddr;
		g_ZI.dwFormat = dwFmt;
		g_ZI.dwSize	= dwSiz;
		g_ZI.dwWidth	= dwWidth;
	}

	DEBUGGER_IF_DUMP((pauseAtNext) ,
	{DebuggerAppendMsg("SetZImg: Addr=0x%08X, Fmt:%s-%sb, Width=%d\n", 
	g_ZI.dwAddr, pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);}
	);

	DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG, 
		{
			DebuggerAppendMsg("Pause after SetZImg: Addr=0x%08X, Fmt:%s-%sb, Width=%d\n", 
				g_ZI.dwAddr, pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);
		}
	);
}

bool IsUsedAsDI(uint32 addr)
{
	if( addr == g_ZI_saves[0].CI_Info.dwAddr )
		return true;
	else if( addr == g_ZI_saves[1].CI_Info.dwAddr && status.gDlistCount - g_ZI_saves[1].updateAtFrame < 10 
		&& g_ZI_saves[1].CI_Info.dwAddr != 0 )
		return true;
	else
		return false;
}

void DLParser_SetCombine(uint32 word0, uint32 word1)
{
	DP_Timing(DLParser_SetCombine);
	uint32 dwMux0 = word0&0x00FFFFFF;
	uint32 dwMux1 = word1;
	CRender::g_pRender->SetMux(dwMux0, dwMux1);
}

void DLParser_SetFillColor(uint32 word0, uint32 word1)
{
	DP_Timing(DLParser_SetFillColor);
	gRDP.fillColor = Convert555ToRGBA((WORD)word1);
	gRDP.originalFillColor = word1;

	LOG_DL("    Color5551=0x%04x = 0x%08x", (WORD)word1, gRDP.fillColor);

}

void DLParser_SetFogColor(uint32 word0, uint32 word1)
{
	DP_Timing(DLParser_SetFogColor);
	uint32 dwRed		= (word1>>24)&0xFF;
	uint32 dwGreen	= (word1>>16)&0xFF;
	uint32 dwBlue	= (word1>>8)&0xFF;
	uint32 dwAlpha	= (word1)&0xFF;

	LOG_DL("    RGBA: %d %d %d %d", dwRed, dwGreen, dwBlue, dwAlpha);

	uint32 dwFogColor = word1;

	CRender::g_pRender->SetFogColor( dwRed, dwGreen, dwBlue, dwAlpha );	
}

void DLParser_SetBlendColor(uint32 word0, uint32 word1)
{
	DP_Timing(DLParser_SetBlendColor);
	uint32 dwRed		= (word1>>24)&0xFF;
	uint32 dwGreen	= (word1>>16)&0xFF;
	uint32 dwBlue	= (word1>>8)&0xFF;
	uint32 dwAlpha	= (word1)&0xFF;

	LOG_DL("    RGBA: %d %d %d %d", dwRed, dwGreen, dwBlue, dwAlpha);

	CRender::g_pRender->SetAlphaRef(dwAlpha);
}


void DLParser_SetPrimColor(uint32 word0, uint32 word1)
{
	DP_Timing(DLParser_SetPrimColor);
	uint32 dwM		= (word0>>8)&0xFF;
	uint32 dwL		= (word0)&0xFF;
	uint32 dwRed		= (word1>>24)&0xFF;
	uint32 dwGreen	= (word1>>16)&0xFF;
	uint32 dwBlue	= (word1>>8)&0xFF;
	uint32 dwAlpha	= (word1)&0xFF;

	LOG_DL("    M:%d L:%d RGBA: %d %d %d %d", dwM, dwL, dwRed, dwGreen, dwBlue, dwAlpha);

	SetPrimitiveColor( COLOR_RGBA(dwRed, dwGreen, dwBlue, dwAlpha), dwM, dwL);
	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_SET_PRIM_COLOR, DebuggerAppendMsg("SetPrimColor: M:%d L:%d RGBA: %d %d %d %d", dwM, dwL, dwRed, dwGreen, dwBlue, dwAlpha));
}

void DLParser_SetEnvColor(uint32 word0, uint32 word1)
{
	DP_Timing(DLParser_SetEnvColor);
	uint32 dwRed		= (word1>>24)&0xFF;
	uint32 dwGreen	= (word1>>16)&0xFF;
	uint32 dwBlue	= (word1>>8)&0xFF;
	uint32 dwAlpha	= (word1)&0xFF;
	LOG_DL("    RGBA: %d %d %d %d",
		dwRed, dwGreen, dwBlue, dwAlpha);

	SetEnvColor( COLOR_RGBA(dwRed, dwGreen, dwBlue, dwAlpha));
}


void RDP_DLParser_Process(void)
{
	status.bRDPHalted = FALSE;
	status.gRDPTime = timeGetTime();
	status.gDlistCount++;

	uint32 dwPC;
	uint32 word0;
	uint32 word1;

	uint32 start = *(g_GraphicsInfo.DPC_START_REG);
	uint32 end = *(g_GraphicsInfo.DPC_END_REG);

	g_dwPCindex=0;
	g_dwPCStack[g_dwPCindex].addr = start;
	g_dwPCStack[g_dwPCindex].limit = ~0;

	// Check if we need to purge
	if (status.gRDPTime - status.lastPurgeTimeTime > 5000)
	{
		gTextureManager.PurgeOldTextures();
		status.lastPurgeTimeTime = status.gRDPTime;
	}
	
	// Lock the graphics context here.
	CRender::g_pRender->SetFillMode(RICE_FILLMODE_SOLID);

	SetVIScales();

	CRender::g_pRender->RenderReset();
	CRender::g_pRender->BeginRendering();
	CRender::g_pRender->SetViewport(0, 0, windowSetting.uViWidth, windowSetting.uViHeight, 0x3FF);

	dwPC = g_dwPCStack[0].addr;
	while( dwPC < end )
	{
		word0 = g_pRDRAMu32[(dwPC>>2)+0];
		word1 = g_pRDRAMu32[(dwPC>>2)+1];
		
		g_dwPCStack[0].addr += 8;
		LOG_DL("%s: %08x - %08X", ucodeNames_GBI1[word0>>24], word0, word1);
		currentUcodeMap[word0>>24](word0, word1);
		dwPC = g_dwPCStack[0].addr;
	}

	CRender::g_pRender->EndRendering();

	status.bRDPHalted = TRUE;
}

void RDP_TriFill(uint32 word0, uint32 word1)
{
}

void RDP_TriFillZ(uint32 word0, uint32 word1)
{
}

void RDP_TriTxtr(uint32 word0, uint32 word1)
{
}

void RDP_TriTxtrZ(uint32 word0, uint32 word1)
{
}

void RDP_TriShade(uint32 word0, uint32 word1)
{
}

void RDP_TriShadeZ(uint32 word0, uint32 word1)
{
}

void RDP_TriShadeTxtr(uint32 word0, uint32 word1)
{
}

void RDP_TriShadeTxtrZ(uint32 word0, uint32 word1)
{
}

static int crc_table_empty = 1;
static ULONG crc_table[256];
static void make_crc_table(void);

static void make_crc_table()
{
  ULONG c;
  int n, k;
  ULONG poly;            /* polynomial exclusive-or pattern */
  /* terms of polynomial defining this crc (except x^32): */
  static const BYTE p[] = {0,1,2,4,5,7,8,10,11,12,16,22,23,26};

  /* make exclusive-or pattern from polynomial (0xedb88320L) */
  poly = 0L;
  for (n = 0; n < sizeof(p)/sizeof(BYTE); n++)
    poly |= 1L << (31 - p[n]);
 
  for (n = 0; n < 256; n++)
  {
    c = (ULONG)n;
    for (k = 0; k < 8; k++)
      c = (c & 1) ? (poly ^ (c >> 1)) : c >> 1;
    crc_table[n] = c;
  }
  crc_table_empty = 0;
}

/* ========================================================================= */
#define DO1(buf) crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);

/* ========================================================================= */
ULONG ComputeCRC32(ULONG crc, const BYTE *buf, UINT len)
{
    if (buf == NULL) return 0L;

    if (crc_table_empty)
      make_crc_table();

    crc = crc ^ 0xffffffffL;
    while (len >= 8)
    {
      DO8(buf);
      len -= 8;
    }
    if (len) do {
      DO1(buf);
    } while (--len);
    return crc ^ 0xffffffffL;
}
