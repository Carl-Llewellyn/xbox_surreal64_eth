#include "MusicManager.h"
#include "XbUtil.h"
#include <assert.h>
#include <stdio.h>
#include <tchar.h>
#include "Util.h"

//#define DEBUG_MUSIC_MAN 1

extern bool EnableBGMusic;
extern bool EnableVideoAudio;
extern char szPathSkins[256];
extern char skinname[32];
extern void ShowTempMessage(const char *msg);
extern bool bAudioBoost;

//extern bool g_fDirectSoundDisableBusyWaitWarning;

#define MAX_TRACK_PATH 256
const DWORD WMA_LOOKAHEAD = 64 * 1024;

//-----------------------------------------------------------------------------
CMusicManager::CMusicManager()
{
	m_mmState		= MM_STOPPED;
	m_bRandom		= FALSE;
	m_dwStream		= 0;
	m_dwSong		= 0;
	m_dwSongID		= 0;
	m_bInitialized	= FALSE;

	m_dwPacketsCompleted	= 0;

	m_pSampleData	= NULL;
	m_pDecoder		= NULL;
	m_pStream[0]	= NULL;
	m_pStream[1]	= NULL;
	m_hDecodingFile	= INVALID_HANDLE_VALUE;
}

//-----------------------------------------------------------------------------
CMusicManager::~CMusicManager()
{
	// This takes care of the decoder and file handle
	Cleanup();

	if( m_pStream[0] )
		m_pStream[0]->Release();
	m_pStream[0] = NULL;

	if( m_pStream[1] )
		m_pStream[1]->Release();
	m_pStream[1] = NULL;

	if( m_pSampleData )
		delete[] m_pSampleData;
	
	m_trackList.clear();
}

//-----------------------------------------------------------------------------
bool CMusicManager::Build()
{
	HANDLE 				hFind;
	WIN32_FIND_DATA		oFindData;
	char				szPathTracks[MAX_TRACK_PATH];
	
	sprintf(szPathTracks, "%s%s\\", szPathSkins, skinname);
	m_szTrackPath = string(szPathTracks);
	
	sprintf(szPathTracks, "%s*.wma", m_szTrackPath.c_str());
	hFind = FindFirstFile(szPathTracks, &oFindData);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if(oFindData.cFileName[0] == '.' || (oFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				continue;
			} else {
				string szFilename (oFindData.cFileName);
			
				// uppercase first letter for sort - optional
				if ((szFilename[0]>=97) && (szFilename[0]<=122))
					szFilename[0] = szFilename[0]-32;
			
				m_trackList.push_back(szFilename);
			}
		} while (FindNextFile(hFind, &oFindData));
	}
	FindClose(hFind);
	
	sort(m_trackList.begin(), m_trackList.end(), StringRLTL);
	
#ifdef DEBUG_MUSIC_MAN
	char dbg[256];
	sprintf(dbg, "MM: Tracks:%i\n", GetSongCount());
	OutputDebugString(dbg);
#endif

	if (GetSongCount() > 0)
		return TRUE;
	else
		return FALSE;
}

