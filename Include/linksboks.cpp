

#include "linksboks.h"

extern "C" {
#include "links-hacked/links.h"
#include "links-hacked/options_hooks.h"
}

// Restore code and data sections to normal.
#pragma code_seg()
#pragma data_seg()
#pragma bss_seg()
#pragma const_seg()

LPDIRECT3D8			g_pLBD3D;			/* Provided d3d object */
LPDIRECT3DDEVICE8	g_pLBd3dDevice;		/* Provided d3d device object */
LinksBoksOption		**g_LinksBoksOptions = NULL;
LinksBoksProtocol	**g_LinksBoksProtocols = NULL;
int (*g_LinksBoksExecFunction)(LinksBoksWindow *pLB, unsigned char *cmdline, unsigned char *filepath, int fg) = NULL;

extern LinksBoksWindow		*g_NewLinksBoksWindow = NULL;

// Yet Another Ugly Thing. Do not do this at home.
extern "C" int __LinksBoks_InitCore(void);
extern "C" int __LinksBoks_NewWindow(void);
extern "C" int __LinksBoks_FrameMove(void);
extern "C" void __LinksBoks_Terminate(void);

extern "C" struct window *get_root_or_first_window(struct terminal *term);
extern "C" void open_in_new_tab(struct terminal *term, unsigned char *exe, unsigned char *param);

// The custom FVF, which describes the custom vertex structure.
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

struct CUSTOMVERTEX
{
    D3DXVECTOR3 position; // The position.
    D3DCOLOR    color;    // The color.
    FLOAT       tu, tv;   // The texture coordinates.
};





/******************* OPTIONS *********************/



BOOL LinksBoks_GetOptionBool(const char *key)
{
	return options_get_bool((unsigned char *)key);
}

INT LinksBoks_GetOptionInt(const char *key)
{
	return options_get_int((unsigned char *)key);
}

unsigned char *LinksBoks_GetOptionString(const char *key)
{
	return options_get((unsigned char *)key);
}

void LinksBoks_SetOptionBool(const char *key, BOOL value)
{
	options_set_bool((unsigned char *)key, (int)value);
}

void LinksBoks_SetOptionInt(const char *key, INT value)
{
	options_set_int((unsigned char *)key, (int)value);
}

void LinksBoks_SetOptionString(const char *key, unsigned char *value)
{
	options_set((unsigned char *)key, value);
}


extern "C" void options_copy_item(void *in, void *out);

OPTIONS_HOOK(xbox_hook)
{
	struct options *in_opt = (struct options *)current;
	struct options *out_opt = (struct options *)changed;

	for(int i = 0; g_LinksBoksOptions[i]; i++)
	{
		if(g_LinksBoksOptions[i]->m_sName == NULL)
			continue;
		if(!strcmp((const char *)in_opt->name, (const char *)g_LinksBoksOptions[i]->m_sName))
		{
			int ret;
			int iOld = 0, iNew = 0;
			switch(g_LinksBoksOptions[i]->m_iType)
			{
			case LINKSBOKS_OPTION_BOOL:
			case LINKSBOKS_OPTION_INT:
				if(in_opt->value && sscanf((const char *)in_opt->value,"%d",&iOld)
						&& out_opt->value && sscanf((const char *)out_opt->value,"%d",&iNew))
					ret = g_LinksBoksOptions[i]->OnBeforeChange(ses, iOld, iNew);

				break;
			case LINKSBOKS_OPTION_STRING:
				ret = g_LinksBoksOptions[i]->OnBeforeChange(ses, in_opt->value, out_opt->value);
				break;
			}

			if(ret)
			{
				options_copy_item(changed, current);
				g_LinksBoksOptions[i]->OnAfterChange(ses);
			}

			return ret;
		}
	}

	return -1;
}

extern "C" void register_options_xbox(void)
{
	if(g_LinksBoksOptions == NULL)
		return;

	for(int i = 0; g_LinksBoksOptions[i]; i++)
		g_LinksBoksOptions[i]->Register();
}

