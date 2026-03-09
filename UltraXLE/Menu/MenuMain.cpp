#include "../ultra.h"
#include <xtl.h>
#include "../../config.h"
extern void MainMenu();
extern void ClearIGM();
#include "XBApp.h"

extern IDirect3DDevice8	*g_pDevice;

void SetupD3D()
{
	D3DXMATRIX mat;
	D3DXMatrixOrthoOffCenterLH(&mat, 0, init.gfxwid, init.gfxhig, 0, 0.0f, 1.0f);
	g_pDevice->SetTransform(D3DTS_PROJECTION, &mat);

	// use an identity matrix for the world and view matrices
	D3DXMatrixIdentity(&mat);
	g_pDevice->SetTransform(D3DTS_WORLD, &mat);
	g_pDevice->SetTransform(D3DTS_VIEW, &mat);

	// disable lighting
	g_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	g_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);
	g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0);
	g_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	g_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	g_pDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0xffffffff);
	g_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	g_pDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
	g_pDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);

	D3DVIEWPORT8 vp;
	vp.X = 0;
	vp.Y = 0;
	vp.Width = init.gfxwid;
	vp.Height = init.gfxhig;
	vp.MinZ = 0.0;
	vp.MaxZ = 1.0;
	g_pDevice->SetViewport(&vp);
}

void RestoreD3D()
{
//	ConfigAppLoad();
	D3DXMATRIX mat;
	
	//fd : Aspect ratio
	if(!bEnableHDTV){
		//4:3 for SD
		D3DXMatrixPerspectiveFovLH(&mat, D3DXToRadian(90.0f), (float)(init.gfxwid/init.gfxhig), 0.9f, 32768.0f);
	}else{
		//4:3 for HD
		D3DXMatrixPerspectiveFovLH(&mat, D3DXToRadian(90.0f), (float)(960/720), 0.9f, 32768.0f);
	}
	g_pDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX *)&mat);

	D3DXMatrixIdentity(&mat);
	g_pDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX *)&mat);

	D3DXMatrixTranslation(&mat, 0, 0, 0);
	g_pDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX *)&mat);
	
	g_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	g_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	g_pDevice->SetTextureStageState(0, D3DTSS_MINFILTER, TextureMode);
	g_pDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, TextureMode);
	
	g_pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	g_pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

	g_pDevice->SetRenderState( D3DRS_SHADEMODE,  D3DSHADE_GOURAUD );
	g_pDevice->SetRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_RGBA(0, 0, 0, 0));

	g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	g_pDevice->SetRenderState(D3DRS_ALPHAREF, 0x0c);
	g_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	g_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	g_pDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0);
	g_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	g_pDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
	g_pDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
}

void MenuMain()
{
SetupD3D();
MainMenu();
ClearIGM();
RestoreD3D();
}