//-----------------------------------------------------------------------------
HRESULT CMusicManager::Initialize()
{
	HRESULT hr;
	
	if (Build()) {
		m_bInitialized = TRUE;
	} else {
		OutputDebugString("MM: Initialization Failed!\n");
		m_bInitialized = FALSE;
		return E_FAIL;
	}
	
	//g_fDirectSoundDisableBusyWaitWarning = TRUE;

	if( m_bRandom )
		SelectSong( rand() % GetSongCount() );
	else
		SelectSong( 0 );

	srand( GetTickCount() );

	// Allocate sample data buffer
	m_pSampleData = new BYTE[ ( PACKET_COUNT + EXTRA_PACKETS ) * PACKET_SIZE ];
	if( !m_pSampleData ) {
		OutputDebugString("MM: Out of Memory!\n");
		return E_OUTOFMEMORY;
	}

    // All user-created soundtracks are created with the same standard wave format: 44.1KHz, 16-bit, stereo PCM.
    // Title-supplied soundtracks should be the same format.
	// If the title needs to use soundtracks with a different format, they can use the SetFormat() method to change the stream format.
	WAVEFORMATEX wfx	= {0};
	wfx.wFormatTag		= WAVE_FORMAT_PCM;
	wfx.nChannels		= 2;
	wfx.nSamplesPerSec	= 44100;
	wfx.wBitsPerSample	= 16;
	wfx.nBlockAlign		= wfx.nChannels * wfx.wBitsPerSample / 8;
	wfx.nAvgBytesPerSec	= wfx.nBlockAlign * wfx.nSamplesPerSec;

	// Set up the stream descriptor so we can create our streams
	DSSTREAMDESC dssd			= {0};
	dssd.dwMaxAttachedPackets	= PACKET_COUNT;
	dssd.lpwfxFormat			= &wfx;
	dssd.lpfnCallback			= StreamCallback;
	dssd.lpvContext				= this;

	// Create the streams
	hr = DirectSoundCreateStream( &dssd, &m_pStream[0] );
	if( FAILED( hr ) ) {
		OutputDebugString("MM: DirectSoundCreateStream 0 Failed!\n");
		return hr;
	}
	hr = DirectSoundCreateStream( &dssd, &m_pStream[1] );
	if( FAILED( hr ) ) {
		OutputDebugString("MM: DirectSoundCreateStream 1 Failed!\n");
		return hr;
	}
	
	// Set up amplitude envelopes to handle the fade-in/fade-out
	DSENVELOPEDESC dsed	= {0};
	dsed.dwEG			= DSEG_AMPLITUDE;
	dsed.dwMode			= DSEG_MODE_ATTACK;
	dsed.dwAttack		= DWORD( 48000 * FADE_TIME / 512 );
	dsed.dwRelease		= DWORD( 48000 * FADE_TIME / 512 );
	dsed.dwSustain		= 255;
	m_pStream[0]->SetEG( &dsed );
	m_pStream[1]->SetEG( &dsed );

	if (bAudioBoost) {
	DSMIXBINVOLUMEPAIR dsmbvp[8] = {
	{DSMIXBIN_FRONT_LEFT, DSBVOLUME_MAX},
	{DSMIXBIN_FRONT_RIGHT, DSBVOLUME_MAX},
	{DSMIXBIN_FRONT_CENTER, DSBVOLUME_MAX},
	{DSMIXBIN_FRONT_CENTER, DSBVOLUME_MAX},
	{DSMIXBIN_BACK_LEFT, DSBVOLUME_MAX},
	{DSMIXBIN_BACK_RIGHT, DSBVOLUME_MAX},
	{DSMIXBIN_LOW_FREQUENCY, DSBVOLUME_MAX},
	{DSMIXBIN_LOW_FREQUENCY, DSBVOLUME_MAX}};

	DSMIXBINS dsmb;
	dsmb.dwMixBinCount = 8;
	dsmb.lpMixBinVolumePairs = dsmbvp;

	m_pStream[0]->SetVolume(DSBVOLUME_MAX);
	m_pStream[1]->SetVolume(DSBVOLUME_MAX);

	m_pStream[0]->SetHeadroom(DSBHEADROOM_MIN);
	m_pStream[1]->SetHeadroom(DSBHEADROOM_MIN);

	m_pStream[0]->SetMixBins(&dsmb);
	m_pStream[1]->SetMixBins(&dsmb);
	}

	#if 0
	//#if _DEBUG
	if( FAILED( DebugVerify() ) )
		OutputDebugString("MM: DebugVerify Failed!\n");
		return E_FAIL;
	}
	#endif // _DEBUG
	
#ifdef DEBUG
	OutputDebugString("MM: Initialized!\n");
#endif

	return S_OK;
}

//-----------------------------------------------------------------------------
HRESULT CMusicManager::Play()
{
	if (!m_bInitialized || !EnableBGMusic || EnableVideoAudio) return E_FAIL;
	
#ifdef DEBUG_MUSIC_MAN
	OutputDebugString("MM: Play\n");
#endif

	// Make sure the streams are unpaused
	if( m_mmState == MM_STOPPED )
		Prepare();
	else if( m_mmState == MM_PAUSED )
	{
		if( m_pStream[0] )
			m_pStream[0]->Pause( DSSTREAMPAUSE_RESUME );
		if( m_pStream[1] )
			m_pStream[1]->Pause( DSSTREAMPAUSE_RESUME );
	}

	m_mmState = MM_PLAYING;

	return S_OK;
}

