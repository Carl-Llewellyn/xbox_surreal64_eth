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

#ifndef _RDP_S2DEX_H_
#define _RDP_S2DEX_H_

#define	G_BG_1CYC			0x01
#define	G_BG_COPY			0x02
#define	G_OBJ_RECTANGLE		0x03
#define	G_OBJ_SPRITE		0x04
#define	G_OBJ_MOVEMEM		0x05
#define	G_SELECT_DL			0xb0
#define	G_OBJ_RENDERMODE	0xb1
#define	G_OBJ_RECTANGLE_R	0xb2
#define	G_OBJ_LOADTXTR		0xc1
#define	G_OBJ_LDTX_SPRITE	0xc2
#define	G_OBJ_LDTX_RECT		0xc3
#define	G_OBJ_LDTX_RECT_R	0xc4
#define	G_RDPHALF_0			0xe4

#define	G_OBJLT_TXTRBLOCK	0x00001033
#define	G_OBJLT_TXTRTILE	0x00fc1034
#define	G_OBJLT_TLUT		0x00000030
#define	G_BGLT_LOADBLOCK	0x0033
#define	G_BGLT_LOADTILE		0xfff4

void RDP_S2DEX_BG_1CYC(DWORD dwCmd0, DWORD dwCmd1);
void RDP_S2DEX_BG_COPY(DWORD dwCmd0, DWORD dwCmd1);
void RDP_S2DEX_OBJ_RECTANGLE(DWORD dwCmd0, DWORD dwCmd1);
void RDP_S2DEX_OBJ_SPRITE(DWORD dwCmd0, DWORD dwCmd1);
void RDP_S2DEX_OBJ_MOVEMEM(DWORD dwCmd0, DWORD dwCmd1);
void RDP_S2DEX_SELECT_DL(DWORD dwCmd0, DWORD dwCmd1);
void RDP_S2DEX_OBJ_RENDERMODE(DWORD dwCmd0, DWORD dwCmd1);
void RDP_S2DEX_OBJ_RECTANGLE_R(DWORD dwCmd0, DWORD dwCmd1);
void RDP_S2DEX_SPObjLoadTxtr(DWORD dwCmd0, DWORD dwCmd1);
void RDP_S2DEX_SPObjLoadTxSprite(DWORD dwCmd0, DWORD dwCmd1);
void RDP_S2DEX_SPObjLoadTxRect(DWORD dwCmd0, DWORD dwCmd1);
void RDP_S2DEX_SPObjLoadTxRectR(DWORD dwCmd0, DWORD dwCmd1);
void RDP_S2DEX_RDPHALF_0(DWORD dwCmd0, DWORD dwCmd1);
void RDP_S2DEX_Yoshi_Unknown(DWORD dwCmd0, DWORD dwCmd1);




/*
typedef	struct	{	//Original format of N64
  u32	type;		// G_OBJLT_TXTRBLOCK divided into types.                                
  u64	*image;		// The texture source address on DRAM.                                  
  u16	tmem;		// The  transferred TMEM word address.   (8byteWORD)                    
  u16	tsize;		// The Texture size.  Specified by the macro  GS_TB_TSIZE().            
  u16	tline;		// The width of the Texture 1-line. Specified by the macro GS_TB_TLINE()
  u16	sid;		// STATE ID Multipled by 4.  Either one of  0,4,8 and 12.               
  u32	flag;		// STATE flag
  u32	mask;		// STATE mask
} uObjTxtrBlock_t;		// 24 bytes
*/

typedef	struct	{	//Intel format
  u32	type;		// G_OBJLT_TXTRBLOCK divided into types.
  u32	image;
  
  u16	tsize;		// The Texture size.  Specified by the macro  GS_TB_TSIZE().            
  u16	tmem;		// The  transferred TMEM word address.   (8byteWORD)  
  
  u16	sid;		// STATE ID Multipled by 4.  Either one of  0,4,8 and 12.               
  u16	tline;		// The width of the Texture 1-line. Specified by the macro GS_TB_TLINE()

  u32	flag;		// STATE flag
  u32	mask;		// STATE mask
} uObjTxtrBlock;		// 24 bytes


/*
typedef	struct	{	//Original format of N64
  u32	type;		// G_OBJLT_TXTRTILE divided into types.                             
  u64	*image;		// The texture source address on DRAM.                              
  u16	tmem;		// The loaded texture source address on DRAM.  (8byteWORD)          
  u16	twidth;		// The width of the Texture. Specified by the macro GS_TT_TWIDTH()  
  u16	theight;	// The height of the Texture. Specified by the macro GS_TT_THEIGHT()
  u16	sid;		// STATE ID  Multiplied by 4.  Either one of 0,4,8 and 12.          
  u32	flag;		// STATE flag  
  u32	mask;		// STATE mask  
} uObjTxtrTile_t;	// 24 bytes
*/

