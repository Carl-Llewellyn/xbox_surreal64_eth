#ifndef MUSICMANAGER_H
#define MUSICMANAGER_H

#include <xtl.h>

#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <vector>
#include <algorithm>
using namespace std;

// Packet size is set up to be 2048 samples
// Samples are 16-bit and stereo
const DWORD PACKET_SIZE		= 2048 * 2 * 2;
const float PACKET_TIME		= 2048.0f / 44100.0f;

const DWORD PACKET_COUNT	= 43;	// Base number of packets
const DWORD EXTRA_PACKETS	= 10;	// Extra packets to get through the transition
const float FADE_TIME		= PACKET_COUNT * PACKET_TIME;

enum MM_STATE
{
	MM_STOPPED,
	MM_PAUSED,
	MM_PLAYING,
};

class CMusicManager
{
public:
	CMusicManager();
	~CMusicManager();
	HRESULT Initialize();	// Initialize MusicManager

	HRESULT Play();								// Start playing
	HRESULT Stop();								// Stop playback
	HRESULT Pause();							// Pause playback
	MM_STATE GetStatus() { return m_mmState; }	// Returns current playback status
	HRESULT SetRandom( bool bRandom );			// Change random mode
	bool	GetRandom() { return m_bRandom; }	// Get random mode
	HRESULT SetVolume( float fVolume );			// Set volume level
	float   GetVolume() { return m_fVolume; }	// Get volume level

	// Returns info on currently playing song
	float   GetPlaybackPosition();	// Returns position in current song
	HRESULT NextSong();				// Switch to next song
	HRESULT RandomSong();			// Switch to a random song
	
	HRESULT Process();		// Workhorse function - update state and manage packets
	
	void	GetSongInfo( UINT uSongIndex, DWORD* pdwID);	// get track id and name
	HANDLE	OpenSong( DWORD dwSongID );						// open the stream
	bool	IsPlaying();									// true if playing music
	UINT	GetSongCount();									// number of tracks in list
	string	GetTrackName(DWORD pdwID);						// get the current tracks name without the extension
	string	GetTrackFile(DWORD pdwID);						// get the current tracks filename with path
	vector<string> m_trackList;								// list of tracks
	
private:
	HRESULT SelectSong( DWORD dwSong );	// Switch to a song
	HRESULT FindNextSong();				// Determine next song to play

	HRESULT Prepare();	// Get ready to decode and playback current song
	HRESULT Cleanup();	// Cleanup from playing current song

	bool	FindFreePacket( DWORD *pdwPacket, DWORD dwStream );		// Looks for a free packet
	HRESULT ProcessSource( DWORD dwPacket, XMEDIAPACKET * pxmp );	// Fills packet from source
	HRESULT ProcessStream( DWORD dwPacket, XMEDIAPACKET * pxmp );	// Submit packet to stream

	// Stream callback routine for updating playback position
	friend static void CALLBACK StreamCallback( LPVOID pStreamContext, LPVOID pPacketContext, DWORD dwStatus );

	// State variables
	bool		m_bRandom;				// TRUE to move randomly
	float		m_fVolume;				// Volume level
	MM_STATE	m_mmState;				// New state set from game thread
	DWORD		m_dwPacketsCompleted;	// # of packets completed

	// Music information
	DWORD		m_dwSong;			// Current song index
	DWORD		m_dwSongID;			// Current song ID
	HANDLE		m_hDecodingFile;	// Song file handle

	// Decode/playback members
	XWmaFileMediaObject*	m_pDecoder;		// WMA decoder
	IDirectSoundStream*		m_pStream[2];	// Output streams
	DWORD					m_dwStream;		// Current stream
	BYTE*					m_pSampleData;	// Audio sample data

	// Packet status values
	DWORD					m_adwStatus[PACKET_COUNT+EXTRA_PACKETS];
	
	bool	Build();			// search skin dir for wma files
	bool	m_bInitialized;		// true if there are vaild tracks to play
	string	m_szTrackPath;		// path to track files
	string	m_szTrackFilename;	// current track filename (no path)

#if _DEBUG
	HRESULT DebugVerify();	// Verify all game soundtracks are present
#endif
};

#endif // MUSICMANAGER_H
