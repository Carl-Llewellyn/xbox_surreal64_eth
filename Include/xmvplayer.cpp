//-----------------------------------------------------------------------------
// File: XMVPlayer.cpp
//
// Desc: Example use of playing XMV content
//
// Hist: 2.7.03 - Created
//
//          XMV playback has a few principle variations:
//              Playing to a texture or to the overlay planes
//              Using the packet interface to read the file, using packets to copy from
//              memory, or letting XMV do the reading.
//              Using Play() to play the entire movie, or GetNextFrame
//              Unpacking to an RGB or YUV texture
//              Playing full screen or on just part of the screen
//
//          If these could be combined arbitrarily this would give us dozens of combinations.
//          A few of the combinations don't make sense - overlay planes are always YUV,
//          Play() always uses the overlay planes, etc.
//
//          Many of these variations - such as using CreateDecoderForFile versus
//          CreateDecoderForPackets - do not affect other aspects of playback, so the
//          different variations can be mixed without difficulty.
//
//          This sample tries to show all sensible combinations of these possibilities
//
//          When playing to an overlay plane this sample also demonstrates placing other
//          graphics above the movie being played.
//
//          The playback logic is encapsulated in the XMVHelper class, so the actual
//          playback process is pretty simple.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include <XBApp.h>
#include <XBFont.h>
//#include <XBHelp.h>
#include <xgraphics.h>

#include <xmv.h>
#include "XMVHelper.h"
//#include "../Launcher/musicmanager.h"
//extern CMusicManager  music;
extern LPDIRECT3DDEVICE8 g_pd3dDevice;
extern bool EnableVideoAudio;
extern int iMovieTop;
extern int iMovieBottom;
extern int iMovieLeft;
extern int iMovieRight;

string activeMovie;

//-----------------------------------------------------------------------------
// Name: g_colorKey
// Desc: Used when color keying is enabled
//-----------------------------------------------------------------------------
const D3DCOLOR g_colorKey = D3DCOLOR_ARGB( 0xFF, 0x3F, 0x00, 0x3F );

//-----------------------------------------------------------------------------
// Name: g_fullScreenRect and g_partialScreenRect
// Desc: Rectangles to use when playing movies full or partial screen.
//-----------------------------------------------------------------------------
const RECT g_fullScreenRect = { 0, 0, 640, 480 };
RECT g_partialScreenRect = { iMovieTop, iMovieLeft, iMovieRight, iMovieBottom };

//-----------------------------------------------------------------------------
// Name: CXMVSample
// Desc: Constructor for CXMVSample class
//-----------------------------------------------------------------------------
CXMVSample::CXMVSample() 
{
    m_bFullScreen =         FALSE;
    m_readMethod =          READ_FROM_PACKETS;
    m_bYUV =                TRUE; //FALSE;
    m_bUseTextures =        TRUE;
    m_bUsePlay =            FALSE;
    m_bUseColorKey =        FALSE;
}

CXMVSample::~CXMVSample(void)
{
	m_player.Destroy();
	activeMovie.clear();
}

//-----------------------------------------------------------------------------
// Name: Initialize
// Desc: Performs initialization
//-----------------------------------------------------------------------------
HRESULT CXMVSample::Init(char* name)
{
	const char* strMovieName = name;//"D:\\Media\\Videos\\Test.xmv";

	if ( m_player.IsPlaying() )
	{
		// Halt movie playback
		m_player.Destroy();
	}
	
	return PlayVideoWithGetNextFrame( strMovieName );
}

