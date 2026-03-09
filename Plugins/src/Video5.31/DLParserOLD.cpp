/*
Copyright (C) 2001 StrmnNrmn

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
const u32 maxUsedUcodes = sizeof(UsedUcodes)/sizeof(UcodeInfo);

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
};

DWORD vertexMultVals[] =
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

#define G_DMATRI	0x05
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
static MicrocodeData g_MicrocodeData[] = 
{
	//crc_size,	crc_800;

	// SGI U64 GFX SW TEAM: S Anderson, S Carr, H Cheng, K Luster, R Moore, N Pooley, A Srinivasan
	{0, 0x150c3ce8, 0x150c3ce8, "RSP SW Version: 2.0D, 04-01-96",}, // Super Mario 64
		// Note ucode 4 - no idea why this is so different!
	{4, 0x2b94276f, 0x2b94276f, "RSP SW Version: 2.0D, 04-01-96",}, // Wave Race 64 (v1.0)
	{4, 0xb1870454, 0xb1870454, "RSP SW Version: 2.0D, 04-01-96",}, // Star Wars - Shadows of the Empire (v1.0), 
	{0, 0x51671ae4, 0x51671ae4, "RSP SW Version: 2.0D, 04-01-96",}, // Pilot Wings 64, 
	{0, 0x67b5ac55, 0x67b5ac55, "RSP SW Version: 2.0D, 04-01-96",}, // Wibble, 
	{0, 0x64dc8104, 0x64dc8104, "RSP SW Version: 2.0D, 04-01-96",}, // Dark Rift, 
	{0, 0x309f363d, 0x309f363d, "RSP SW Version: 2.0D, 04-01-96",}, // Killer Instinct Gold, 
	{0, 0xfcb57e57, 0xfcb57e57, "RSP SW Version: 2.0D, 04-01-96",}, // Blast Corps, 
	{0, 0xb420f35a, 0xb420f35a, "RSP SW Version: 2.0D, 04-01-96",}, // Blast Corps, 
	{0, 0x6e26c1df, 0x7c98e9c2, "RSP SW Version: 2.0D, 04-01-96",}, 
	{2, 0xc02ac7bc, 0xc02ac7bc, "RSP SW Version: 2.0G, 09-30-96",}, // GoldenEye 007, 
	{0, 0xe5fee3bc, 0xe5fee3bc, "RSP SW Version: 2.0G, 09-30-96",}, // Aero Fighters Assault, 
	{0, 0x72109ec6, 0x72109ec6, "RSP SW Version: 2.0H, 02-12-97",}, // Duke Nukem 64, 
	{0, 0xf24a9a04, 0xf24a9a04, "RSP SW Version: 2.0H, 02-12-97",}, // Tetrisphere, 
	{15,0x700de42e, 0x700de42e, "RSP SW Version: 2.0H, 02-12-97",}, // Wipeout 64 (uses GBI1 too!), 
	{15,0x1b304a74, 0x1b304a74, "RSP SW Version: 2.0H, 02-12-97",}, // Flying Dragon, 
	{15,0xe4bb5ad8, 0xa7b2f704, "RSP SW Version: 2.0H, 02-12-97",}, // Silicon Valley, 
	{15,0xe4bb5ad8, 0x88202781, "RSP SW Version: 2.0H, 02-12-97",}, // Glover, 
	{0, 0xe466b5bd, 0xe466b5bd, "Unknown 0xe466b5bd, 0xe466b5bd",}, // Dark Rift, 
	{9, 0x7064a163, 0x7064a163, "Unknown 0x7064a163, 0x7064a163",}, // Perfect Dark (v1.0), 
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
	{5, 0x39e3e95a, 0x39e3e95a, "RSP Gfx ucode F3DEX       fifo 2.05  Yoshitaka Yasumoto 1998 Nintendo.",}, // Knife Edge - Nose Gunner, 
	{5, 0xd2913522, 0xd2913522, "RSP Gfx ucode F3DAM       fifo 2.05  Yoshitaka Yasumoto 1998 Nintendo.",}, // Hey You, Pikachu!, 
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
};


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//                     GFX State                        //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
// Multithread protection for our renderer
CCritSect g_RendererLock;
FiddledVtx * g_pVtxBase=NULL;

SetImgInfo g_TI = { G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, 0 };
SetImgInfo g_CI = { G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, 0 };
SetImgInfo g_DI = { G_IM_FMT_RGBA, G_IM_SIZ_16b, 1, 0 };

DList	g_dwPCStack[MAX_DL_STACK_SIZE];
int		g_dwPCindex= -1;

TMEMLoadMapInfo g_tmemLoadAddrMap[0x200];	// Totally 4KB TMEM

char *pszImgSize[4] = {"4", "8", "16", "32"};
const char *textluttype[4] = {"RGB16", "I16?", "RGBA16", "IA16"};
WORD	g_wRDPPal[0x200];
DWORD	g_dwRDPPalCrc[16];

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

HRESULT DLParser_Init()
{
	int i;

	status.gRDPTime = 0;
	status.gRDPFrame = 0;
	status.frameReadByCPU = FALSE;
	status.frameWriteByCPU = FALSE;
	status.SPCycleCount = 0;
	status.DPCycleCount = 0;

	status.bDetermineduCode = FALSE;
	status.lastPurgeTimeTime = status.gRDPTime;

	status.curRenderBuffer = NULL;
	status.curDisplayBuffer = NULL;
	status.curVIOriginReg = NULL;

	status.primitiveType = PRIM_TRI1;

	status.lastPurgeTimeTime = 0;		// Time textures were last purged

	status.UseLargerTile[0] = status.UseLargerTile[0] = false;
	status.LargerTileRealLeft[0] = status.LargerTileRealLeft[1] = 0;

	g_RendererLock.Lock();

	for( i=0; i<8; i++ )
	{
		memset(&gRDP.tiles[i], 0, sizeof(Tile));
	}
	memset(g_tmemLoadAddrMap, 0, sizeof(g_tmemLoadAddrMap));

	for( i=0; i<MAX_UCODE_INFO; i++ )
	{
		memset(&ucodeInfo[i], 0, sizeof(UcodeInfo));
	}

	status.useModifiedMap = false;
	status.ucodeHasBeenSet = false;
	
	// Check DKR for ucode 6
	char name[200];
	strcpy(name, g_curRomInfo.szGameName);
	g_RendererLock.Unlock();

	memset(&lastUcodeInfo, 0, sizeof(UcodeInfo));
	memset(&UsedUcodes, 0, sizeof(UsedUcodes));
	memset(&g_TmemFlag, 0, sizeof(g_TmemFlag));
	memset(&g_RecentCIInfo, 0, sizeof(RecentCIInfo)*3);
	extern RecentViOriginInfo g_RecentVIOriginInfo[3];
	memset(&g_RecentVIOriginInfo, 0, sizeof(RecentViOriginInfo)*3);

	status.UseLargerTile[0] = status.UseLargerTile[1] = false;
	status.LargerTileRealLeft[0] = status.LargerTileRealLeft[1] = 0;

	return S_OK;
}


HRESULT RDP_GFX_Reset()
{
	g_dwPCindex=-1;

	status.bDetermineduCode = FALSE;

	gTextureCache.DropTextures();

	return S_OK;
}


void RDP_Cleanup()
{
	if( g_bUsingFakeCI )
	{
		g_bUsingFakeCI = FALSE;
		SAFE_RELEASE(pfakeBufferSurf);
		SAFE_RELEASE( pfakeDepthBufferSurf );
		SAFE_RELEASE(pBackBufferSurf);
		SAFE_RELEASE(pBackDepthBufferSurf);
	}
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//                   Task Handling                      //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
void RDP_SetUcodeMap(int ucode)
{
	status.useModifiedMap = false;
	switch( ucode )
	{
	case 0:	// Mario and demos
		break;
	case 1:	// F3DEX GBI1
		break;
	case 2: // Golden Eye
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode0, sizeof(UcodeMap));
		//LoadedUcodeMap[9]=DLParser_GBI1_Sprite2DBase;
		//LoadedUcodeMap[0xaf]=DLParser_GBI1_LoadUCode;
		//LoadedUcodeMap[0xb0]=DLParser_GBI1_BranchZ;
		LoadedUcodeMap[0xb4]=DLParser_RDPHalf_1_0xb4_GoldenEye;
		status.useModifiedMap = true;
		break;
	case 3:	// S2DEX GBI2
		break;
	case 4:
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode0, sizeof(UcodeMap));
		LoadedUcodeMap[4]=DLParser_Vtx_WRUS;
		LoadedUcodeMap[0xb1]=DLParser_GBI1_Tri2;
		//LoadedUcodeMap[9]=DLParser_GBI1_Sprite2DBase;
		//LoadedUcodeMap[0xaf]=DLParser_GBI1_LoadUCode;
		//LoadedUcodeMap[0xb0]=DLParser_GBI1_BranchZ;
		//LoadedUcodeMap[0xb2]=DLParser_GBI1_ModifyVtx;
		status.useModifiedMap = true;
		break;
	case 5:	// F3DEX GBI2
		break;
	case 6: // DKR, Jet Force Gemini, Mickey
	case 11: // DKR, Jet Force Gemini, Mickey
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode0, sizeof(UcodeMap));
		LoadedUcodeMap[1]=DLParser_Mtx_DKR;
		LoadedUcodeMap[4]=DLParser_Vtx_DKR;
		if( ucode == 11 )	LoadedUcodeMap[4]=DLParser_Vtx_Gemini;
		LoadedUcodeMap[5]=DLParser_DMA_Tri_DKR;
		LoadedUcodeMap[7]=DLParser_DL_In_MEM_DKR;
		LoadedUcodeMap[0xbc]=DLParser_MoveWord_DKR;
		LoadedUcodeMap[0xbf]=DLParser_Set_Addr_Ucode6;
		//LoadedUcodeMap[9]=DLParser_GBI1_Sprite2DBase;
		//LoadedUcodeMap[0xb0]=DLParser_GBI1_BranchZ;
		//LoadedUcodeMap[0xb2]=DLParser_GBI1_ModifyVtx;
		status.useModifiedMap = true;
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
		LoadedUcodeMap[8]=DLParser_Nothing;
		LoadedUcodeMap[9]=DLParser_Nothing;
		LoadedUcodeMap[10]=DLParser_Nothing;
		LoadedUcodeMap[11]=DLParser_Nothing;
		LoadedUcodeMap[0x80]=DLParser_RS_Block;
		LoadedUcodeMap[0xb4]=DLParser_Ucode8_0xb4;
		LoadedUcodeMap[0xb5]=DLParser_Ucode8_0xb5;
		LoadedUcodeMap[0xb8]=DLParser_Ucode8_EndDL;
		LoadedUcodeMap[0xbc]=DLParser_Ucode8_0xbc;
		LoadedUcodeMap[0xbd]=DLParser_Ucode8_0xbd;
		LoadedUcodeMap[0xbe]=DLParser_RS_0xbe;
		LoadedUcodeMap[0xbF]=DLParser_Ucode8_0xbf;
		status.useModifiedMap = true;
		break;
	case 9:	// Perfect Dark
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode0, sizeof(UcodeMap));
		LoadedUcodeMap[4]=DLParser_Vtx_PD;
		LoadedUcodeMap[7]=DLParser_Set_Vtx_CI_PD;
		LoadedUcodeMap[0xb1]=DLParser_Tri4_PD;
		status.useModifiedMap = true;
		break;
	case 10: // Conker BFD
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode5, sizeof(UcodeMap));
		LoadedUcodeMap[1]=DLParser_Vtx_Conker;
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
		status.useModifiedMap = true;
		break;
	case 12: // Silicon Velley, Space Station
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode1, sizeof(UcodeMap));
		LoadedUcodeMap[0x01]=DLParser_Mtx_SiliconValley;
		status.useModifiedMap = true;
		break;
	case 13: // modified S2DEX
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode7, sizeof(UcodeMap));
		//LoadedUcodeMap[G_BG_1CYC] = GFXInstructionUcode1[G_BG_1CYC];
		LoadedUcodeMap[G_OBJ_RECTANGLE] = GFXInstructionUcode1[G_OBJ_RECTANGLE];
		LoadedUcodeMap[G_OBJ_SPRITE] = GFXInstructionUcode1[G_OBJ_SPRITE];
		//LoadedUcodeMap[G_OBJ_RENDERMODE] = GFXInstructionUcode1[G_OBJ_RENDERMODE];
		//LoadedUcodeMap[G_OBJ_RECTANGLE_R] = GFXInstructionUcode1[G_OBJ_RECTANGLE_R];
		LoadedUcodeMap[G_RDPHALF_0] = GFXInstructionUcode1[G_RDPHALF_0];
		status.useModifiedMap = true;
		break;
	case 14: // OgreBattle Background
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode5, sizeof(UcodeMap));
		LoadedUcodeMap[0xda] = DLParser_OgreBatter64BG;
		LoadedUcodeMap[0xdc] = RDP_S2DEX_OBJ_MOVEMEM;
		status.useModifiedMap = true;
		break;
	case 15: // Ucode 0 with Sprite2D
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode0, sizeof(UcodeMap));
		LoadedUcodeMap[G_SPRITE2D_BASE] = DLParser_GBI_Sprite2DBase;
		LoadedUcodeMap[G_SPRITE2D_SCALEFLIP] = DLParser_GBI1_Sprite2DScaleFlip;
		LoadedUcodeMap[G_SPRITE2D_DRAW] = DLParser_GBI0_Sprite2DDraw;
		status.useModifiedMap = true;
		break;
	default:
		memcpy( &LoadedUcodeMap, &GFXInstructionUcode5, sizeof(UcodeMap));
		status.useModifiedMap = true;
		break;
	}

#ifdef _DEBUG
	if( logUnknownOpcode )
		TRACE1("Using ucode %d", ucode);
#endif
}

void RDP_SetUcode(int ucode, u32 ucStart, u32 ucDStart, u32 ucSize)
{
	if( status.ucodeHasBeenSet && gRSP.ucode == ucode )
		return;

	status.ucodeHasBeenSet = true;

	if( ucode < 0 )
		ucode = 5;

	RDP_SetUcodeMap(ucode);
	if( status.useModifiedMap )
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
static u32 DLParser_IdentifyUcodeFromString( const CHAR * str_ucode )
{
	const CHAR str_ucode0[] = "RSP SW Version: 2.0";
	const CHAR str_ucode1[] = "RSP Gfx ucode ";

	//Super Mario 64,									0, 'RSP SW Version: 2.0D, 04-01-96'
	//The Legend of Zelda - Ocarina of Time (v1.0),	5, 'RSP Gfx ucode F3DZEX.NoN  fifo 2.06H Yoshitaka Yasumoto 1998 Nintendo.'
	//RSP Gfx ucode F3DLP.Rej     1.23 Yoshitaka Yasumoto Nintendo

	if ( strnicmp( str_ucode, str_ucode0, strlen(str_ucode0) ) == 0 )
	{
		return 0;
	}

	if ( strnicmp( str_ucode, str_ucode1, strlen(str_ucode1) ) == 0 )
	{
		// RSP Gfx ucode TYPE[.SPEC] [fifo] [VER] Yoshitaka Yasumoto...
		/*
		CHAR ucode_type[300];
		CHAR fifo[300];
		CHAR ucode_version[300];

		str_ucode += strlen(str_ucode1);

		int n = sscanf( str_ucode, "%s %s %s", ucode_type, fifo, ucode_version );

		if ( n < 3 )
		{
			strcpy( ucode_version, fifo );
		}

		DebuggerAppendMsg("Type is %s, Version %s", ucode_type, ucode_version );
		
		if ( ucode_version[0] == '2' )
		{
			return 5;
		}
		else if ( ucode_version[0] == '1' || ucode_version[0] == '0' )
		{
			return 1;
		}
		*/

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

	// Special case for DKR
	return 5;

}

