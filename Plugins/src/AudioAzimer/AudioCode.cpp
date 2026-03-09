#ifdef _XBOX
#include <xtl.h>
#else
#include <windows.h>
#endif
#include <dsound.h>

#include <stdio.h>
#include "AudioCode.h"
#include "AudioSpec.h"

DWORD last_pos = 0, write_pos = 0, play_pos = 0, temp = 0;
DWORD last_write = -1;
LPVOID lpvPtr1, lpvPtr2;
DWORD dwBytes1, dwBytes2;

LPDIRECTSOUNDBUFFER lpdsbuff;

DWORD buffsize = 0;
DWORD laststatus = 0;

void UpdateStatus () {
	extern AUDIO_INFO AudioInfo;
	if (lpdsbuff == NULL)
		return;

	//*AudioInfo.AI_LEN_REG = buffsize;
	//return ;

	if FAILED(lpdsbuff->GetCurrentPosition((unsigned long*)&play_pos, NULL)) {
		//MessageBox (NULL, "Error getting audio position...", "AudioLLE Error", MB_OK|MB_ICONSTOP);
		return;
	}
	if (play_pos < buffsize) write_pos = (buffsize * SEGMENTS) - buffsize;
	else write_pos = ((play_pos / buffsize) * buffsize) - buffsize;

	int writediff = (write_pos - last_write);

	if (writediff < 0) {
		writediff += SEGMENTS * buffsize;
	}

	DWORD play_seg = play_pos / buffsize;
	DWORD write_seg = play_pos / buffsize;
	DWORD last_seg = last_write / buffsize;


	if (last_seg == write_seg) { // The FIFO is still full and DMA is busy...
		*AudioInfo.AI_STATUS_REG = 0xC0000000;
		*AudioInfo.AI_LEN_REG = buffsize-(play_pos-((play_pos/buffsize)*buffsize));
		if (play_pos > write_pos) {
			*AudioInfo.AI_LEN_REG = buffsize-((write_pos-play_pos)-buffsize);
		} else {
			*AudioInfo.AI_LEN_REG = (buffsize - play_pos);
		}
		*AudioInfo.AI_LEN_REG += buffsize;
		laststatus = 0xC0000000;
		return;
	}
	if (laststatus == 0xC0000000) { // Then we need to generate an interrupt now...
		*AudioInfo.AI_LEN_REG = buffsize;//-(play_pos-((play_pos/buffsize)*buffsize));
		*AudioInfo.AI_LEN_REG = buffsize-(play_pos-((play_pos/buffsize)*buffsize));
		//if (*AudioInfo.AI_LEN_REG > buffsize)
		//	__asm int 3;
		*AudioInfo.AI_STATUS_REG = 0x40000000; // DMA is still busy...
		if (((play_seg - last_seg)&7) > 3) {
			*AudioInfo.MI_INTR_REG |= MI_INTR_AI;
			AudioInfo.CheckInterrupts();
		}
		laststatus = 0x40000000;
		return;
	}
	if (laststatus == 0x40000000) {
		if (writediff > (int)(buffsize*2)) { // This means we are doing a buffer underrun... damnit!
			*AudioInfo.AI_LEN_REG = 0;
			*AudioInfo.AI_STATUS_REG = 0x00000000; // DMA is still busy...
			if (((play_seg - last_seg)&7) > 2) {
				*AudioInfo.MI_INTR_REG |= MI_INTR_AI;
				AudioInfo.CheckInterrupts();
			}
		}
		return;
	}

}

