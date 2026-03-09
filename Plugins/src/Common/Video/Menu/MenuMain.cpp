#include "..\stdafx_video.h"
#include "MenuMain.h"
#include "D3DSave.h"

extern bool bEnableHDTV;

static void SetupD3D()
{
	D3DXMATRIX mat;
	DWORD videoFlags = XGetVideoFlags();
	
	D3DXMatrixOrthoOffCenterLH(&mat, 0, 640, 480, 0, 0.0f, 1.0f);

	if(XGetAVPack() == XC_AV_PACK_HDTV) {
		if(videoFlags & XC_VIDEO_FLAGS_HDTV_720p && bEnableHDTV) {
			D3DXMatrixOrthoOffCenterLH(&mat, 0, 1280, 720, 0, 0.0f, 1.0f);
		}
	}
	g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mat);

	// use an identity matrix for the world and view matrices
	D3DXMatrixIdentity(&mat);
	g_pd3dDevice->SetTransform(D3DTS_WORLD, &mat);
	g_pd3dDevice->SetTransform(D3DTS_VIEW, &mat);

	// disable lighting
	g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
}

extern void MainMenu(void);
extern void ClearIGM();

void RunIngameMenu()
{
	// save and init d3d
	g_d3dSave.PushRenderState();

	SetupD3D();

	MainMenu();
	ClearIGM();
	
	g_d3dSave.PopRenderState();

}