//-----------------------------------------------------------------------------
// Name: Render
// Desc: Renders the scene
//-----------------------------------------------------------------------------
HRESULT CXMVSample::Render()
{
    PlayVideoFrame();
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: OpenMovie()
// Desc: Open a movie file in one of the three supported ways.
//-----------------------------------------------------------------------------
HRESULT CXMVSample::OpenMovie( const CHAR* strFilename, D3DFORMAT format, BOOL bAllocateTextures )
{
    HRESULT hr = E_FAIL;
    switch ( m_readMethod )
    {
        case READ_FROM_FILE:
            hr = m_player.OpenFile( strFilename, format, g_pd3dDevice, bAllocateTextures );
            break;
        case READ_FROM_PACKETS:
            hr = m_player.OpenFileForPackets( strFilename, format, g_pd3dDevice, bAllocateTextures  );
            break;
        case READ_FROM_MEMORY:
            hr = m_player.OpenMovieFromMemory( strFilename, format, g_pd3dDevice, bAllocateTextures  );
            break;
    }
    return hr;
}

//-----------------------------------------------------------------------------
// Name: MoviePlayerThread()
// Desc: This function is used in a separate thread so that the main thread can
// do other tasks, such as loading data, or just checking for the user to press A.
// Alternately the movie can be played in the main thread, with a sub-thread
// to check for button presses.
//-----------------------------------------------------------------------------
DWORD __stdcall MoviePlayerThread( void* pMovieData )
{
    assert( pMovieData );
    CXMVPlayer *pPlayer = ( CXMVPlayer* )pMovieData;

    // Play the movie
    // Can also be played in a subrectangle by specifying a rectangle, but that
    // rarely makes sense with the Play() interface.
    pPlayer->Play( XMVFLAG_NONE, &g_partialScreenRect );

    return 0;
}

//-----------------------------------------------------------------------------
// Name: SimplePlayVideo()
// Desc: Plays specified video file.
//-----------------------------------------------------------------------------
HRESULT CXMVSample::PlayVideoWithPlay( const CHAR* strFilename )
{
    // When using Play(), which uses overlays, we have to use D3DFMT_YUY2
    D3DFORMAT format = D3DFMT_YUY2;

    // We can use the file or memory or packet interface - it doesn't matter.
    HRESULT hr = OpenMovie( strFilename, format, FALSE );
    if ( FAILED( hr ) )
        return hr;

    // Start the movie stopping thread - this should always succeed.
    HANDLE hThread = CreateThread( 0, 0, &MoviePlayerThread, &m_player, 0, 0 );

    // Loop waiting for the user to press A or the movie to exit.
    // Resource loading or other activity can be placed here.
    for ( ;; )
    {
        // Wait a little while, or until the movie thread exits.
        // Can do useful work here.
        DWORD waitResult = WaitForSingleObject( hThread, 1000 / 60 );

        // WAIT_OBJECT_0 means the thread exited and we should exit.
        if ( waitResult == WAIT_OBJECT_0 )
            break;

        // Refresh the input data.
        XBInput_GetInput( g_Gamepads );

        // See if the user has pressed A on any of the controllers.
        for ( int i = 0; i < ( sizeof( g_Gamepads ) / sizeof( g_Gamepads[0] ) ); ++i )
        {
            if( g_Gamepads[i].bPressedAnalogButtons[XINPUT_GAMEPAD_B] )
            {
                // If the user presses A, terminate the currently playing movie.
                // This may take a few hundred milliseconds.
                m_player.TerminatePlayback();
                goto exit;
            }
        }
    }

exit:
    // We have to make sure the thread is terminated *before* we close the movie
    // decoder, to make sure it has stopped referencing the movie player object.

    // Wait for the thread to terminate.
    WaitForSingleObject( hThread, INFINITE );

    // Clean up our thread handles to free all thread resources.
    // This has to be done after we finish waiting on the handle.
    CloseHandle( hThread );

    // Free all movie playback resources.
    m_player.Destroy();

    return hr;
}

//-----------------------------------------------------------------------------
// Name: TexturePlayVideo()
// Desc: Plays specified video file on a texture.
//-----------------------------------------------------------------------------
HRESULT CXMVSample::PlayVideoWithGetNextFrame( const CHAR* strFilename )
{
    assert( !m_player.IsPlaying() );

    // If we're not currently playing a movie then start playing one.
    D3DFORMAT format = D3DFMT_YUY2;
    //if ( m_bUseTextures )
    {
        if ( !m_bYUV )
        {
            // The only non-YUV formats allowed are D3DFMT_LIN_A8R8G8B8 and D3DFMT_LIN_X8R8G8B8
            format = D3DFMT_LIN_A8R8G8B8;
        }
    }

    // We can use the file or memory or packet interface - it doesn't matter.
    HRESULT hr = OpenMovie( strFilename, format, TRUE );

    return hr;
}

//-----------------------------------------------------------------------------
// Name: PlayVideoFrame()
// Desc: Plays one frame of video if a movie is currently open and if there is
// a frame available. This function is safe to call at any time.
//-----------------------------------------------------------------------------
BOOL CXMVSample::PlayVideoFrame()
{
    if ( !m_player.IsPlaying() )
        {
			
			m_player.Destroy();
			Init((char*)activeMovie.c_str());
		
		
		return FALSE;
	}

    const FLOAT fMovieWidth = FLOAT( m_player.GetWidth() );
    const FLOAT fMovieHeight = FLOAT( m_player.GetHeight() );

    // Move to the next frame.
    LPDIRECT3DTEXTURE8 pTexture = 0;
    if ( m_bUseTextures )
        pTexture = m_player.AdvanceFrameForTexturing( g_pd3dDevice );
    else
        pTexture = m_player.AdvanceFrameForOverlays( g_pd3dDevice );

    // See if the movie is over now.
    if ( !m_player.IsPlaying() )
    {
        // Clean up the movie, then return.
        m_player.Destroy();
        return FALSE;
    }

    // If no texture is ready, return TRUE to indicate that a movie is playing,
    // but don't render anything yet.
    if ( !pTexture )
        return TRUE;

        
// Have the texture start small and scale up, just to prove it's on
        // a texture.
        /*const DWORD FRAMES_TO_EASE_IN = 40;
        FLOAT fRatio = 1.0;
        if ( m_player.GetCurrentFrame() < FRAMES_TO_EASE_IN )
            fRatio = FLOAT( m_player.GetCurrentFrame() ) / FRAMES_TO_EASE_IN;*/



        // Draw the texture.
        g_pd3dDevice->SetRenderState( D3DRS_FILLMODE,         D3DFILL_SOLID );
        g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,         D3DCULL_CCW );
        g_pd3dDevice->SetRenderState( D3DRS_ZENABLE,          FALSE );

        // Draw the texture as a quad.
        g_pd3dDevice->SetTexture( 0, pTexture );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );

        // Wrapping isn't allowed on linear textures.
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSU,  D3DTADDRESS_CLAMP );
        g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESSV,  D3DTADDRESS_CLAMP );

        // If we're unpacking to a YUV surface we have to tell the hardware that we
        // are rendering from a YUV surface.
        if ( m_bYUV )
            g_pd3dDevice->SetRenderState( D3DRS_YUVENABLE, TRUE );


        // On linear textures the texture coordinate range is from 0,0 to width,height, instead
        // of 0,0 to 1,1.
	g_pd3dDevice->SetVertexShader( D3DFVF_XYZRHW|D3DFVF_TEX1 );
	g_pd3dDevice->Begin( D3DPT_QUADLIST );
	
	
		g_pd3dDevice->SetVertexData2f( D3DVSDE_TEXCOORD0, 0, fMovieHeight );
		g_pd3dDevice->SetVertexData4f( D3DVSDE_VERTEX, (float)iMovieLeft,  (float)iMovieBottom, 0.0f, 1.0f );
		g_pd3dDevice->SetVertexData2f( D3DVSDE_TEXCOORD0, 0, 0 );
		g_pd3dDevice->SetVertexData4f( D3DVSDE_VERTEX, (float)iMovieLeft,  (float)iMovieTop,    0.0f, 1.0f );
		g_pd3dDevice->SetVertexData2f( D3DVSDE_TEXCOORD0, fMovieWidth, 0 );
		g_pd3dDevice->SetVertexData4f( D3DVSDE_VERTEX, (float)iMovieRight, (float)iMovieTop,    0.0f, 1.0f );
		g_pd3dDevice->SetVertexData2f( D3DVSDE_TEXCOORD0, fMovieWidth, fMovieHeight );
		g_pd3dDevice->SetVertexData4f( D3DVSDE_VERTEX, (float)iMovieRight, (float)iMovieBottom, 0.0f, 1.0f );


	g_pd3dDevice->End();

        // If we switched to YUV texturing then we need to switch back.
        if ( m_bYUV )
            g_pd3dDevice->SetRenderState( D3DRS_YUVENABLE, FALSE );
		return TRUE;
}
   