// Fills up a buffer and remixes the audio
void AudioCode::FillBuffer (BYTE *buff, DWORD len) {
	DWORD write_seg = 0;
	DWORD last_seg = 0;
	buffsize = len; // Save it globally

	static FILE *dtest = fopen ("c:\\debug.txt", "wt");

	fprintf (dtest, "%08X - %02X\n", (DWORD)buff - (DWORD)AudioInfo.RDRAM, len);

	if FAILED(lpdsbuf->GetCurrentPosition((unsigned long*)&play_pos, NULL)) {
		//MessageBox (NULL, "Error getting audio position...", "AudioLLE Error", MB_OK|MB_ICONSTOP);
		return;
	}
	if (play_pos < len) write_pos = (len * SEGMENTS) - len;
	else write_pos = ((play_pos / len) * len) - len;

	if (last_write == -1) {
		last_write = (write_pos - (2 * len)); // Back up 2 segments...
		if (last_write < 0)
			last_write += (SEGMENTS * len);
	}

	if (last_write == write_pos) { // Then we must freeze...
		//return; // Ignore it
		do {
			if FAILED(lpdsbuf->GetCurrentPosition((unsigned long*)&play_pos, NULL)) {
				//MessageBox (NULL, "Error getting audio position...", "AudioLLE Error", MB_OK|MB_ICONSTOP);
				return;
			}
			if (play_pos < len) write_pos = (len * SEGMENTS) - len;
			else write_pos = ((play_pos / len) * len) - len;
		} while (write_pos == last_write);
	}

	last_seg = (last_write / len);
	write_seg = (write_pos / len);


	int opt = 0;

	if (last_seg == ((write_seg-2)&0x7)) { // Means first buffer is clear to write to...
		write_pos = (last_write+len);
		if (write_pos >= len*SEGMENTS)
			write_pos -= (len*SEGMENTS);
		// Set DMA Busy
		last_write += len;
		if (last_write >= len*SEGMENTS)
			last_write -= (len*SEGMENTS);
		opt = 1;
		*AudioInfo.AI_STATUS_REG |= 0x40000000;
		laststatus = 0x40000000;
	} else if (last_seg == ((write_seg-1)&0x7)) {
		// Set DMA Busy
		// Set FIFO Buffer Full
		last_write = write_pos; // Lets get it back up to speed for audio accuracy...
		opt = 2;
		*AudioInfo.AI_STATUS_REG |= 0xC0000000;
		laststatus = 0xC0000000;
	} else { // We get here if our audio stream from the game is running TOO slow...
		last_write = write_pos; // Lets get it back up to speed for audio accuracy...
		opt = 3;
		*AudioInfo.AI_STATUS_REG = 0x00000000;
		laststatus = 0x00000000;
	}


	if (last_write == (len*8))
		__asm int 3;

/*
	do {
		if FAILED(lpdsbuf->GetCurrentPosition((unsigned long*)&play_pos, NULL)) {
			MessageBox (NULL, "Error getting audio position...", "AudioLLE Error", MB_OK|MB_ICONSTOP);
			return;
		}
		if (play_pos < len) write_pos = (len * SEGMENTS) - len;
		else write_pos = ((play_pos / len) * len) - len;
	} while (write_pos == last_pos);

	last_pos = write_pos;
*/
	if (DS_OK != lpdsbuf->Lock(write_pos, len, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0)) {
		char buff[100];
		sprintf (buff, "Error Locking Buffer: %i - %i - %i", last_write, write_pos, play_pos);
		//MessageBox (NULL, buff, "Audio LLE", MB_OK|MB_ICONSTOP);
		return;
	}

	memcpy (lpvPtr1, buff, dwBytes1);
	memcpy (lpvPtr2, buff+dwBytes1, dwBytes2);

	if FAILED(lpdsbuf->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2)) {
		//MessageBox(NULL, "Error unlocking sound buffer","Audio LLE", MB_OK|MB_ICONSTOP);
		return;
	}

//		*AudioInfo.AI_STATUS_REG = 0;
//		*AudioInfo.MI_INTR_REG |= MI_INTR_AI;
//		AudioInfo.CheckInterrupts();

}


DWORD WINAPI AudioThreadProc (AudioCode *ac) {
	DWORD dwStatus;
	//LPDIRECTSOUNDBUFFER  lpdsbuf = ac->lpdsbuf;
	LPDIRECTSOUND        lpds  = ac->lpds;

	IDirectSoundBuffer_GetStatus(lpdsbuff,&dwStatus);
		if ((dwStatus & DSBSTATUS_PLAYING) == 0) {
			IDirectSoundBuffer_Play(lpdsbuff, 0, 0, 0 );
	}

	//SetThreadPriority (handleAudioThread, THREAD_PRIORITY_HIGHEST);

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
				UpdateStatus ();
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
		UpdateStatus ();
		Sleep (10);
		UpdateStatus ();
	}

_exit_:
	ReleaseMutex(ac->hMutex);
	ac->handleAudioThread = NULL;
	ExitThread (0);
	return 0;
}





//------------------------------------------------------------------------

// Setup and Teardown Functions