LinksBoksOption::LinksBoksOption(const char *name, const char *caption, int type, int depth, unsigned char *default_value)
{
	m_sName = name;
	m_sCaption = caption;
	m_iType = type;
	m_iDepth = depth;
	m_sDefaultValue = default_value;
}

LinksBoksOption::LinksBoksOption(const char *name, const char *caption, int type, int depth, int default_value)
{
	m_sName = name;
	m_sCaption = caption;
	m_iType = type;
	m_iDepth = depth;
	m_iDefaultValue = default_value;
}

BOOL LinksBoksOption::OnBeforeChange(void *session, unsigned char *oldvalue, unsigned char *newvalue)
{
	return TRUE;
}

BOOL LinksBoksOption::OnBeforeChange(void *session, int oldvalue, int newvalue)
{
	return TRUE;
}

VOID LinksBoksOption::OnAfterChange(void *session)
{
}

VOID LinksBoksOption::MsgBox(void *session, unsigned char *title, unsigned char *msg)
{
	struct session *ses = (struct session *)session;
	msg_box(ses->term, NULL, title, AL_CENTER, msg, NULL, 1, TXT(T_OK), NULL, B_ENTER | B_ESC);
}

VOID LinksBoksOption::Register()
{
	switch(m_iType)
	{
	case LINKSBOKS_OPTION_GROUP:
		register_option(NULL, (unsigned char *)m_sCaption, OPT_TYPE_CHAR, NULL, m_iDepth);
		break;
	case LINKSBOKS_OPTION_BOOL:
		register_option((unsigned char *)m_sName, (unsigned char *)m_sCaption, OPT_TYPE_BOOL, (unsigned char *)"", m_iDepth);
		options_set_bool((unsigned char *)m_sName, m_iDefaultValue);
		break;
	case LINKSBOKS_OPTION_INT:
		register_option((unsigned char *)m_sName, (unsigned char *)m_sCaption, OPT_TYPE_INT, (unsigned char *)"", m_iDepth);
		options_set_int((unsigned char *)m_sName, m_iDefaultValue);
		break;
	case LINKSBOKS_OPTION_STRING:
		register_option_char((unsigned char *)m_sName, (unsigned char *)m_sCaption, (unsigned char *)m_sDefaultValue, m_iDepth);
		break;
	}
	options_set_hook((unsigned char *)m_sName, xbox_hook);
}






/******************* CUSTOM PROTOCOLS *********************/





VOID LinksBoksExternalProtocol::Register()
{
	register_external_protocol(m_sName, m_iPort, NULL, xbox_external_protocols_func, m_bFreeSyntax, m_bNeedSlashes, m_bNeedSlashAfterHost);
}

VOID LinksBoksInternalProtocol::Register()
{
	register_external_protocol(m_sName, m_iPort, xbox_internal_protocols_func, NULL, m_bFreeSyntax, m_bNeedSlashes, m_bNeedSlashAfterHost);
}



extern "C" void xbox_internal_protocols_func(struct connection *connection)
{
	for(int i = 0; g_LinksBoksProtocols[i]; i++)
	{
		if(g_LinksBoksProtocols[i]->m_sName == NULL)
			continue;
		if(!strcmp((const char *)get_protocol_name(connection->url), (const char *)g_LinksBoksProtocols[i]->m_sName))
		{
			setcstate(connection, g_LinksBoksProtocols[i]->OnCall(connection->url, connection));
			abort_connection(connection);
			return;
		}
	}

	setcstate(connection, S_BAD_URL);
	abort_connection(connection);
}

extern "C" void xbox_external_protocols_func(struct session *session, unsigned char *url)
{
	for(int i = 0; g_LinksBoksProtocols[i]; i++)
	{
		if(g_LinksBoksProtocols[i]->m_sName == NULL)
			continue;
		if(!strcmp((const char *)get_protocol_name(url), (const char *)g_LinksBoksProtocols[i]->m_sName))
		{
			g_LinksBoksProtocols[i]->OnCall(url, session);
			return;
		}
	}
}

