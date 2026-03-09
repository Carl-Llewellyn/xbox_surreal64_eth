/*
Copyright (C) 2001 Lkb

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

#ifdef LIBPNG_SUPPORT

#include "PngUtil.h"

// cexcept interface
static png_structp png_ptr = NULL;
static png_infop info_ptr = NULL;

static void png_cexcept_error(png_structp png_ptr, png_const_charp msg)
{
	// Do nothing
}



BOOL PngSaveImage (LPCTSTR pstrFileName, png_byte *pDiData,
                   int iWidth, int iHeight)
{
    const int           ciBitDepth = 8;
    const int           ciChannels = 4;

    static FILE        *pfFile;
    png_uint_32         ulRowBytes;
    static png_byte   **ppbRowPointers = NULL;
    int                 i;

    // open the PNG output file

    if (!pstrFileName)
        return FALSE;

    if (!(pfFile = fopen(pstrFileName, "wb")))
        return FALSE;

    // prepare the standard PNG structures

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL,
      (png_error_ptr)png_cexcept_error, (png_error_ptr)NULL);
    if (!png_ptr)
    {
        fclose(pfFile);
        return FALSE;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
	{
        fclose(pfFile);
        png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
        return FALSE;
    }

    //Try
    {
        // initialize the png structure
		png_init_io(png_ptr, pfFile);
    
        // we're going to write a very simple 3x8 bit RGB image
        
        png_set_IHDR(png_ptr, info_ptr, iWidth, iHeight, ciBitDepth,
            PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
            PNG_FILTER_TYPE_BASE);
        
        // write the file header information
        
        png_write_info(png_ptr, info_ptr);
        
        // swap the BGR pixels in the DiData structure to RGB
        
        png_set_bgr(png_ptr);
        
        // row_bytes is the width x number of channels
        
        ulRowBytes = iWidth * ciChannels;
        
        // we can allocate memory for an array of row-pointers
        
        if ((ppbRowPointers = (png_bytepp) malloc(iHeight * sizeof(png_bytep))) == NULL)
		{
            //Throw "Visualpng: Out of memory";
		}
        
        // set the individual row-pointers to point at the correct offsets
        
        for (i = 0; i < iHeight; i++)
            ppbRowPointers[i] = pDiData + i * (((ulRowBytes + 3) >> 2) << 2);
        
        // write out the entire image data in one call
        
        png_write_image (png_ptr, ppbRowPointers);
        
        // write the additional chunks to the PNG file (not really needed)
        
        png_write_end(png_ptr, info_ptr);
        
        // and we're done
        
        free (ppbRowPointers);
        ppbRowPointers = NULL;
        
        // clean up after the write, and free any memory allocated
        
        png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
        
        // yepp, done
    }

    fclose (pfFile);
    
    return TRUE;
}

inline void MakeShifts(DWORD mask, DWORD & l, DWORD & r)
{
	l = 0;
	r = 0;
	// While zeros in bottom bit, increase right shift
	while ((mask & 0x01) == 0)
	{
		r++;
		mask>>=1;
	}
	// While zeros in top bit, increase left shift;
	while ((mask & 0x80) == 0)
	{
		l++;
		mask<<=1;
	}
}

static void PngConvert32To32(const DDSURFACEDESC2 & ddsd, BYTE * pBytes)
{
	LONG y, x;
	u32 * pSrc;
	BYTE * pDst;

	DWORD dwMaskR = ddsd.ddpfPixelFormat.dwRBitMask;
	DWORD dwMaskG = ddsd.ddpfPixelFormat.dwGBitMask;
	DWORD dwMaskB = ddsd.ddpfPixelFormat.dwBBitMask;

	DWORD nShiftR_R, nShiftR_L;
	DWORD nShiftG_R, nShiftG_L;
	DWORD nShiftB_R, nShiftB_L;


	MakeShifts(dwMaskR, nShiftR_L, nShiftR_R);
	MakeShifts(dwMaskG, nShiftG_L, nShiftG_R);
	MakeShifts(dwMaskB, nShiftB_L, nShiftB_R);


	pDst = pBytes;
	for (y = 0; y < ddsd.dwHeight; y++)
	{
		pSrc = (u32*)((u8*)ddsd.lpSurface + y * ddsd.lPitch);
		for (x = 0; x < ddsd.dwWidth; x++)
		{
			// Write RGBA
			u32 s = pSrc[x];

			pDst[2] = ((s & dwMaskR) >> nShiftR_R) << nShiftR_L;
			pDst[1] = ((s & dwMaskG) >> nShiftG_R) << nShiftG_L;
			pDst[0] = ((s & dwMaskB) >> nShiftB_R) << nShiftB_L;
			pDst[3] = 255;

			pDst += 4;

		}
	}
}

typedef struct 
{
	u8 r,g,b;
} u24;

static void PngConvert24To32(const DDSURFACEDESC2 & ddsd, BYTE * pBytes)
{
	LONG y, x;
	u24 * pSrc;
	BYTE * pDst;

	DWORD dwMaskR = ddsd.ddpfPixelFormat.dwRBitMask;
	DWORD dwMaskG = ddsd.ddpfPixelFormat.dwGBitMask;
	DWORD dwMaskB = ddsd.ddpfPixelFormat.dwBBitMask;

	DWORD nShiftR_R, nShiftR_L;
	DWORD nShiftG_R, nShiftG_L;
	DWORD nShiftB_R, nShiftB_L;


	MakeShifts(dwMaskR, nShiftR_L, nShiftR_R);
	MakeShifts(dwMaskG, nShiftG_L, nShiftG_R);
	MakeShifts(dwMaskB, nShiftB_L, nShiftB_R);

	pDst = pBytes;
	for (y = 0; y < ddsd.dwHeight; y++)
	{
		pSrc = (u24*)((u8*)ddsd.lpSurface + y * ddsd.lPitch);
		for (x = 0; x < ddsd.dwWidth; x++)
		{
			// Not sure if this will work right - r/b might need to be swapped
			u32 s = (pSrc[x].r << 16) | (pSrc[x].g << 8) | pSrc[x].b;

			pDst[2] = ((s & dwMaskR) >> nShiftR_R) << nShiftR_L;
			pDst[1] = ((s & dwMaskG) >> nShiftG_R) << nShiftG_L;
			pDst[0] = ((s & dwMaskB) >> nShiftB_R) << nShiftB_L;
			pDst[3] = 255;

			pDst += 4;

		}
	}
}


//RGB 0x0000f800 0x000007e0 0x0000001f
static void PngConvert16To32(const DDSURFACEDESC2 & ddsd, BYTE * pBytes)
{
	LONG y, x;
	u16 * pSrc;
	BYTE * pDst;

	DWORD dwMaskR = ddsd.ddpfPixelFormat.dwRBitMask;
	DWORD dwMaskG = ddsd.ddpfPixelFormat.dwGBitMask;
	DWORD dwMaskB = ddsd.ddpfPixelFormat.dwBBitMask;

	DWORD nShiftR_R, nShiftR_L;
	DWORD nShiftG_R, nShiftG_L;
	DWORD nShiftB_R, nShiftB_L;


	MakeShifts(dwMaskR, nShiftR_L, nShiftR_R);
	MakeShifts(dwMaskG, nShiftG_L, nShiftG_R);
	MakeShifts(dwMaskB, nShiftB_L, nShiftB_R);


	pDst = pBytes;
	for (y = 0; y < ddsd.dwHeight; y++)
	{
		pSrc = (u16*)((u8*)ddsd.lpSurface + y * ddsd.lPitch);
		for (x = 0; x < ddsd.dwWidth; x++)
		{
			// Write RGBA
			u16 s = pSrc[x];

			pDst[2] = ((s & dwMaskR) >> nShiftR_R) << nShiftR_L;
			pDst[1] = ((s & dwMaskG) >> nShiftG_R) << nShiftG_L;
			pDst[0] = ((s & dwMaskB) >> nShiftB_R) << nShiftB_L;
			pDst[3] = 255;

			pDst += 4;

		}
	}
}



void PngSaveSurface(LPCTSTR szFileName, LPDIRECT3DTEXTURE8 pSurf)
{
	HRESULT hr;
	LONG nBytes;
	BYTE * pBytes;
	DDSURFACEDESC2 ddsd;
	ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
	ddsd.dwSize = sizeof(ddsd);
	hr = pSurf->Lock(NULL, &ddsd, DDLOCK_NOSYSLOCK|DDLOCK_WAIT , NULL);

	if (FAILED(hr))
		return;

	nBytes = ddsd.dwWidth * ddsd.dwHeight * 4;
	pBytes = new BYTE[nBytes];

	if (pBytes != NULL)
	{
		switch (ddsd.ddpfPixelFormat.dwRGBBitCount)
		{
		case 32:
			PngConvert32To32(ddsd, pBytes);
			break;
		case 24:
			PngConvert24To32(ddsd, pBytes);
			break;
		case 16:
			PngConvert16To32(ddsd, pBytes);
			break;
		}

		PngSaveImage(szFileName, pBytes, ddsd.dwWidth, ddsd.dwHeight);

		delete [] pBytes;
	}
	pSurf->Unlock(NULL);



}



#endif // LIBPNG_SUPPORT
