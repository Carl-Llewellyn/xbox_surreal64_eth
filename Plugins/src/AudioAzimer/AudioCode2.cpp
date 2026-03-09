#ifdef _XBOX
#include <xtl.h>
#else
#include <windows.h>
#endif
#include <dsound.h>
#include <stdio.h>
#include "AudioCode.h"
#include "Audio.h"

DWORD last_pos = 0, write_pos = 0, play_pos = 0, temp = 0;
DWORD last_write = -1;
LPVOID lpvPtr1, lpvPtr2;
DWORD dwBytes1, dwBytes2;

LPDIRECTSOUNDBUFFER lpdsbuff;

DWORD buffsize = 0;
DWORD laststatus = 0;

// Fills up a buffer and remixes the audio
void AudioCode::FillBuffer (BYTE *buff, DWORD len) {
	DWORD cnt = 0;
	DWORD x = 0;
	DWORD pastFill = readLoc;
	DWORD pastLoc = (len - remainingBytes)/2;

	if (remainingBytes == 0) {
		memset (buff, 0, len);
		return;
	}
//	if (remainingBytes < len) {
//		float ratio = (float)remainingBytes/(float)len;
//		while (cnt != len) {
//		}
//		readLoc = writeLoc;
//	}
	while ((remainingBytes > 0) && (cnt != len)) { // Optimize this copy routine later
		buff[cnt] = SoundBuffer[readLoc];
		cnt++; readLoc++; remainingBytes--;
		if (readLoc == MAXBUFFER)
			readLoc = 0;
	}
	//pastFill += pastLoc&~0x3;
	while (cnt != len) {
		//buff[cnt] = SoundBuffer[pastFill++];
		//if (pastFill == MAXBUFFER)
		//	pastFill = 0;
		//buff[cnt] = 0;
		cnt++;
	}
}


DWORD WINAPI AudioThreadProc (AudioCode *ac) {
	DWORD dwStatus;
	//LPDIRECTSOUNDBUFFER  lpdsbuf = ac->lpdsbuf;
	LPDIRECTSOUND        lpds  = ac->lpds;

	lpdsbuff = ac->lpdsbuf;

	while (lpdsbuff == NULL)
		Sleep (10);

	IDirectSoundBuffer_GetStatus(lpdsbuff,&dwStatus);
		if ((dwStatus & DSBSTATUS_PLAYING) == 0) {
			IDirectSoundBuffer_Play(lpdsbuff, 0, 0, 0 );
	}

	SetThreadPriority (ac->handleAudioThread, THREAD_PRIORITY_HIGHEST);

	while (ac->audioIsDone == false) { // While the thread is still alive
		while (last_pos == write_pos) { // Cycle around until a new buffer position is available
			if (lpdsbuff == NULL)
				ExitThread (-1);
			WaitForSingleObject (ac->hMutex, INFINITE);
			if FAILED(lpdsbuff->GetCurrentPosition((unsigned long*)&play_pos, NULL)) {
				//MessageBox (NULL, "Error getting audio position...", "AudioLLE Error", MB_OK|MB_ICONSTOP);
				goto _exit_;
			}
			ReleaseMutex (ac->hMutex);
			if (play_pos < LOCK_SIZE) write_pos = (LOCK_SIZE * SEGMENTS) - LOCK_SIZE;
			else write_pos = ((play_pos / LOCK_SIZE) * LOCK_SIZE) - LOCK_SIZE;

			if (write_pos == last_pos) {
				Sleep (1);
			}
		}
		last_pos = write_pos;
		WaitForSingleObject (ac->hMutex, INFINITE);
		if (DS_OK != lpdsbuff->Lock(write_pos, LOCK_SIZE, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0)) {
			//MessageBox (NULL, "Error locking sound buffer", "Audio LLE", MB_OK|MB_ICONSTOP);
			goto _exit_;
		}
		ac->FillBuffer ((BYTE *)lpvPtr1, dwBytes1);
		if (dwBytes2)
			ac->FillBuffer ((BYTE *)lpvPtr2, dwBytes2);
		//
		if FAILED(lpdsbuff->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2)) {
			//MessageBox(NULL, "Error unlocking sound buffer","Audio LLE", MB_OK|MB_ICONSTOP);
			goto _exit_;
		}
		ReleaseMutex(ac->hMutex);
		//Sleep (10);
	}

_exit_:
	ReleaseMutex(ac->hMutex);
	ac->handleAudioThread = NULL;
	ExitThread (0);
	return 0;
}





//------------------------------------------------------------------------

// Setup and Teardown Functions