VOID LinksBoksExternalProtocol::MsgBox(void *session, unsigned char *title, unsigned char *msg)
{
	struct session *ses = (struct session *)session;
	msg_box(ses->term, NULL, title, AL_CENTER, msg, NULL, 1, TXT(T_OK), NULL, B_ENTER | B_ESC);
}

int LinksBoksInternalProtocol::SendResponse(void *connection, unsigned char *content_type, unsigned char *data, int data_size)
{
	struct connection *c = (struct connection *)connection;
	struct cache_entry *e;
	char *head = new char[19 + strlen((const char *)content_type)];
	if (get_cache_entry(c->url, &e))
		return -1;

	if(e->head)
		mem_free(e->head);

	snprintf(head, 18 + strlen((const char *)content_type), "\r\nContent-Type: %s\r\n", content_type);
	e->head = (unsigned char *)head;
	c->cache = e;

	add_fragment(e, 0, data, data_size);
	truncate_entry(e, data_size, 1);

	c->cache->incomplete = 0;

	return 0;
}


/******************* MAIN ROUTINES *********************/




int LinksBoks_InitCore(LinksBoksOption *options[], LinksBoksProtocol *protocols[])
{
	XLoadSection("LBOX_RD");
	XLoadSection("LBOX_RW");

	g_LinksBoksOptions = options;
	g_LinksBoksProtocols = protocols;

	/* We can register the protocols right now */
	if(g_LinksBoksProtocols != NULL)
	{
		for(int i = 0; g_LinksBoksProtocols[i]; i++)
			g_LinksBoksProtocols[i]->Register();
	}

	return __LinksBoks_InitCore();
}

LinksBoksWindow *LinksBoks_CreateWindow(LPDIRECT3D8 pD3D, LPDIRECT3DDEVICE8 pd3dDevice, LinksBoksViewPort viewport)
{
	g_pLBD3D = pD3D;
	g_pLBd3dDevice = pd3dDevice;
	g_NewLinksBoksWindow = new LinksBoksWindow(viewport);

    if(__LinksBoks_NewWindow())
	{
		delete g_NewLinksBoksWindow;
		g_NewLinksBoksWindow = NULL;
		return NULL;
	}
	else
	{
		LinksBoksWindow *pLB = g_NewLinksBoksWindow;
		g_NewLinksBoksWindow = NULL;
		return pLB;
	}
}

VOID LinksBoks_Terminate(BOOL bFreeXBESections)
{
	__LinksBoks_Terminate();
	if(bFreeXBESections)
	{
		XFreeSection("LBOX_RD");
		XFreeSection("LBOX_RW");
		XFreeSection("BFONTS");
	}
}


/******************* TIMERS *********************/




int LinksBoks_RegisterNewTimer(long t, void (*func)(void *), void *data)
{
	return install_timer(t, func, data);
}





/******************* EXTERNAL VIEWERS *********************/




/* Replacement for function in terminal.c */
extern "C" void exec_on_terminal(struct terminal *term, unsigned char *path, unsigned char *file, int fg)
{
	LinksBoksWindow *pLB = (LinksBoksWindow *)term->dev->driver_data;

	if(g_LinksBoksExecFunction && pLB)
	{
		int ret = g_LinksBoksExecFunction(pLB, path, file, fg);

		if(ret >= 0)
			unlink((const char *)file);
	}
}

VOID LinksBoks_SetExecFunction(int (*exec_function)(LinksBoksWindow *pLB, unsigned char *cmdline, unsigned char *filepath, int fg))
{
	g_LinksBoksExecFunction = exec_function;
}






/******************* WINDOWS *********************/





LinksBoksWindow::LinksBoksWindow(LinksBoksViewPort viewport)
{
	m_ViewPort = viewport;
	m_pD3D = g_pLBD3D;
	m_pd3dDevice = g_pLBd3dDevice;
	m_bWantFlip = FALSE;
}