typedef	struct	{	//Intel Format
  u32	type;		// G_OBJLT_TXTRTILE divided into types.                             
  u32	image;

  u16	twidth;		// The width of the Texture. Specified by the macro GS_TT_TWIDTH()  
  u16	tmem;		// The loaded texture source address on DRAM.  (8byteWORD)          

  u16	sid;		// STATE ID  Multiplied by 4.  Either one of 0,4,8 and 12.          
  u16	theight;	// The height of the Texture. Specified by the macro GS_TT_THEIGHT()

  u32	flag;		// STATE flag  
  u32	mask;		// STATE mask  
} uObjTxtrTile;	// 24 bytes

/*
typedef	struct	{	// N64 format
  u32	type;		// G_OBJLT_TLUT divided into types.                            
  u64	*image;		// the texture source address on DRAM.                         
  u16	phead;		// The pallet number of the load header.  Between 256 and 511. 
  u16	pnum;		// The loading pallet number -1.                               
  u16   zero;		// Assign 0 all the time.                                      
  u16	sid;		// STATE ID  Multiplied by 4.  Either one of 0,4,8 and 12.     
  u32	flag;		// STATE flag  
  u32	mask;		// STATE mask  
} uObjTxtrTLUT_t;	// 24 bytes 
*/
typedef	struct	{	// Intel Format
  u32	type;		// G_OBJLT_TLUT divided into types.                            
  u32	image;
  
  u16	pnum;		// The loading pallet number -1.   
  u16	phead;		// The pallet number of the load header.  Between 256 and 511. 
  
  u16	sid;		// STATE ID  Multiplied by 4.  Either one of 0,4,8 and 12.    
  u16   zero;		// Assign 0 all the time.                                      
  
  u32	flag;		// STATE flag  
  u32	mask;		// STATE mask  
} uObjTxtrTLUT;	// 24 bytes 

typedef union {
  uObjTxtrBlock      block;
  uObjTxtrTile       tile;
  uObjTxtrTLUT       tlut;
} uObjTxtr;


/*
typedef struct {	// N64 format
  s16  objX;		// The x-coordinate of the upper-left end. s10.2 OBJ                
  u16  scaleW;		// Scaling of the u5.10 width direction.                            
  u16  imageW;		// The width of the u10.5 texture. (The length of the S-direction.) 
  u16  paddingX;	// Unused.  Always 0.                                               
  s16  objY;		// The y-coordinate of the s10.2 OBJ upper-left end.                
  u16  scaleH;		// Scaling of the u5.10 height direction.                           
  u16  imageH;		// The height of the u10.5 texture. (The length of the T-direction.)
  u16  paddingY;	// Unused.  Always 0.                                               
  u16  imageStride;	// The folding width of the texel.        (In units of 64bit word.) 
  u16  imageAdrs;	// The texture header position in  TMEM.  (In units of 64bit word.)
  u8   imageFmt;	// The format of the texel.   G_IM_FMT_*       
  u8   imageSiz;	// The size of the texel.         G_IM_SIZ_*       
  u8   imagePal;	//The pallet number.  0-7                        
  u8   imageFlags;	// The display flag.    G_OBJ_FLAG_FLIP*            
} uObjSprite_t;		// 24 bytes 
*/

typedef struct {	// Intel format
  u16  scaleW;		// Scaling of the u5.10 width direction.     
  s16  objX;		// The x-coordinate of the upper-left end. s10.2 OBJ                
  
  u16  paddingX;	// Unused.  Always 0.        
  u16  imageW;		// The width of the u10.5 texture. (The length of the S-direction.) 
  
  u16  scaleH;		// Scaling of the u5.10 height direction. 
  s16  objY;		// The y-coordinate of the s10.2 OBJ upper-left end.                
  
  u16  paddingY;	// Unused.  Always 0.              
  u16  imageH;		// The height of the u10.5 texture. (The length of the T-direction.)
  
  u16  imageAdrs;	// The texture header position in  TMEM.  (In units of 64bit word.)
  u16  imageStride;	// The folding width of the texel.        (In units of 64bit word.) 

  u8   imageFlags;	// The display flag.    G_OBJ_FLAG_FLIP*  
  u8   imagePal;	//The pallet number.  0-7                        
  u8   imageSiz;	// The size of the texel.         G_IM_SIZ_*       
  u8   imageFmt;	// The format of the texel.   G_IM_FMT_*       
} uObjSprite;		// 24 bytes 


typedef	struct	{
  uObjTxtr	txtr;
  uObjSprite	sprite;
} uObjTxSprite;		/* 48 bytes */


