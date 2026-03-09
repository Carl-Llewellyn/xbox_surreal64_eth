#pragma once

class D3DSave
{
public:
	D3DSave(void);
	~D3DSave(void);

	void PushRenderState();
	void PopRenderState();

private:
	void LoadDefaultRenderState();

private:
	DWORD m_renderState[100];
	DWORD m_stageState[150];
	D3DXMATRIX m_transformState[15];
	D3DVIEWPORT8 m_viewport;
};

extern D3DSave g_d3dSave;