//*****************************************************************************
//
//*****************************************************************************
#ifndef ARRAYSIZE
#define ARRAYSIZE(x)	(  sizeof( (x)  ) / sizeof(  (x)[0]  )   )
#endif

static uint32 DLParser_IdentifyUcode( u32 crc_size, u32 crc_800, char* str )
{
	for ( u32 i = 0; i < ARRAYSIZE(g_MicrocodeData); i++ )
	{
#ifdef _DEBUG
		if ( /*crc_size == g_MicrocodeData[i].crc_size &&*/ crc_800 == g_MicrocodeData[i].crc_800 )
		{
			if( strlen(str)==0 || strcmp(g_MicrocodeData[i].ucode_name, str) == 0 ) 
			{
				TRACE0(g_MicrocodeData[i].ucode_name);
			}
			else
			{
				DebuggerAppendMsg("Incorrect description for this ucode:\n%x, %x, %s",crc_800, crc_size, str);
			}
			status.bDetermineduCode = TRUE;
			gRSP.bNearClip = !g_MicrocodeData[i].non_nearclip;
			gRSP.bRejectVtx = g_MicrocodeData[i].reject;
			DebuggerAppendMsg("Identify ucode = %d, crc = %08X, %s", g_MicrocodeData[i].ucode, crc_800, str);
			return g_MicrocodeData[i].ucode;
		}
#else
		if ( crc_800 == g_MicrocodeData[i].crc_800 )
		{
			status.bDetermineduCode = TRUE;
			gRSP.bNearClip = !g_MicrocodeData[i].non_nearclip;
			gRSP.bRejectVtx = g_MicrocodeData[i].reject;
			return g_MicrocodeData[i].ucode;
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
	status.bDetermineduCode = FALSE;
	return ~0;
}

uint32 DLParser_CheckUcode(u32 ucStart, u32 ucDStart, u32 ucSize, u32 ucDSize)
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
			if( gRSP.ucode != UsedUcodes[usedUcodeIndex].ucode && logUnknownOpcode)
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

	u32 base = ucDStart & 0x1fffffff;
	CHAR str[300] = "";
	for ( u32 i = 0; i < 0x1000; i++ )
	{
		
		if ( g_ps8RamBase[ base + ((i+0) ^ 3) ] == 'R' &&
			 g_ps8RamBase[ base + ((i+1) ^ 3) ] == 'S' &&
			 g_ps8RamBase[ base + ((i+2) ^ 3) ] == 'P' )
		{
			CHAR * p = str;
			while ( g_ps8RamBase[ base + (i ^ 3) ] >= ' ')
			{
				*p++ = g_ps8RamBase[ base + (i ^ 3) ];
				i++;
			}
			*p++ = 0;
			break;
		}
	}

	//if ( strcmp( str, gLastMicrocodeString ) != 0 )
	{
		u32 size = ucDSize;
		base = ucStart & 0x1fffffff;

		u32 crc_size = daedalus_crc32( 0, &g_pu8RamBase[ base ], 8);//size );
		u32 crc_800 = daedalus_crc32( 0, &g_pu8RamBase[ base ], 0x800 );

		//DBGConsole_Msg(0, "Ucode is: 0x%08x, %s", crc_800, str );

		u32 ucode;
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
								  "CRC800: 0x%08x\n"
								  "CRCSize: 0x%08x\n\n"
								  "Please mail StrmnNrmn@emuhelp.com with the contents of c:\\ucodes.txt",
						str, crc_800, crc_size);
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

void SetVIScales();

void DLParser_Process(OSTask * pTask)
{
	static int skipframe=0;

	if( options.skipFrame )
	{
		if(skipframe>1)
		{
			skipframe = 0;
			TriggerDPInterrupt();
			return;
		}
		else
		{
			skipframe++;
		}
	}

	StartProfiler(PROFILE_ALL);
	g_pOSTask = pTask;
	
	DebuggerPauseCountN( NEXT_DLIST );
	StartRDP();
	status.gRDPTime = timeGetTime();
	status.gRDPFrame++;

	DWORD dwPC;
	DWORD dwCmd0;
	DWORD dwCmd1;

	//if (status.bDetermineduCode == FALSE)
	if ( lastUcodeInfo.ucStart != (u32)(pTask->t.ucode) )
	{
		u32 ucode = DLParser_CheckUcode(pTask->t.ucode, pTask->t.ucode_data, pTask->t.ucode_size, pTask->t.data_size);
		RDP_SetUcode(ucode, pTask->t.ucode, pTask->t.ucode_data, pTask->t.ucode_size);
		DEBUGGER_PAUSE_AND_DUMP(NEXT_SWITCH_UCODE,{DebuggerAppendMsg("Pause at switching ucode");});
	}

	// Initialize stack
	g_dwPCindex=0;
	g_dwPCStack[g_dwPCindex].addr = (u32)pTask->t.data_ptr;
	g_dwPCStack[g_dwPCindex].limit = ~0;
	DEBUGGER_PAUSE_AT_COND_AND_DUMP_COUNT_N((g_dwPCStack[g_dwPCindex].addr == 0 && pauseAtNext && eventToPause==NEXT_UNKNOWN_OP),
			{DebuggerAppendMsg("Start Task without DLIST: ucode=%08X, data=%08X", (u32)pTask->t.ucode, (u32)pTask->t.ucode_data);});


	// Check if we need to purge
	if (status.gRDPTime - status.lastPurgeTimeTime > 5000)
	{
		gTextureCache.PurgeOldTextures();
		status.lastPurgeTimeTime = status.gRDPTime;
	}

	status.dwNumDListsCulled = 0;
	status.dwNumTrisRendered = 0;
	status.dwNumTrisClipped = 0;
	status.dwNumVertices = 0;

	DL_PF("DP: Firing up RDP!");

	// Lock the graphics context here.
	g_RendererLock.Lock();
	if( currentRomOptions.forceBufferClear && CGraphicsContext::needCleanScene )
	{
		CDaedalusRender::g_pRender->ClearBuffer(true,true);
		CGraphicsContext::needCleanScene = false;
	}
	//CDaedalusRender::g_pRender->ClearBuffer(true, true);
	if ( CDaedalusRender::g_pRender != NULL)
	{
		SetVIScales();
		CDaedalusRender::g_pRender->RenderReset();

		//for( int i=0; i<8; i++ )
		//{
		//	memset(&gRDP.tiles[i], 0, sizeof(Tile));
		//}

		CDaedalusRender::g_pRender->BeginRendering();
		CDaedalusRender::g_pRender->SetViewport(0, 0, windowSetting.uViWidth, windowSetting.uViHeight, 0x3FF);


		// Process the entire display list in one go
		while (!status.bRDPHalted)
		{
#ifdef _DEBUG
			if( options.bWinFrameMode )
			{
				CDaedalusRender::g_pRender->SetFillMode(DAEDALUS_FILLMODE_WINFRAME );
			}
			else
			{
				CDaedalusRender::g_pRender->SetFillMode(DAEDALUS_FILLMODE_SOLID);
			}

			if( debuggerEnableZBuffer )
			{
				CDaedalusRender::g_pRender->ZBufferEnable( TRUE );
			}
			else
			{
				CDaedalusRender::g_pRender->ZBufferEnable( FALSE );
			}

			DEBUGGER_PAUSE_COUNT_N(NEXT_UCODE);
			if( debuggerPause )
			{
				DebuggerPause();
			}
#endif
			// Current PC is the last value on the stack
			dwPC = g_dwPCStack[g_dwPCindex].addr;
			dwCmd0 = g_pu32RamBase[(dwPC>>2)+0];
			dwCmd1 = g_pu32RamBase[(dwPC>>2)+1];

#ifdef _DEBUG
			if (dwPC > g_dwRamSize)
			{
				DebuggerAppendMsg("Error: dwPC is %08X", dwPC );
				break;
			}
#endif

			g_dwPCStack[g_dwPCindex].addr += 8;

			StartProfiler(PROFILE_UCODE_PARSER);
			DL_PF("0x%08x: %08x %08x %-10s", 
				dwPC, dwCmd0, dwCmd1, (gRSP.ucode!=5&&gRSP.ucode!=10)?g_szRDPInstrName[(dwCmd0>>24)]:g_szRDPInstrName_GBI2[(dwCmd0>>24)]);

			currentUcodeMap[dwCmd0>>24](dwCmd0, dwCmd1);
			StopProfiler(PROFILE_UCODE_PARSER);

			// Check limit
			if (!status.bRDPHalted)
			{
				g_dwPCStack[g_dwPCindex].limit--;
				if (g_dwPCStack[g_dwPCindex].limit == ~0)
				{
					DL_PF("**EndDLInMem");
					// If we're here, then we musn't have finished with the display list yet
					// Check if this is the lasy display list in the sequence
					if(g_dwPCindex == 0)
					{
						StopRDP();
					}
					else
					{
						g_dwPCindex--;
					}
				}	

			}
		}

		CDaedalusRender::g_pRender->EndRendering();
	}

	g_RendererLock.Unlock();
	TriggerDPInterrupt();
	StopProfiler(PROFILE_ALL);
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//                   Util Functions                     //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void RDP_NOIMPL_Real(LPCTSTR op, DWORD dwCmd0, DWORD dwCmd1) 
{
#ifdef _DEBUG
	if( logUnknownOpcode )
	{
		TRACE0("Stack Trace");
		for( int i=0; i<g_dwPCindex; i++ )
		{
			DebuggerAppendMsg("  %08X", g_dwPCStack[i].addr);
		}
		DWORD dwPC = g_dwPCStack[g_dwPCindex].addr-8;
		DebuggerAppendMsg("PC=%08X",dwPC);
		DebuggerAppendMsg(op, dwCmd0, dwCmd1);
	}
	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_UNKNOWN_OP, {TRACE0("Paused at unimplemented ucode\n");})
#endif
}

void RDP_NOIMPL_WARN(LPCTSTR op)
{
#ifdef _DEBUG
	if(logUnknownOpcode)
	{
		TRACE0(op);
	}
#endif
}


void DLParser_GBI1_Noop(DWORD dwCmd0, DWORD dwCmd1)
{
}


void RDP_GFX_PopDL()
{
	DL_PF("Returning from DisplayList: level=%d", g_dwPCindex+1);
	DL_PF("############################################");
	DL_PF("/\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\");
	DL_PF("");

	if (g_dwPCindex < 0)
	{
		DL_PF("EndDisplayList: Too many EndDL calls - ignoring");
		StopRDP();			// Stop, damn it!
		return;
	}

	// Check if this is the last display list in the sequence
	if(g_dwPCindex == 0)
	{
		DL_PF("EndDisplayList: The last one, halt RSP");
		StopRDP();
		g_dwPCindex=-1;
		return;
	}

	// If we're here, then we musn't have finished with the display list yet
	g_dwPCindex--;
}

DWORD CalcalateCRC(DWORD* srcPtr, DWORD srcSize)
{
	DWORD crc=0;
	for( DWORD i=0; i<srcSize; i++ )
	{
		crc += srcPtr[i];
	}
	return crc;
}


void RDP_GFX_InitGeometryMode()
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
	CDaedalusRender::g_pRender->SetCullMode(bCullFront, bCullBack);
	
	if (bShade && bShadeSmooth)		CDaedalusRender::g_pRender->SetShadeMode( SHADE_SMOOTH );
	else							CDaedalusRender::g_pRender->SetShadeMode( SHADE_FLAT );
	
	CDaedalusRender::g_pRender->SetFogEnable( bFog );
	SetTextureGen(bTextureGen);

	SetLighting( bLighting );
	CDaedalusRender::g_pRender->ZBufferEnable( bZBuffer );
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//                   DP Ucodes                          //
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

void DLParser_SetKeyGB(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(DLParser_SetKeyGB);
	static BOOL bWarned = FALSE;

	DL_PF("    SetKeyGB (not implemented)");
	gRDP.keyB = (dwCmd1>>8)&0xFF;
	gRDP.keyG = (dwCmd1>>24)&0xFF;
	gRDP.keyA = (gRDP.keyR+gRDP.keyG+gRDP.keyB)/3;
	gRDP.fKeyA = gRDP.keyA/255.0f;
	if (!bWarned)
	{
		RDP_NOIMPL("RDP: SetKeyGB 0x%08x 0x%08x", dwCmd0, dwCmd1);
		bWarned = TRUE;
	}

}
void DLParser_SetKeyR(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(DLParser_SetKeyR);
	static BOOL bWarned = FALSE;

	DL_PF("    SetKeyR (not implemented)");
	gRDP.keyR = (dwCmd1>>8)&0xFF;
	gRDP.keyA = (gRDP.keyR+gRDP.keyG+gRDP.keyB)/3;
	gRDP.fKeyA = gRDP.keyA/255.0f;
	if (!bWarned)
	{
		RDP_NOIMPL("RDP: SetKeyR 0x%08x 0x%08x", dwCmd0, dwCmd1);
		bWarned = TRUE;
	}
}

int g_convk0,g_convk1,g_convk2,g_convk3,g_convk4,g_convk5;
float g_convc0,g_convc1,g_convc2,g_convc3,g_convc4,g_convc5;
void DLParser_SetConvert(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(DLParser_SetConvert);
	/*
#define gsDPSetConvert(k0, k1, k2, k3, k4, k5)				\
{									\
	(_SHIFTL(G_SETCONVERT, 24, 8) |					\
	 _SHIFTL(k0, 13, 9) | _SHIFTL(k1, 4, 9) | _SHIFTL(k2, 5, 4)),	\
	(_SHIFTL(k2, 27, 5) | _SHIFTL(k3, 18, 9) | _SHIFTL(k4, 9, 9) | 	\
	 _SHIFTL(k5, 0, 9))						\
}
*/
	int temp;

	temp = (dwCmd0>>13)&0x1FF;
	g_convk0 = temp>0xFF ? -(temp-0x100) : temp;

	temp = (dwCmd0>>4)&0x1FF;
	g_convk1 = temp>0xFF ? -(temp-0x100) : temp;

	temp = dwCmd0&0xF;
	temp = (temp<<5)|((dwCmd1>>27)&0x1F);
	g_convk2 = temp>0xFF ? -(temp-0x100) : temp;

	temp = (dwCmd1>>18)&0x1FF;
	g_convk3 = temp>0xFF ? -(temp-0x100) : temp;

	temp = (dwCmd1>>9)&0x1FF;
	g_convk4 = temp>0xFF ? -(temp-0x100) : temp;

	temp = dwCmd1&0x1FF;
	g_convk5 = temp>0xFF ? -(temp-0x100) : temp;

	g_convc0 = g_convk5/255.0f+1.0f;
	g_convc1 = g_convk0/255.0f*g_convc0;
	g_convc2 = g_convk1/255.0f*g_convc0;
	g_convc3 = g_convk2/255.0f*g_convc0;
	g_convc4 = g_convk3/255.0f*g_convc0;
}
void DLParser_SetPrimDepth(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(DLParser_SetPrimDepth);
	DWORD dwZ  = (dwCmd1 >> 16) & 0xFFFF;
	DWORD dwDZ = (dwCmd1      ) & 0xFFFF;

	DL_PF("SetPrimDepth: 0x%08x 0x%08x - z: 0x%04x dz: 0x%04x",
		dwCmd0, dwCmd1, dwZ, dwDZ);
	
	SetPrimitiveDepth(dwZ, dwDZ);
	DEBUGGER_PAUSE(NEXT_SET_PRIM_COLOR);
}

void DLParser_RDPSetOtherMode(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(DLParser_RDPSetOtherMode);
	gRDP.otherMode._u32[1] = dwCmd0;	// High
	gRDP.otherMode._u32[0] = dwCmd1;	// Low

	if( gRDP.otherModeH != (dwCmd0 & 0x0FFFFFFF) )
	{
		gRDP.otherModeH = (dwCmd0 & 0x0FFFFFFF);

		DWORD dwTextFilt  = (gRDP.otherModeH>>G_MDSFT_TEXTFILT)&0x3;
		CDaedalusRender::g_pRender->SetTextureFilter(dwTextFilt<<G_MDSFT_TEXTFILT);
	}

	if( gRDP.otherModeL != dwCmd1 )
	{
		if( (gRDP.otherModeL&ZMODE_DEC) != (dwCmd1&ZMODE_DEC) )
		{
			if( (dwCmd1&ZMODE_DEC) )
				CDaedalusRender::g_pRender->SetZBias( 2 );
			else
				CDaedalusRender::g_pRender->SetZBias( 0 );
		}

		gRDP.otherModeL = dwCmd1;

		BOOL bZCompare		= (gRDP.otherModeL & Z_CMP)			? TRUE : FALSE;
		BOOL bZUpdate		= (gRDP.otherModeL & Z_UPD)			? TRUE : FALSE;

		CDaedalusRender::g_pRender->SetZCompare( bZCompare );
		CDaedalusRender::g_pRender->SetZUpdate( bZUpdate );

		DWORD dwAlphaTestMode = (gRDP.otherModeL >> G_MDSFT_ALPHACOMPARE) & 0x3;

		if ((dwAlphaTestMode) != 0)
			CDaedalusRender::g_pRender->SetAlphaTestEnable( TRUE );
		else
			CDaedalusRender::g_pRender->SetAlphaTestEnable( FALSE );
	}

	u16 blender = gRDP.otherMode.blender;
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



void DLParser_RDPLoadSync(DWORD dwCmd0, DWORD dwCmd1)	
{ 
	DP_Timing(DLParser_RDPLoadSync);
	/*DL_PF("LoadSync: (Ignored)");*/ 
}

void DLParser_RDPPipeSync(DWORD dwCmd0, DWORD dwCmd1)	
{ 
	DP_Timing(DLParser_RDPPipeSync);
	/*DL_PF("PipeSync: (Ignored)");*/ 
}
void DLParser_RDPTileSync(DWORD dwCmd0, DWORD dwCmd1)	
{ 
	DP_Timing(DLParser_RDPTileSync);
	/*DL_PF("TileSync: (Ignored)");*/ 
}

void DLParser_RDPFullSync(DWORD dwCmd0, DWORD dwCmd1)
{ 
	DP_Timing(DLParser_RDPFullSync);
	//RECT rect= {0,0,100,100};
	//CDaedalusRender::g_pRender->DrawText("Hello", &rect);
	TriggerDPInterrupt();
}

void DLParser_SetScissor(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(DLParser_SetScissor);

	ScissorType tempScissor;
	// The coords are all in 8:2 fixed point
	tempScissor.x0   = (dwCmd0>>12)&0xFFF;
	tempScissor.y0   = (dwCmd0>>0 )&0xFFF;
	tempScissor.mode = (dwCmd1>>24)&0x03;
	tempScissor.x1   = (dwCmd1>>12)&0xFFF;
	tempScissor.y1   = (dwCmd1>>0 )&0xFFF;

	tempScissor.left	= tempScissor.x0/4;
	tempScissor.top		= tempScissor.y0/4;
	tempScissor.right	= tempScissor.x1/4;
	tempScissor.bottom	= tempScissor.y1/4;

	if( options.enableHacks )
	{
		if( g_CI.dwWidth == 0x200 && tempScissor.right == 0x200 )
		{
			//DWORD val = *g_GraphicsInfo.VI_X_SCALE_REG & 0xFFF;
			//float xscale = (float)val / (1<<10);
			//DWORD start = *g_GraphicsInfo.VI_H_START_REG >> 16;
			//DWORD end = *g_GraphicsInfo.VI_H_START_REG&0xFFFF;
			DWORD width = *g_GraphicsInfo.VI_WIDTH_REG & 0xFFF;
			//float fViWidth = (end-start)*xscale;

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
		SetVIScales();
		CDaedalusRender::g_pRender->UpdateClipRectangle();
		CDaedalusRender::g_pRender->UpdateScissor();
		CDaedalusRender::g_pRender->SetViewportRender();
	}

	DL_PF("SetScissor: x0=%d y0=%d x1=%d y1=%d mode=%d",
		gRDP.scissor.left, gRDP.scissor.top,
		gRDP.scissor.right, gRDP.scissor.bottom,
		gRDP.scissor.mode);
}


void DLParser_FillRect(DWORD dwCmd0, DWORD dwCmd1)
{ 
	DP_Timing(DLParser_FillRect);	// fix me
	status.primitiveType = PRIM_FILLRECT;

	DWORD x0   = ((dwCmd1>>12)&0xFFF)/4;
	DWORD y0   = ((dwCmd1>>0 )&0xFFF)/4;
	DWORD x1   = ((dwCmd0>>12)&0xFFF)/4;
	DWORD y1   = ((dwCmd0>>0 )&0xFFF)/4;

	// Note, in some modes, the right/bottom lines aren't drawn

	DL_PF("    (%d,%d) (%d,%d)", x0, y0, x1, y1);

	// TODO - In 1/2cycle mode, skip bottom/right edges!?
	if( gRDP.otherMode.cycle_type >= CYCTYPE_COPY )
	{
		x1++;
		y1++;
	}


	if (g_DI.dwAddr == g_CI.dwAddr)
	{
		/*
		if( g_bUsingFakeCI ) 
		{
			SaveFakeFrameBuffer();
			//CloseFakeFrameBufferWithoutSave();
		}
		*/
		
		// Clear the Z Buffer
		CDaedalusRender::g_pRender->ClearBuffer(false,true);	//Check me

		DL_PF("    Clearing ZBuffer");

	}
	else if( g_bUsingFakeCI && currentRomOptions.N64FrameBufferEmuType != FRM_DISABLE && currentRomOptions.N64FrameBufferEmuType != FRM_COMPLETE )
	{
		if( (currentRomOptions.N64FrameBufferEmuType == FRM_FASTER||currentRomOptions.N64FrameBufferEmuType ==FRM_FASTER_RECHECK) )
		{
			if( g_FakeFrameBufferInfo.CI_Info.dwSize == G_IM_SIZ_16b )
			{
				WORD color = (WORD)gRDP.originalFillColor;
				u32 pitch = g_FakeFrameBufferInfo.width<<1;
				DWORD base = (DWORD)(g_pu8RamBase + g_FakeFrameBufferInfo.CI_Info.dwAddr);
				for( DWORD i =y0; i<y1; i++ )
				{
					for( DWORD j=x0; j<x1; j++ )
					{
						*(WORD*)((base+pitch*i+j)^2) = color;
					}
				}
			}
			else
			{
				//TRACE0("Fake frame buffer: FillRect, CI/8b, has not been implemented");
				BYTE color = (BYTE)gRDP.originalFillColor;
				u32 pitch = g_FakeFrameBufferInfo.width;
				DWORD base = (DWORD)(g_pu8RamBase + g_FakeFrameBufferInfo.CI_Info.dwAddr);
				for( DWORD i=y0; i<y1; i++ )
				{
					for( DWORD j=x0; j<x1; j++ )
					{
						*(BYTE*)((base+pitch*i+j)^3) = color;
					}
				}
			}
		}
		else	// == FRM_IGNORE
		{
		}
		DEBUGGER_PAUSE_AND_DUMP_COUNT_N( NEXT_FLUSH_TRI,{TRACE0("Pause after FillRect\n");});
		DEBUGGER_PAUSE_AND_DUMP_COUNT_N( NEXT_FILLRECT, {DebuggerAppendMsg("FillRect: X0=%d, Y0=%d, X1=%d, Y1=%d, Color=0x%08X", x0, y0, x1, y1, gRDP.originalFillColor);
				DebuggerAppendMsg("Pause after FillRect: Color=%08X\n", gRDP.originalFillColor);});
	}
	else
	{
		DL_PF("    Filling Rectangle");
		if( gRDP.otherMode.cycle_type == CYCTYPE_FILL )
		{
			if( !g_bUsingFakeCI || g_FakeFrameBufferInfo.CI_Info.dwSize == G_IM_SIZ_16b )
			{
				CDaedalusRender::g_pRender->FillRect(x0, y0, x1, y1, gRDP.fillColor);
			}
		}
		else
		{
			DaedalusColor primColor = GetPrimitiveColor();
			//if( RGBA_GETALPHA(primColor) != 0 )
			{
				CDaedalusRender::g_pRender->FillRect(x0, y0, x1, y1, primColor);
			}
		}
	}
}


void DLParser_SetZImg(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(DLParser_SetZImg);
	DL_PF("    Image: 0x%08x", RDPSegAddr(dwCmd1));

	DWORD dwFmt   = (dwCmd0>>21)&0x7;
	DWORD dwSiz   = (dwCmd0>>19)&0x3;
	DWORD dwWidth = (dwCmd0&0x0FFF) + 1;

	g_DI.dwAddr = RDPSegAddr(dwCmd1);
	g_DI.dwFormat = dwFmt;
	g_DI.dwSize = dwSiz;
	g_DI.dwWidth = dwWidth;

	DEBUGGER_PAUSE_AND_DUMP_NO_UPDATE(NEXT_SET_CIMG, 
		{
			DebuggerAppendMsg("Pause after SetZImg: Addr=0x%08X, Fmt:%s-%sb, Width=%d\n", 
				g_DI.dwAddr, pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);
		}
	);
	DEBUGGER_IF_DUMP((pauseAtNext && (eventToPause==NEXT_FAKE_FRAME_BUFFER)), 
		{
			DebuggerAppendMsg("SetZImg: Addr=0x%08X, Fmt:%s-%sb, Width=%d\n", 
				g_DI.dwAddr, pszImgFormat[dwFmt], pszImgSize[dwSiz], dwWidth);
		}
	);
}

// NOT CHECKED
/*
#define G_ZBUFFER				0x00000001
#define G_TEXTURE_ENABLE		0x00000002	// Microcode use only 
#define G_SHADE					0x00000004	// enable Gouraud interp
#define G_SHADING_SMOOTH		0x00000200	// flat or smooth shaded
#define G_CULL_FRONT			0x00001000
#define G_CULL_BACK				0x00002000
#define G_CULL_BOTH				0x00003000	// To make code cleaner
#define G_FOG					0x00010000
#define G_LIGHTING				0x00020000
#define G_TEXTURE_GEN			0x00040000
#define G_TEXTURE_GEN_LINEAR	0x00080000*/


void DLParser_SetCombine(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(DLParser_SetCombine);
	DWORD dwMux0 = dwCmd0&0x00FFFFFF;
	DWORD dwMux1 = dwCmd1;
	CDaedalusRender::g_pRender->SetMux(dwMux0, dwMux1);
}

void DLParser_SetFillColor(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(DLParser_SetFillColor);
	// TODO - Check colour image format to work out how this ahould be decoded!
	gRDP.fillColor = Convert555ToRGBA((WORD)dwCmd1);
	gRDP.originalFillColor = dwCmd1;

	DL_PF("    Color5551=0x%04x = 0x%08x", (WORD)dwCmd1, gRDP.fillColor);

}

void DLParser_SetFogColor(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(DLParser_SetFogColor);
	DWORD dwRed		= (dwCmd1>>24)&0xFF;
	DWORD dwGreen	= (dwCmd1>>16)&0xFF;
	DWORD dwBlue	= (dwCmd1>>8)&0xFF;
	DWORD dwAlpha	= (dwCmd1)&0xFF;

	DL_PF("    RGBA: %d %d %d %d", dwRed, dwGreen, dwBlue, dwAlpha);

	DWORD dwFogColor = dwCmd1;

	CDaedalusRender::g_pRender->SetFogColor( dwRed, dwGreen, dwBlue, dwAlpha );	
}

void DLParser_SetBlendColor(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(DLParser_SetBlendColor);
	DWORD dwRed		= (dwCmd1>>24)&0xFF;
	DWORD dwGreen	= (dwCmd1>>16)&0xFF;
	DWORD dwBlue	= (dwCmd1>>8)&0xFF;
	DWORD dwAlpha	= (dwCmd1)&0xFF;

	DL_PF("    RGBA: %d %d %d %d", dwRed, dwGreen, dwBlue, dwAlpha);

	CDaedalusRender::g_pRender->SetAlphaRef(dwAlpha);
}


void DLParser_SetPrimColor(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(DLParser_SetPrimColor);
	DWORD dwM		= (dwCmd0>>8)&0xFF;
	DWORD dwL		= (dwCmd0)&0xFF;
	DWORD dwRed		= (dwCmd1>>24)&0xFF;
	DWORD dwGreen	= (dwCmd1>>16)&0xFF;
	DWORD dwBlue	= (dwCmd1>>8)&0xFF;
	DWORD dwAlpha	= (dwCmd1)&0xFF;

	DL_PF("    M:%d L:%d RGBA: %d %d %d %d", dwM, dwL, dwRed, dwGreen, dwBlue, dwAlpha);

	SetPrimitiveColor( DAEDALUS_COLOR_RGBA(dwRed, dwGreen, dwBlue, dwAlpha), dwM, dwL);
	DEBUGGER_PAUSE_AND_DUMP_COUNT_N(NEXT_SET_PRIM_COLOR, DebuggerAppendMsg("SetPrimColor: M:%d L:%d RGBA: %d %d %d %d", dwM, dwL, dwRed, dwGreen, dwBlue, dwAlpha));
}

void DLParser_SetEnvColor(DWORD dwCmd0, DWORD dwCmd1)
{
	DP_Timing(DLParser_SetEnvColor);
	DWORD dwRed		= (dwCmd1>>24)&0xFF;
	DWORD dwGreen	= (dwCmd1>>16)&0xFF;
	DWORD dwBlue	= (dwCmd1>>8)&0xFF;
	DWORD dwAlpha	= (dwCmd1)&0xFF;
	DL_PF("    RGBA: %d %d %d %d",
		dwRed, dwGreen, dwBlue, dwAlpha);

	SetEnvColor( DAEDALUS_COLOR_RGBA(dwRed, dwGreen, dwBlue, dwAlpha));
}