/*
typedef struct {	// N64 format
  s32   A, B, C, D;     // s15.16 
  s16   X, Y;           // s10.2 
  u16   BaseScaleX;	// u5.10 
  u16   BaseScaleY;	// u5.10
} uObjMtx_t;		// 24 bytes
*/

typedef struct {		// Intel format
  s32   A, B, C, D;     // s15.16 

  s16   Y;	           // s10.2 
  s16	X;	           // s10.2 

  u16   BaseScaleY;		// u5.10
  u16   BaseScaleX;		// u5.10 
} uObjMtx;				// 24 bytes

typedef struct {
  float   A, B, C, D;     // s15.16 
  float	  X;	           // s10.2 
  float   Y;	           // s10.2 
  float   BaseScaleX;		// u5.10 
  float   BaseScaleY;		// u5.10
} uObjMtxReal;


/*
typedef struct {	// N64 format
  s16   X, Y;           // s10.2  
  u16   BaseScaleX;		// u5.10  
  u16   BaseScaleY;		// u5.10  
} uObjSubMtx_t;			// 8 bytes 
*/

typedef struct {	//Intel format
  s16   Y;				// s10.2  
  s16   X;				// s10.2  
  u16   BaseScaleY;		// u5.10  
  u16   BaseScaleX;		// u5.10  
} uObjSubMtx;			// 8 bytes 

/*
typedef	struct	{	N64 Format
  u16   imageX;		// The x-coordinate of the upper-left position of the texture.  (u11.5)
  u16	imageW;		// The width of the texture.                                    (u10.2)
  s16	frameX;		// The upper-left position of the transferred frame.            (s10.2)
  u16	frameW;		// The width of the transferred frame.                          (u10.2)

  u16   imageY; 	// The y-coordinate of the upper-left position of the texture.  (u11.5)
  u16	imageH;		// The height of the texture.                                   (u10.2)
  s16	frameY;		// The upper-left position of the transferred frame.            (s10.2)
  u16	frameH;		// The height of the transferred frame.                         (u10.2)

  u64  *imagePtr;	// The texture source address on DRAM.           
  u16	imageLoad;	// Which to use,  LoadBlock or  LoadTile?        
  u8	imageFmt;	// The format of the texel.  G_IM_FMT_*          
  u8	imageSiz;	// The size of the texel        G_IM_SIZ_*       
  u16   imagePal; 	// The pallet number.                            
  u16	imageFlip;	// The right & left inversion of the image. Inverted by G_BG_FLAG_FLIPS

  // Because the following are set in the initialization routine guS2DInitBg(), the user doesn't 
  // have to set it.
  u16	tmemW;		// The TMEM width and Work size of the frame 1 line. 
                           At LoadBlock, GS_PIX2TMEM(imageW/4,imageSiz)
                           At LoadTile  GS_PIX2TMEM(frameW/4,imageSiz)+1  
  u16	tmemH;		// The height of TMEM loadable at a time.  (s13.2) The 4 times value.
			   When the normal texture,   512/tmemW*4
			   When the CI texture,    256/tmemW*4       
  u16	tmemLoadSH;	// The SH value
			   At LoadBlock,  tmemSize/2-1
			   At LoadTile,  tmemW*16-1                  
  u16	tmemLoadTH;	// The TH value or the Stride value 
			   At LoadBlock,  GS_CALC_DXT(tmemW)
			   At LoadTile,  tmemH-1                     
  u16	tmemSizeW;	// The skip value of imagePtr for image 1-line.  
			   At LoadBlock, tmemW*2
                           At LoadTile,  GS_PIX2TMEM(imageW/4,imageSiz)*2 
  u16	tmemSize;	// The skip value of  imagePtr for 1-loading.  
			   = tmemSizeW*tmemH                         
} uObjBg_t;		// 40 bytes
*/