int LinksBoksWindow::Initialize(void *grdev)
{
	m_grdev = grdev;

	SetRect( &m_FlipRegion, 0, 0, m_ViewPort.width, m_ViewPort.height );

	/* Create the surfaces and initiate rendering */
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,          TRUE );
//    m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
//    m_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE );
	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   FALSE );
	m_pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
	m_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	m_pd3dDevice->SetRenderState( D3DRS_CULLMODE,         D3DCULL_NONE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTEXF_NONE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTEXF_LINEAR );

    // Note: The hardware requires CLAMP for linear textures
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP );

    m_pd3dDevice->CreateRenderTarget( m_ViewPort.width, m_ViewPort.height, D3DFMT_LIN_X8R8G8B8, 0, 0, &m_pdSurface );
    m_pd3dDevice->CreateRenderTarget( m_ViewPort.width, m_ViewPort.height, D3DFMT_LIN_X8R8G8B8, 0, 0, &m_pdBkBuffer );

	// Erase the 2 rendering surfaces
	CreatePrimitive( -m_ViewPort.margin_left, -m_ViewPort.margin_top, m_ViewPort.width, m_ViewPort.height, 0x00000000 );
	
	// The first...
	RenderPrimitive( m_pdSurface );

	// And the second
	RenderPrimitive( m_pdBkBuffer );

	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );

	m_pd3dDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX );

	// And finally, tell Links to finish initing itself */
	return 0;
}





/******************* RENDERING *********************/




LPDIRECT3DSURFACE8 LinksBoksWindow::GetSurface(void)
{
	return m_pdSurface;
}

int LinksBoksWindow::FlipSurface(void)
{
	if( !m_bWantFlip )
		return 1;

	m_pd3dDevice->CopyRects( m_pdBkBuffer, NULL, 0, m_pdSurface, NULL );

	m_bWantFlip = FALSE;

	return 0;
}

static void xbox_flip_surface (void *pointer)
{
	LinksBoksWindow *pLB = (LinksBoksWindow *)pointer;

	pLB->FlipSurface();
}

VOID LinksBoksWindow::RegisterFlip( int x, int y, int w, int h )
{
	if (x < 0 || y < 0 || w < 1 || h < 1)
		return;

	w = x + w - 1;
	h = y + h - 1;

	if( m_bWantFlip )
	{
		if (m_FlipRegion.left > x)  m_FlipRegion.left = (LONG)x;
		if (m_FlipRegion.top > y)  m_FlipRegion.top = (LONG)y;
		if (m_FlipRegion.right < w)  m_FlipRegion.right = (LONG)w;
		if (m_FlipRegion.bottom < h)  m_FlipRegion.bottom = (LONG)h;
	}
	else
	{
		m_FlipRegion.left = x;
		m_FlipRegion.top = y;
		m_FlipRegion.right = w;
		m_FlipRegion.bottom = h;

		m_bWantFlip = TRUE;

		register_bottom_half (xbox_flip_surface, this);
	}
}

VOID LinksBoksWindow::Blit( LPDIRECT3DSURFACE8 pSurface, int x, int y, int w, int h )
{
	RECT srcRect, dstRect;
	RECT screenRect = { 0, 0, GetViewPortWidth(), GetViewPortHeight() };

	// Don't forget to consider the clipping region
	SetRect( &srcRect, x, y, x+w, y+h );
	IntersectRect( &dstRect, &srcRect, &m_ClipArea );
	IntersectRect( &dstRect, &dstRect, &screenRect );

	if( !IsRectEmpty( &dstRect ) && dstRect.right - dstRect.left > 1 && dstRect.bottom - dstRect.top > 1 )
	{
		POINT destPoint;
		destPoint.x = dstRect.left + m_ViewPort.margin_left;
		destPoint.y = dstRect.top + m_ViewPort.margin_top;
		OffsetRect( &dstRect, -x, -y );
		m_pd3dDevice->CopyRects( pSurface, &dstRect, 1, m_pdBkBuffer, &destPoint );
	}
}