//-----------------------------------------------------------------------------
HRESULT CMusicManager::Stop()
{
	if (!m_bInitialized) return E_FAIL;
	
#ifdef DEBUG_MUSIC_MAN
	OutputDebugString("MM: Stop\n");
#endif

	// Can always transition to stopped
	if( m_mmState != MM_STOPPED )
	{
		if( m_pStream[0] )
			m_pStream[0]->FlushEx( 0, DSSTREAMFLUSHEX_ASYNC );
		if( m_pStream[1] )
			m_pStream[1]->FlushEx( 0, DSSTREAMFLUSHEX_ASYNC );
		Cleanup();
	}
	
	m_mmState = MM_STOPPED;

	// added by GogoAckman
	// This takes care of the decoder and file handle
	Cleanup();

	if( m_pStream[0] )
		m_pStream[0]->Release();
	m_pStream[0] = NULL;

	if( m_pStream[1] )
		m_pStream[1]->Release();
	m_pStream[1] = NULL;

	if( m_pSampleData )
		delete[] m_pSampleData;
	
	m_trackList.clear();

	return S_OK;
}

//-----------------------------------------------------------------------------
HRESULT CMusicManager::Pause()
{
	if (!m_bInitialized) return E_FAIL;

	if(m_mmState != MM_PLAYING) {
		return S_FALSE;
	}
	
#ifdef DEBUG_MUSIC_MAN
	OutputDebugString("MM: Pause\n");
#endif
	
	// Can only transition to paused from playing.
	if( m_mmState == MM_PLAYING )
	{
		if( m_pStream[0] )
			m_pStream[0]->Pause( DSSTREAMPAUSE_PAUSE );
		if( m_pStream[1] )
			m_pStream[1]->Pause( DSSTREAMPAUSE_PAUSE );
	}

	m_mmState = MM_PAUSED;

	return S_OK;
}

//-----------------------------------------------------------------------------
HRESULT CMusicManager::SetRandom( bool bRandom )
{
	m_bRandom = bRandom;
	
#ifdef DEBUG_MUSIC_MAN
	char dbg[256];
	sprintf(dbg, "MM: Random:%i\n", (m_bRandom ? 1 : 0));
	OutputDebugString(dbg);
#endif
	
	return S_OK;
}

//-----------------------------------------------------------------------------
HRESULT CMusicManager::SetVolume( float fVolume )
{
	if (!m_bInitialized) return E_FAIL;
	
#ifdef DEBUG_MUSIC_MAN
	OutputDebugString("MM: SetVolume\n");
#endif

	m_fVolume = fVolume;

	if( m_pStream[0] )
		m_pStream[0]->SetVolume( (LONG)fVolume );
	if( m_pStream[1] )
		m_pStream[1]->SetVolume( (LONG)fVolume );

	return S_OK;
}

//-----------------------------------------------------------------------------
float CMusicManager::GetPlaybackPosition()
{
    return PACKET_TIME * m_dwPacketsCompleted;
}

//-----------------------------------------------------------------------------
HRESULT CMusicManager::NextSong()
{
	if (!m_bInitialized || (GetSongCount() < 2) || !EnableBGMusic || EnableVideoAudio)
		return E_FAIL;
		
#ifdef DEBUG_MUSIC_MAN
	OutputDebugString("MM: NextSong\n");
#endif

	// Can always transition to stopped
	if( m_mmState != MM_STOPPED )
	{
		if( m_pStream[0] )
			m_pStream[0]->FlushEx( 0, DSSTREAMFLUSHEX_ASYNC );
		if( m_pStream[1] )
			m_pStream[1]->FlushEx( 0, DSSTREAMFLUSHEX_ASYNC );
		Cleanup();
	}
	
	m_mmState = MM_STOPPED;

	if( m_mmState == MM_STOPPED )
	{
		m_mmState = MM_PLAYING;
		return FindNextSong();
	}

	m_mmState = MM_PLAYING;

	return S_FALSE;
}