typedef	struct	{	// Intel Format
  u16	imageW;		// The width of the texture.                                    (u10.2)
  u16   imageX;		// The x-coordinate of the upper-left position of the texture.  (u11.5)

  u16	frameW;		// The width of the transferred frame.                          (u10.2)
  s16	frameX;		// The upper-left position of the transferred frame.            (s10.2)

  u16	imageH;		// The height of the texture.                                   (u10.2)
  u16   imageY; 	// The y-coordinate of the upper-left position of the texture.  (u11.5)

  u16	frameH;		// The height of the transferred frame.                         (u10.2)
  s16	frameY;		// The upper-left position of the transferred frame.            (s10.2)

  u32	imagePtr;	// The texture source address on DRAM.           

  u8	imageSiz;	// The size of the texel        G_IM_SIZ_*       
  u8	imageFmt;	// The format of the texel.  G_IM_FMT_*          
  u16	imageLoad;	// Which to use,  LoadBlock or  LoadTile?        

  u16	imageFlip;	// The right & left inversion of the image. Inverted by G_BG_FLAG_FLIPS
  u16   imagePal; 	// The pallet number.                            

  // Because the following are set in the initialization routine guS2DInitBg(), the user doesn't 
  // have to set it.
  u16	tmemH;		// The height of TMEM loadable at a time.  (s13.2) The 4 times value.
			   //When the normal texture,   512/tmemW*4
			   //When the CI texture,    256/tmemW*4       
  u16	tmemW;		// The TMEM width and Work size of the frame 1 line. 
                    //       At LoadBlock, GS_PIX2TMEM(imageW/4,imageSiz)
                    //       At LoadTile  GS_PIX2TMEM(frameW/4,imageSiz)+1  

  u16	tmemLoadTH;	// The TH value or the Stride value 
			   //At LoadBlock,  GS_CALC_DXT(tmemW)
			   //At LoadTile,  tmemH-1                     
  u16	tmemLoadSH;	// The SH value
			   //At LoadBlock,  tmemSize/2-1
			   //At LoadTile,  tmemW*16-1                  
  u16	tmemSize;	// The skip value of  imagePtr for 1-loading.  
			   //= tmemSizeW*tmemH                         
  u16	tmemSizeW;	// The skip value of imagePtr for image 1-line.  
			   //At LoadBlock, tmemW*2
               //            At LoadTile,  GS_PIX2TMEM(imageW/4,imageSiz)*2 
} uObjBg;		// 40 bytes

/*
// Scalable background plane 
typedef	struct	{	//N64 Format
  u16   imageX;		// The x-coordinate of the upper-left position of the texture.  (u11.5)
  u16	imageW;		// The width of the texture.                                    (u10.2)
  s16	frameX;		// The upper-left position of the transferred frame.            (s10.2)
  u16	frameW;		// The width of the transferred frame.                          (u10.2)

  u16   imageY; 	// The y-coordinate of the upper-left position of the texture.  (u11.5) 
  u16	imageH;		// The height of the texture.                                   (u10.2)
  s16	frameY;		// The upper-left position of the transferred frame.            (s10.2)
  u16	frameH;		// The height of the transferred frame.                         (u10.2)

  u64  *imagePtr;	// The texture source address on DRAM.       
  u16	imageLoad;	// Which to use,  LoadBlock or  LoadTile?    
  u8	imageFmt;	// The format of the texel.   G_IM_FMT_*     
  u8	imageSiz;	// The size of the texel      G_IM_SIZ_*     
  u16   imagePal; 	// The pallet number.                        
  u16	imageFlip;	// The right & left inversion of the image. Inverted by G_BG_FLAG_FLIPS

  u16	scaleW;		// The scale value of the X-direction.           (u5.10)
  u16	scaleH;		// The scale value of the Y-direction.           (u5.10)
  s32	imageYorig;	// The start point of drawing on the image.      (s20.5)
  
  u8	padding[4];
  
} uObjScaleBg_t;	// 40 bytes 
*/

typedef	struct	{	// Intel Format
  u16	imageW;		// The width of the texture.                                    (u10.2)
  u16   imageX;		// The x-coordinate of the upper-left position of the texture.  (u11.5)

  u16	frameW;		// The width of the transferred frame.                          (u10.2)
  s16	frameX;		// The upper-left position of the transferred frame.            (s10.2)

  u16	imageH;		// The height of the texture.                                   (u10.2)
  u16   imageY; 	// The y-coordinate of the upper-left position of the texture.  (u11.5) 

  u16	frameH;		// The height of the transferred frame.                         (u10.2)
  s16	frameY;		// The upper-left position of the transferred frame.            (s10.2)

  u32	imagePtr;	// The texture source address on DRAM.       

  u8	imageSiz;	// The size of the texel      G_IM_SIZ_*     
  u8	imageFmt;	// The format of the texel.   G_IM_FMT_*     
  u16	imageLoad;	// Which to use,  LoadBlock or  LoadTile?    

  u16	imageFlip;	// The right & left inversion of the image. Inverted by G_BG_FLAG_FLIPS
  u16   imagePal; 	// The pallet number.                        


  u16	scaleH;		// The scale value of the Y-direction.           (u5.10)
  u16	scaleW;		// The scale value of the X-direction.           (u5.10)

  s32	imageYorig;	// The start point of drawing on the image.      (s20.5)
  
  u8	padding[4];
} uObjScaleBg;		// 40 bytes 



#define DAEDALUSFVF_SPRITEVERTEX ( D3DFVF_XYZRHW | D3DFVF_TEX1 )
 typedef struct _SPRITEVERTEX {
	float x,y,z;
	float rhw;
	float  tu1, tv1;
} SPRITEVERTEX, *LPSPRITEVERTEX;


#endif