HRESULT LinksBoksWindow::CreatePrimitive( int x, int y, int w, int h, int color )
{
	LPDIRECT3DVERTEXBUFFER8 pVB = NULL; // Buffer to hold vertices
	float left, top, right, bottom;

	// Create the vertex buffer. Here we are allocating enough memory
    // (from the default pool) to hold all our 3 custom vertices. We also
    // specify the FVF, so the vertex buffer knows what data it contains.
    if( FAILED( m_pd3dDevice->CreateVertexBuffer( 3*sizeof(CUSTOMVERTEX),
                                                  0, 
                                                  D3DFVF_CUSTOMVERTEX,
                                                  D3DPOOL_DEFAULT, &pVB ) ) )
        return E_FAIL;

	
	CUSTOMVERTEX* pVertices;

	if( FAILED( pVB->Lock( 0, 4*sizeof(CUSTOMVERTEX), (BYTE**)&pVertices, 0 ) ) )
		return E_FAIL;


	left	= ((float)(m_ViewPort.margin_left + x) / (float)m_ViewPort.width) * 2.0f - 1.0f;
	right	= ((float)(m_ViewPort.margin_left + x + w) / (float)m_ViewPort.width) * 2.0f - 1.0f;
	top		= 1.0f - ((float)(m_ViewPort.margin_top + y) / (float)m_ViewPort.height) * 2.0f;
	bottom	= 1.0f - ((float)(m_ViewPort.margin_top + y + h) / (float)m_ViewPort.height) * 2.0f;


	pVertices[0].position=D3DXVECTOR3( left,  top, 0.5f );
	pVertices[0].color=color;
	pVertices[0].tu=0.0;
	pVertices[0].tv=0.0;
	pVertices[1].position=D3DXVECTOR3( right,  top, 0.5f );
	pVertices[1].color=color;
	pVertices[1].tu=(FLOAT)w;
	pVertices[1].tv=0.0;
	pVertices[2].position=D3DXVECTOR3( left,  bottom, 0.5f );
	pVertices[2].color=color;
	pVertices[2].tu=0.0;
	pVertices[2].tv=(FLOAT)h;
	pVertices[3].position=D3DXVECTOR3(  right,  bottom, 0.5f );
	pVertices[3].color=color;
	pVertices[3].tu=(FLOAT)w;
	pVertices[3].tv=(FLOAT)h;
	
	pVB->Unlock();

	m_pd3dDevice->SetStreamSource( 0, pVB, sizeof(CUSTOMVERTEX) );
    m_pd3dDevice->SetVertexShader( D3DFVF_CUSTOMVERTEX );

	pVB->Release();

	return S_OK;
}

VOID LinksBoksWindow::RenderPrimitive( LPDIRECT3DSURFACE8 pTargetSurface )
{
	LPDIRECT3DSURFACE8 pRenderTarget, pZBuffer;
    m_pd3dDevice->GetRenderTarget( &pRenderTarget );
    m_pd3dDevice->GetDepthStencilSurface( &pZBuffer );

	m_pd3dDevice->SetRenderTarget( pTargetSurface, NULL );

    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

    m_pd3dDevice->SetRenderTarget( pRenderTarget, pZBuffer );

	if( pRenderTarget ) pRenderTarget->Release();
    if( pZBuffer )      pZBuffer->Release();
}

