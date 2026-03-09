
#ifdef LIBPNG_SUPPORT

#include <png.h>

BOOL PngSaveImage (LPCTSTR pstrFileName, png_byte *pDiData,
                   int iWidth, int iHeight);

void PngSaveSurface(LPCTSTR szFileName, LPDIRECT3DTEXTURE8 pSurf);

#endif //LIBPNG_SUPPORT