// Generates nice alignment with N64 samples...
extern BOOL bAudioBoostAzimer;
void AudioCode::SetSegmentSize (DWORD length) {

    DSBUFFERDESC        dsbdesc;
    WAVEFORMATEX        wfm;
    HRESULT             hr;

    if (SampleRate == 0) { return; }
	SegmentSize = length;

	WaitForSingleObject (hMutex, INFINITE);
    memset( &wfm, 0, sizeof( WAVEFORMATEX ) ); 

	wfm.wFormatTag = WAVE_FORMAT_PCM;
	wfm.nChannels = 2;
	wfm.nSamplesPerSec = SampleRate;
	wfm.wBitsPerSample = 16;
	wfm.nBlockAlign = wfm.wBitsPerSample / 8 * wfm.nChannels;
	wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;

    memset( &dsbdesc, 0, sizeof( DSBUFFERDESC ) ); 
    dsbdesc.dwSize = sizeof( DSBUFFERDESC ); 
    //dsbdesc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
	//dsbdesc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_LOCSOFTWARE;
    dsbdesc.dwBufferBytes = SegmentSize * SEGMENTS;  
    dsbdesc.lpwfxFormat = &wfm; 

	if ( FAILED( hr = IDirectSound_CreateSoundBuffer(lpds, &dsbdesc, &lpdsbuf, NULL ) ) ) {
		__asm int 3;
		return;
	}

	if (bAudioBoostAzimer) {
	DSMIXBINS			dsmb;

	DSMIXBINVOLUMEPAIR dsmbvp[8] = {
    {DSMIXBIN_FRONT_LEFT, DSBVOLUME_MAX},
    {DSMIXBIN_FRONT_RIGHT, DSBVOLUME_MAX},
    {DSMIXBIN_FRONT_CENTER, DSBVOLUME_MAX},
    {DSMIXBIN_FRONT_CENTER, DSBVOLUME_MAX},
    {DSMIXBIN_BACK_LEFT, DSBVOLUME_MAX},
    {DSMIXBIN_BACK_RIGHT, DSBVOLUME_MAX},
    {DSMIXBIN_LOW_FREQUENCY, DSBVOLUME_MAX},
    {DSMIXBIN_LOW_FREQUENCY, DSBVOLUME_MAX}};
	
	IDirectSoundBuffer_SetVolume(lpdsbuf, DSBVOLUME_MAX);
	IDirectSoundBuffer_SetHeadroom(lpdsbuf, DSBHEADROOM_MIN);

	dsmb.dwMixBinCount = 8;
	dsmb.lpMixBinVolumePairs = dsmbvp;
	IDirectSoundBuffer_SetMixBins(lpdsbuf, &dsmb);
	}	
	
	IDirectSoundBuffer_Play(lpdsbuf, 0, 0, DSBPLAY_LOOPING );
	lpdsbuff = this->lpdsbuf;
	ReleaseMutex (hMutex);
}

void AiCallBack (DWORD n) {}

BOOL AudioCode::Initialize (void (*AiCallBack)( DWORD ), HWND hwnd) {
	return Initialize (hwnd);
}

BOOL AudioCode::Initialize (HWND hwnd) {
	audioIsPlaying = FALSE;
 
    HRESULT             hr;

	DeInitialize (); // Release just in case...

	hMutex = CreateMutex(NULL,FALSE,NULL);

	WaitForSingleObject (hMutex, INFINITE);

    if ( FAILED( hr = DirectSoundCreate( NULL, &lpds, NULL ) ) ) {
		__asm int 3;
        return FALSE;
	}
 

	ReleaseMutex (hMutex);

	SetSegmentSize (LOCK_SIZE);

	return TRUE;
}

void AudioCode::DeInitialize () {

	StopAudio ();
    if (lpdsbuf) { 
		IDirectSoundBuffer_Stop(lpdsbuf);
		IDirectSoundBuffer_Release(lpdsbuf);
		lpdsbuf = NULL;
	}
    if ( lpds ) {
		IDirectSound_Release(lpds);
		lpds = NULL;
	}
	if (hMutex) { 
		CloseHandle(hMutex); 
		hMutex = NULL;
	}
}

// ---------BLAH--------
//#define u32 DWORD // redef

static u32 Frequency = 0;
static u32 Length = 0;
static double CountsPerByte;
static u32 SecondBuff = 0;
static u32 CurrentCount;
static u32 CurrentLength;
static u32 IntScheduled = 0;
//extern u32 VsyncTiming; // Need this so I can time things to VSync...

// Buffer Functions for the Audio Code
void AudioCode::SetFrequency (DWORD Frequency) {
	SampleRate = Frequency;
	SegmentSize = 0; // Trash it... we need to redo the Frequency anyway...
	SetSegmentSize (LOCK_SIZE);
}

DWORD AudioCode::AddBuffer (BYTE *start, DWORD length) {
	DWORD retVal = 0;
	DWORD max = MAXBUFFER;

	if (length == 0)
		return 0;
/*
	// Set Status to FULL for a few COUNT cycles
	if (CurrentLength == 0) {
		CurrentLength = length;
		CurrentCount = *AudioInfo.MMU_COUNT_RG;
		IntScheduled = (u32)((double)length * CountsPerByte);
		AudioInfo.SetInterruptTimer (IntScheduled);
	} else {
		SecondBuff = length;
		*AudioInfo.AI_STATUS_RG |= 0x80000000;
//		ScheduleEvent (AI_DMA_EVENT, 20, 0, NULL);
	}
	return 0;
*/
	if (!audioIsPlaying)
		StartAudio ();

	while (remainingBytes >= (MAXBUFFER-LOCK_SIZE)) {
		Sleep(1);
	}
	WaitForSingleObject (hMutex, INFINITE);


	for (DWORD x = 0; x < length; x++) {
		SoundBuffer[writeLoc] = start[x];
		writeLoc++; remainingBytes++;
		if (writeLoc == MAXBUFFER)
			writeLoc = 0;
	}

	ReleaseMutex (hMutex);

	return retVal;
}

// Management functions
// TODO: For silent emulation... the Audio should still be "processed" somehow...
void AudioCode::StopAudio () {
	if (!audioIsPlaying) return;
	audioIsPlaying = FALSE;
	//TerminateThread (this->handleAudioThread, 0);
	ExitThread (0);
}

void AudioCode::StartAudio () {
	if (audioIsPlaying) return;
	audioIsPlaying = TRUE;
	this->handleAudioThread = CreateThread (NULL, NULL, (LPTHREAD_START_ROUTINE)AudioThreadProc, this, NULL, &this->dwAudioThreadId);
	writeLoc = 0x0000;
	readLoc = 0x0000;
	remainingBytes = 0;
}

DWORD AudioCode::GetReadStatus () {
	return 0;
}