VOID LinksBoksWindow::FillArea(int x1, int y1, int x2, int y2, long color)
{
	RECT srcRect, dstRect;
	RECT screenRect = { 0, 0, GetViewPortWidth(), GetViewPortHeight() };

    D3DLOCKED_RECT d3dlr;
    m_pdBkBuffer->LockRect( &d3dlr, 0, 0 );
	DWORD * pDst = (DWORD *)d3dlr.pBits;
	int DPitch = d3dlr.Pitch/4;

	// Don't forget to consider the clipping region
	SetRect( &srcRect, x1, y1, x2, y2 );
	IntersectRect( &dstRect, &srcRect, &m_ClipArea );
	IntersectRect( &dstRect, &dstRect, &screenRect );

	if( !IsRectEmpty( &dstRect ) )
		for (int y=dstRect.top; y<dstRect.bottom; ++y)
			for (int x=dstRect.left; x<dstRect.right; ++x)
				pDst[(y+m_ViewPort.margin_top)*DPitch + (x+m_ViewPort.margin_left)] = color;

	m_pdBkBuffer->UnlockRect( );

	RegisterFlip( x1, y1, x2 - x1, y2 - y1 );
}

VOID LinksBoksWindow::DrawHLine(int x1, int y, int x2, long color)
{
	RECT srcRect, dstRect;

	D3DLOCKED_RECT d3dlr;
    m_pdBkBuffer->LockRect( &d3dlr, 0, 0 );
	DWORD * pDst = (DWORD *)d3dlr.pBits;
	int DPitch = d3dlr.Pitch/4;

	// Don't forget to consider the clipping region
	SetRect( &srcRect, x1, y, x2, y+1 );
	IntersectRect( &dstRect, &srcRect, &m_ClipArea );

	if( !IsRectEmpty( &dstRect ) )
		for (int x=dstRect.left; x<dstRect.right; ++x)
			pDst[(y+m_ViewPort.margin_top)*DPitch + (x+m_ViewPort.margin_left)] = color;

	m_pdBkBuffer->UnlockRect( );

	RegisterFlip( x1, y, x2 - x1, 1 );
}

VOID LinksBoksWindow::DrawVLine(int x, int y1, int y2, long color)
{
	RECT srcRect, dstRect;

    D3DLOCKED_RECT d3dlr;
    m_pdBkBuffer->LockRect( &d3dlr, 0, 0 );
	DWORD * pDst = (DWORD *)d3dlr.pBits;
	int DPitch = d3dlr.Pitch/4;

	// Don't forget to consider the clipping region
	SetRect( &srcRect, x, y1, x+1, y2 );
	IntersectRect( &dstRect, &srcRect, &m_ClipArea );

	if( !IsRectEmpty( &dstRect ) )
		for (int y=dstRect.top; y<dstRect.bottom; ++y)
			pDst[(y+m_ViewPort.margin_top)*DPitch + (x+m_ViewPort.margin_left)] = color;

	m_pdBkBuffer->UnlockRect( );

	RegisterFlip( x, y1, 1, y2 - y1 );
}

VOID LinksBoksWindow::ScrollBackBuffer(int x1, int y1, int x2, int y2, int offx, int offy)
{
	LPDIRECT3DSURFACE8 pSurface = NULL;
	RECT srcRect;
	POINT destPoint = { 0, 0 };

	m_pd3dDevice->CreateImageSurface( x2 - x1, y2 - y1, D3DFMT_LIN_A8R8G8B8, &pSurface );

	SetRect( &srcRect, x1, y1, x2, y2 );
	OffsetRect( &srcRect, m_ViewPort.margin_left, m_ViewPort.margin_top );

	m_pd3dDevice->CopyRects( m_pdBkBuffer, &srcRect, 1, pSurface, &destPoint );

	OffsetRect( &srcRect, -m_ViewPort.margin_left + offx, -m_ViewPort.margin_top + offy );
	Blit( pSurface, srcRect.left, srcRect.top, srcRect.right - srcRect.left, srcRect.bottom - srcRect.top);

	pSurface->Release( );
}

VOID LinksBoksWindow::SetClipArea(int x1, int y1, int x2, int y2)
{
	SetRect( &m_ClipArea, x1, y1, x2, y2 );
}

int LinksBoksWindow::GetViewPortWidth(void)
{
	return m_ViewPort.width - m_ViewPort.margin_left - m_ViewPort.margin_right;
}