//-----------------------------------------------------------------------------
HRESULT CMusicManager::RandomSong()
{
	if (!m_bInitialized) return E_FAIL;
	
#ifdef DEBUG_MUSIC_MAN
	OutputDebugString("MM: RandomSong\n");
#endif

	if( m_mmState == MM_STOPPED )
	{
		return SelectSong( rand() % GetSongCount() );
	}
	else {
		return S_FALSE;
	}
}

//-----------------------------------------------------------------------------
HRESULT CMusicManager::SelectSong( DWORD dwSong )
{
	if (!m_bInitialized) return E_FAIL;
	
#ifdef DEBUG_MUSIC_MAN
	OutputDebugString("MM: SelectSong\n");
#endif

	m_dwSong = dwSong;
	GetSongInfo( m_dwSong, &m_dwSongID );
	//Prepare();
	
	if (!EnableBGMusic || EnableVideoAudio)
		return E_FAIL;
	else
		Prepare();
	
	// osd track message if there's multiple files
	if (GetSongCount() > 1) {
		char tmpmsg[256];
		sprintf(tmpmsg, "Playing: %s", GetTrackName(m_dwSong).c_str());
		ShowTempMessage(tmpmsg);
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
HRESULT CMusicManager::FindNextSong()
{
	if (!m_bInitialized) return E_FAIL;
	
#ifdef DEBUG_MUSIC_MAN
	OutputDebugString("MM: FindNextSong\n");
#endif

	if( m_bRandom ) {
		DWORD dwRandom = rand() % GetSongCount();
		
		if (dwRandom == m_dwSong) { // same song
			// 100 tries to get a different song
			for( UINT i=0; i<100; i++ ) {
				dwRandom = rand() % GetSongCount();
				if (dwRandom != m_dwSong) break;
			}
		}
		
		SelectSong( dwRandom );
	} 
	else {
		SelectSong( ( m_dwSong + 1 ) % GetSongCount() );
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
HRESULT CMusicManager::Prepare()
{
	if (!m_bInitialized) return E_FAIL;
	
#ifdef DEBUG_MUSIC_MAN
	OutputDebugString("MM: Prepare\n");
#endif
	
	// Open the song
    m_hDecodingFile = OpenSong( m_dwSongID );

	// Create the new decoder
	WMAXMODECODERPARAMETERS Params	= {0};
	Params.hFile					= m_hDecodingFile;
	Params.dwLookaheadBufferSize	= WMA_LOOKAHEAD;

	HRESULT hr = XWmaDecoderCreateMediaObject( (LPCWMAXMODECODERPARAMETERS)&Params, &m_pDecoder );
	if( FAILED( hr ) ) {
		OutputDebugString("MM: XWmaDecoderCreateMediaObject Failed!\n");
		return hr;
	}
	
	return S_OK;
}

//-----------------------------------------------------------------------------
HRESULT CMusicManager::Cleanup()
{
	if (!m_bInitialized) return E_FAIL;
	
#ifdef DEBUG_MUSIC_MAN
	OutputDebugString("MM: Cleanup\n");
#endif
	
	// Free up the decoder
	if( m_pDecoder )
	{
		m_pDecoder->Release();
		m_pDecoder = NULL;
	}

	// Close the open file
	if( m_hDecodingFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle( m_hDecodingFile );
		m_hDecodingFile = INVALID_HANDLE_VALUE;
	}

	// Zero packets completed
	m_dwPacketsCompleted = 0;

	return S_OK;
}

//-----------------------------------------------------------------------------
HRESULT CMusicManager::Process()
{
	if (!m_bInitialized) return E_FAIL;

	HRESULT hr;
	DWORD   dwPacket;

	// If we're currently playing, then process packets.
	// Note that we could be playing, and not yet have a decoder if the worker thread is still trying to open the file.
	if( m_pDecoder )
	{
		// Pump the decoder so it can handle its asynchronous file I/O
		m_pDecoder->DoWork();

		// Process packets
		while( FindFreePacket( &dwPacket, m_dwStream ) )
		{
			XMEDIAPACKET xmp;

			hr = ProcessSource( dwPacket, &xmp );
			if( FAILED( hr ) ) {
				return hr;
			}
			
			// If we got a packet of size zero, that means that we either
			// 1) Hit the end of the current track, or 
			// 2) The decoder is not yet ready to produce output.  
			// Either way, we'll skip out, let the decoder do some work, and try again next time.
			if( xmp.dwMaxSize > 0 )
			{
				hr = ProcessStream( dwPacket, &xmp );
				if( FAILED( hr ) ) {
					return hr;
				}
			}
			else
				break;
		}
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
bool CMusicManager::FindFreePacket( DWORD* pdwPacket, DWORD dwStream )
{
    for( DWORD dwIndex = dwStream; dwIndex < PACKET_COUNT; dwIndex += 2 )
    {
        // The first EXTRA_PACKETS * 2 packets are reserved - odd packets
        // for stream 1, even packets for stream 2.  This is to ensure
        // that there are packets available during the crossfade
        if( XMEDIAPACKET_STATUS_PENDING != m_adwStatus[dwIndex] &&
            ( dwIndex > EXTRA_PACKETS * 2 || dwIndex % 2 == dwStream ) )
        {
            (*pdwPacket) = dwIndex;
            return TRUE;
        }
    }

    return FALSE;
}

//-----------------------------------------------------------------------------
HRESULT CMusicManager::ProcessSource( DWORD dwPacket, XMEDIAPACKET* pxmp )
{
	HRESULT      hr;
	DWORD        dwBytesDecoded;

	ZeroMemory( pxmp, sizeof( XMEDIAPACKET ) );

	// See if the decoder is ready to provide output yet
	DWORD dwStatus;
	m_pDecoder->GetStatus( &dwStatus );
	if( ( dwStatus & XMO_STATUSF_ACCEPT_OUTPUT_DATA ) == 0 ) {
		return S_FALSE;
	}
	
	// Set up the XMEDIAPACKET structure
	pxmp->pvBuffer          = m_pSampleData + dwPacket * PACKET_SIZE;
	pxmp->dwMaxSize         = PACKET_SIZE;
	pxmp->pdwCompletedSize  = &dwBytesDecoded;

	hr = m_pDecoder->Process( NULL, pxmp );
	if( FAILED( hr ) ) {
		return hr;
	}

	if( 0 == dwBytesDecoded )
	{
		// We hit the end of the current track.  Key the fade-out
		// Note that we queue an asynchronous flush to be done, and then (in Cleanup), we release the stream.
		// The flush should still occur on the next call to DirectSoundDoWork().
		m_pStream[m_dwStream]->Discontinuity();
		m_pStream[m_dwStream]->FlushEx( 0, DSSTREAMFLUSHEX_ASYNC | DSSTREAMFLUSHEX_ENVELOPE );

		// Clean up resources from this song, and get
		// ready for the next one:
		Cleanup();
		FindNextSong();
		m_dwStream = ( m_dwStream + 1 ) % 2;
	}

	pxmp->dwMaxSize = dwBytesDecoded;

	return S_OK;
}

//-----------------------------------------------------------------------------
HRESULT CMusicManager::ProcessStream( DWORD dwPacket, XMEDIAPACKET* pxmp )
{
    HRESULT      hr;

    // The XMEDIAPACKET should already have been filled out by the call to ProcessSource().
	// In addition, ProcessSource() should have properly set the dwMaxSize member to reflect how much data was decoded.
    pxmp->pdwStatus         = &m_adwStatus[dwPacket];
    pxmp->pdwCompletedSize  = NULL;
    pxmp->pContext          = (LPVOID)m_dwStream;

    hr = m_pStream[m_dwStream]->Process( pxmp, NULL );
    if( FAILED( hr ) ) {
        return hr;
	}

    return S_OK;
}

//-----------------------------------------------------------------------------
void CALLBACK StreamCallback( void* pStreamContext, void* pPacketContext, DWORD dwStatus )
{
    CMusicManager* pMusicManager = (CMusicManager*)pStreamContext;

    if( DWORD(pPacketContext) == pMusicManager->m_dwStream &&
        dwStatus == XMEDIAPACKET_STATUS_SUCCESS )
    {
        ++pMusicManager->m_dwPacketsCompleted;
    }
}

//-----------------------------------------------------------------------------
HANDLE CMusicManager::OpenSong( DWORD dwSongID )
{
	if (!m_bInitialized) return INVALID_HANDLE_VALUE;
	
#ifdef DEBUG_MUSIC_MAN
	char dbg[256];
	sprintf(dbg, "MM: OpenSong:%i\n", (int)dwSongID);
	OutputDebugString(dbg);
#endif

	return CreateFile( GetTrackFile(dwSongID).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, NULL );
}

//-----------------------------------------------------------------------------
UINT CMusicManager::GetSongCount()
{
	return m_trackList.size();
}

//-----------------------------------------------------------------------------
void CMusicManager::GetSongInfo( UINT uSongIndex, DWORD* pdwID) //, DWORD* pdwLength , WCHAR strName[MAX_SONG_NAME] )
{
	if (!m_bInitialized) return;
	
#ifdef DEBUG_MUSIC_MAN
	char dbg[256];
	sprintf(dbg, "MM: GetSongInfo: IDX:%i ID:%i\n", (int)uSongIndex, (int)pdwID);
	OutputDebugString(dbg);
#endif
	
	(*pdwID) = uSongIndex;
	
	DWORD dwSongCount = 0;
	for (vector<string>::iterator i = m_trackList.begin(); i != m_trackList.end(); ++i)
	{
		if (dwSongCount == (DWORD)uSongIndex)
		{
			m_szTrackFilename = (*i).c_str();
			break;
		}
		dwSongCount++;
	}
}

//-----------------------------------------------------------------------------
string CMusicManager::GetTrackName(DWORD pdwID)
{
#ifdef DEBUG_MUSIC_MAN
	char dbg[256];
	sprintf(dbg, "MM: GetTrackName:%i\n", (int)pdwID);
	OutputDebugString(dbg);
#endif

	if (m_szTrackFilename.length() > 4)
		return string (m_szTrackFilename, 0, (m_szTrackFilename.length() - 4)); // rem .wma
	else
		return m_szTrackFilename; // empty
}

//-----------------------------------------------------------------------------
string CMusicManager::GetTrackFile(DWORD pdwID)
{
	char pathtrack[MAX_TRACK_PATH];
	sprintf(pathtrack, "%s%s", m_szTrackPath.c_str(), m_szTrackFilename.c_str());
	
#ifdef DEBUG_MUSIC_MAN
	char dbg[256];
	sprintf(dbg, "MM: GetTrackFile:%s\n", pathtrack);
	OutputDebugString(dbg);
#endif

	return string(pathtrack);
}

//-----------------------------------------------------------------------------
bool CMusicManager::IsPlaying()
{
	if (GetStatus() == MM_PLAYING)
		return TRUE;
	else
		return FALSE;
}

#if _DEBUG
//-----------------------------------------------------------------------------
HRESULT CMusicManager::DebugVerify()
{
	if (!m_bInitialized) return E_FAIL;
	
#ifdef DEBUG_MUSIC_MAN
	OutputDebugString("MM: DebugVerify\n");
#endif

	// Verify we can open all track files.  This could take a while.
	for( UINT uSong = 0; uSong < GetSongCount(); uSong++ )
	{
		DWORD dwSongID;
		GetSongInfo( uSong, &dwSongID);
		HANDLE h = OpenSong( dwSongID );
		if( INVALID_HANDLE_VALUE == h )
		{
			char dbgmsg[256];
			sprintf(dbgmsg, "MM: Test Failed for Track: %s\n", GetTrackFile(dwSongID).c_str());
			OutputDebugString(dbgmsg);
			return E_FAIL;
		}
		CloseHandle( h );
	}

	return S_OK;
}
#endif // _DEBUG