CXMVSample moviePlayer;
//extern string activeMovie;

bool initMovie(char* moviename) {

	
	if(moviename == activeMovie)
		return true;
	//dprintf("Start Movie: %s\n", name);
	if(FAILED(moviePlayer.Init(moviename))) {
		activeMovie.clear();
		/*if(EnableVideoAudio){
			music.Play();
		}*/
		return false;
	}
	//dprintf("Success Movie: %s\n", name);
	
	activeMovie = moviename;
	/*if(EnableVideoAudio){
		music.Pause();
	}*/

	//moviePlayer.PlayVideoWithPlay(moviename);
    return TRUE;
}
void drawMovie() {

	moviePlayer.Render();
	/*if(EnableVideoAudio){
		music.Pause();
	}*/
}

bool getPlaying() {
	return (moviePlayer.m_player.IsPlaying() ? true : false);
}


void stopMovie() {
	
	if ( moviePlayer.m_player.IsPlaying() )
	{
			// Clean up the movie, then return.
		moviePlayer.m_player.Destroy();
	}	

	/*if(EnableVideoAudio){
		music.Play();
	}*/
}
void resetMovie(){
	char *movieName;
	movieName = new char[activeMovie.length() + 1];
	strcpy(movieName, activeMovie.c_str());
	stopMovie();
	initMovie(movieName);
	drawMovie();
}