int LinksBoksWindow::GetViewPortHeight(void)
{
	return m_ViewPort.height - m_ViewPort.margin_top - m_ViewPort.margin_bottom;
}

void LinksBoksWindow::ResizeWindow(LinksBoksViewPort viewport)
{
	m_ViewPort = viewport;
	struct graphics_device *dev = (struct graphics_device *)m_grdev;

	dev->size.x2 = GetViewPortWidth();
	dev->size.y2 = GetViewPortHeight();

	dev->resize_handler(dev);

	// Black-fill the edges of the front buffer to clean up garbage
	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	CreatePrimitive( -m_ViewPort.margin_left, -m_ViewPort.margin_top, m_ViewPort.width, m_ViewPort.margin_top, 0x000000ff );
	RenderPrimitive( m_pdBkBuffer );
	CreatePrimitive( -m_ViewPort.margin_left, -m_ViewPort.margin_top, m_ViewPort.margin_left, m_ViewPort.height, 0x000000ff );
	RenderPrimitive( m_pdBkBuffer );
	CreatePrimitive( -m_ViewPort.margin_left, GetViewPortHeight(), m_ViewPort.width, m_ViewPort.margin_bottom, 0x000000ff );
	RenderPrimitive( m_pdBkBuffer );
	CreatePrimitive( GetViewPortWidth(), -m_ViewPort.margin_top, m_ViewPort.margin_right, m_ViewPort.height, 0x000000ff );
	RenderPrimitive( m_pdBkBuffer );
	m_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
}




/******************* KEYBOARD/MOUSE ACTIONS *********************/




VOID LinksBoksWindow::KeyboardAction(int key, int flags)
{
	struct graphics_device *dev = (struct graphics_device *)m_grdev;
	dev->keyboard_handler(dev, key, flags);
}

VOID LinksBoksWindow::MouseAction(int x, int y, int buttons)
{
	struct graphics_device *dev = (struct graphics_device *)m_grdev;
	dev->mouse_handler(dev, x, y, buttons);
}




/******************* SESSION ACTIONS *********************/




VOID LinksBoksWindow::GoToURL(unsigned char *url)
{
	struct graphics_device *dev = (struct graphics_device *)m_grdev;

	struct terminal *term = (struct terminal *)dev->user_data;
	struct window *win = get_root_or_first_window(term);

	goto_url((struct session *)win->data, url);
}

VOID LinksBoksWindow::GoToURLInNewTab(unsigned char *url)
{
	struct graphics_device *dev = (struct graphics_device *)m_grdev;

	struct terminal *term = (struct terminal *)dev->user_data;

	open_in_new_tab(term, NULL, url);
}

int LinksBoksWindow::NumberOfTabs()
{
	struct graphics_device *dev = (struct graphics_device *)m_grdev;
	struct terminal *term = (struct terminal *)dev->user_data;

	return number_of_tabs(term);
}

VOID LinksBoksWindow::SwitchToTab(int index)
{
	struct graphics_device *dev = (struct graphics_device *)m_grdev;
	struct terminal *term = (struct terminal *)dev->user_data;

	switch_to_tab(term, index);
}

VOID LinksBoksWindow::CloseCurrentTab()
{
	struct graphics_device *dev = (struct graphics_device *)m_grdev;
	struct terminal *term = (struct terminal *)dev->user_data;

	close_tab(term);
}




/******************* TERMINATION *********************/




VOID LinksBoksWindow::Close(void)
{
	struct graphics_device *dev = (struct graphics_device *)m_grdev;
	dev->keyboard_handler(dev, KBD_CLOSE, 0);
}

VOID LinksBoksWindow::Terminate(void)
{
	m_pD3D = NULL;
	m_pd3dDevice = NULL;
	m_pdSurface->Release();
	m_pdSurface = NULL;
	m_pdBkBuffer->Release();
	m_pdBkBuffer = NULL;
	unregister_bottom_half (xbox_flip_surface, this);

}