BOOL AudioCode::Initialize (void (*AiCallBack)( DWORD ), HWND hwnd) {
	audioIsPlaying = FALSE;

	LPDIRECTSOUNDBUFFER lpdsb;
    DSBUFFERDESC        dsPrimaryBuff;
    WAVEFORMATEX        wfm;
    HRESULT             hr;

	CallBack = AiCallBack;

	DeInitialize (); // Release just in case...

    if ( FAILED( hr = DirectSoundCreate( NULL, &lpds, NULL ) ) ) {
        return FALSE;
	}

    if ( FAILED( hr = IDirectSound_SetCooperativeLevel(lpds, hwnd, DSSCL_PRIORITY   ))) {
        return FALSE;
	}
	hMutex = CreateMutex(NULL,FALSE,NULL);

	if (lpdsbuf) { 		
		IDirectSoundBuffer_Release(lpdsbuf); 
		lpdsbuf = NULL;
	}
	memset( &dsPrimaryBuff, 0, sizeof( DSBUFFERDESC ) ); 
    
	dsPrimaryBuff.dwSize        = sizeof( DSBUFFERDESC ); 
    dsPrimaryBuff.dwFlags       = DSBCAPS_PRIMARYBUFFER; 
    dsPrimaryBuff.dwBufferBytes = 0;  
    dsPrimaryBuff.lpwfxFormat   = NULL; 
    memset( &wfm, 0, sizeof( WAVEFORMATEX ) ); 

	wfm.wFormatTag = WAVE_FORMAT_PCM;
	wfm.nChannels = 2;
	wfm.nSamplesPerSec = 44100;
	wfm.wBitsPerSample = 16;
	wfm.nBlockAlign = wfm.wBitsPerSample / 8 * wfm.nChannels;
	wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;

	hr = IDirectSound_CreateSoundBuffer(lpds,&dsPrimaryBuff, &lpdsb, NULL);
	
	if (SUCCEEDED ( hr ) ) {
		IDirectSoundBuffer_SetFormat(lpdsb, &wfm );
	    IDirectSoundBuffer_Play(lpdsb, 0, 0, DSBPLAY_LOOPING );
	}

	return TRUE;
}

void AudioCode::DeInitialize () {
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

// Generates nice alignment with N64 samples...
void AudioCode::SetSegmentSize (DWORD length) {

    DSBUFFERDESC        dsbdesc;
    WAVEFORMATEX        wfm;
    HRESULT             hr;

    if (SampleRate == 0) { return; }
	SegmentSize = length;

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

	IDirectSoundBuffer_Play(lpdsbuf, 0, 0, DSBPLAY_LOOPING );
}

// Buffer Functions for the Audio Code
void AudioCode::SetFrequency (DWORD Frequency) {
	SampleRate = Frequency;
	SegmentSize = 0; // Trash it... we need to redo the Frequency anyway...
}

DWORD AudioCode::AddBuffer (BYTE *start, DWORD length) {
	DWORD retVal = 0;

	if (length == 0)
		return 0;
	if (length == 0x8C0) { // TODO:  This proves I need more buffering!!!
		length = 0x840;
	}
	if (length == 0x880) { // TODO:  This proves I need more buffering!!!
		length = 0x840;
	}
	if (length != SegmentSize) {
		SetSegmentSize (length);
	}

	if (!audioIsPlaying)
		StartAudio ();
	if (readLoc == writeLoc) // Reset our pointer if we can...
		writeLoc = readLoc = 0;

	if (readLoc != writeLoc) // Then we have stuff in the buffer already...  This is a double buffer
		retVal |= SND_IS_FULL;
	
	retVal |= SND_IS_NOT_EMPTY;  // Buffer is not empty...

	memcpy ((SoundBuffer+writeLoc), start, length); // Buffer this audio data...
	writeLoc += length;

	// Temporary
	WaitForSingleObject (hMutex, INFINITE);
	lpdsbuf = this->lpdsbuf;
	FillBuffer (start, length);
	readLoc = writeLoc = 0;
	//CallBack (0x04000000);
	UpdateStatus ();
	ReleaseMutex(hMutex);

	return retVal;
}

// Management functions
// TODO: For silent emulation... the Audio should still be "processed" somehow...
void AudioCode::StopAudio () {
	if (!audioIsPlaying) return;
	audioIsPlaying = FALSE;
}

void AudioCode::StartAudio () {
	if (audioIsPlaying) return;
	audioIsPlaying = TRUE;
}

DWORD AudioCode::GetReadStatus () {
	// I think updating the status on the Flags would be good here as well...
	lpdsbuff = this->lpdsbuf;
	UpdateStatus ();
	return *AudioInfo.AI_LEN_REG;
	//return (writeLoc - readLoc);
}
