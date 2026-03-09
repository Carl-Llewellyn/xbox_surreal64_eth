/*
Copyright (C) 2002 Rice1964

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


//Attention
// If using CUR as an argument, use it as Arg2, not Arg1. I don't know why, 
// Geforce2 seems to be picky about this

// LERP and MULTIPLYADD are actually implemented in 2 stages in video chip
// they can only be used with SEL() before it, or use 1 stage only

// SEL(SPE) only is not good for alpha channel
// In fact, Specular color does not have alpha channel

// Modifier D3DTA_COMPLEMENT does not work
// ADDSMOOTH does not work

// When using MOD with TEX and other, TEX must be the first argument, Arg1
// When MOD the DIF and FAC, using MOD(FAC,DIF) instead of MOD(DIF,FAC)

// Don't MOD(TEX,DIF) at Alpha channel, I don't know why this does not work
// probably there is not alpha blending for DIFFUSE at alpha channel

// Modifier COMPLEMENT and ALPHAREPLICATE only works as the first argument of the MOD operate
// Modifier ALPHAREPLICATE works
// Combined modifier of COMPLEMENT and ALPHAREPLICATE also works

