//#include <windows.h>
#include <xtl.h>
#include <dsound.h>
#include <stdio.h>
#include "audio.h"

#define AI_STATUS_FIFO_FULL	0x80000000		/* Bit 31: full */
#define AI_STATUS_DMA_BUSY	0x40000000		/* Bit 30: busy */
#define MI_INTR_AI			0x04			/* Bit 2: AI intr */
#define NUMCAPTUREEVENTS	3
#define BufferSize			0x5000

#define Buffer_Empty		0
#define Buffer_Playing		1
#define Buffer_HalfFull		2
#define Buffer_Full			3

void B_FillBuffer            ( int buffer );
BOOL B_FillBufferWithSilence ( LPDIRECTSOUNDBUFFER lpDsb );
void B_FillSectionWithSilence( int buffer );
void B_SetupDSoundBuffers    ( void );
void B_Soundmemcpy           ( void * dest, const void * src, size_t count );
void B_DisplayError (char * Message, ...);

DWORD Frequency, B_Dacrate = 0, Snd1Len, SpaceLeft, SndBuffer[3], Playing;
AUDIO_INFO AudioInfo;
BYTE *Snd1ReadPos;

LPDIRECTSOUNDBUFFER  lpdsbuf;
LPDIRECTSOUND        lpds;
LPDIRECTSOUNDNOTIFY  lpdsNotify;
HANDLE               rghEvent[NUMCAPTUREEVENTS];
DSBPOSITIONNOTIFY    rgdscbpn[NUMCAPTUREEVENTS];


void _AUDIO_BASIC_AiDacrateChanged (int SystemType) {
	if (B_Dacrate != *AudioInfo.AI_DACRATE_REG) {
		B_Dacrate = *AudioInfo.AI_DACRATE_REG;
		switch (SystemType) {
		case SYSTEM_NTSC: Frequency = 48681812 / (B_Dacrate + 1); break;
		case SYSTEM_PAL:  Frequency = 49656530 / (B_Dacrate + 1); break;
		case SYSTEM_MPAL: Frequency = 48628316 / (B_Dacrate + 1); break;
		}
		B_SetupDSoundBuffers();
	}
}

void _AUDIO_BASIC_AiLenChanged (void) {
	int count, offset, temp;
	DWORD dwStatus;

	if (*AudioInfo.AI_LEN_REG == 0) { return; }
	*AudioInfo.AI_STATUS_REG |= AI_STATUS_FIFO_FULL;
	Snd1Len = (*AudioInfo.AI_LEN_REG & 0x3FFF8);
	temp = Snd1Len;
	Snd1ReadPos = AudioInfo.RDRAM + (*AudioInfo.AI_DRAM_ADDR_REG & 0x00FFFFF8);
	if (Playing) {
		for (count = 0; count < 3; count ++) {
			if (SndBuffer[count] == Buffer_Playing) {
				offset = (count + 1) & 3;
			}
		}
	} else {
		offset = 0;
	}

	for (count = 0; count < 3; count ++) {
		if (SndBuffer[(count + offset) & 3] == Buffer_HalfFull) {
			B_FillBuffer((count + offset) & 3);
			count = 3;
		}
	}
	for (count = 0; count < 3; count ++) {
		if (SndBuffer[(count + offset) & 3] == Buffer_Full) {
			B_FillBuffer((count + offset + 1) & 3);
			B_FillBuffer((count + offset + 2) & 3);
			count = 20;
		}
	}
	if (count < 10) {
		B_FillBuffer((0 + offset) & 3);
		B_FillBuffer((1 + offset) & 3);
		B_FillBuffer((2 + offset) & 3);
	}

	if (!Playing) {
		for (count = 0; count < 3; count ++) {
			if (SndBuffer[count] == Buffer_Full) {
				Playing = TRUE;
				IDirectSoundBuffer_Play(lpdsbuf, 0, 0, 0 );
				return;
			}
		}
	} else {
		IDirectSoundBuffer_GetStatus(lpdsbuf,&dwStatus);
		if ((dwStatus & DSBSTATUS_PLAYING) == 0) {
			IDirectSoundBuffer_Play(lpdsbuf, 0, 0, 0 );
		}
	}
}

DWORD _AUDIO_BASIC_AiReadLength (void) {
	return Snd1Len;
}

void _AUDIO_BASIC_AiUpdate (BOOL Wait) {
	DWORD dwEvt;
	DirectSoundDoWork();
/*
	if (Wait) {
		dwEvt = MsgWaitForMultipleObjects(NUMCAPTUREEVENTS,rghEvent,FALSE,
			INFINITE,QS_ALLINPUT);
	} else {
		dwEvt = MsgWaitForMultipleObjects(NUMCAPTUREEVENTS,rghEvent,FALSE,
			0,QS_ALLINPUT);
	}*/
	dwEvt = WaitForMultipleObjects(NUMCAPTUREEVENTS, rghEvent, FALSE, 0);
	dwEvt -= WAIT_OBJECT_0;
	if (dwEvt == NUMCAPTUREEVENTS) {
		return;
	}

	switch (dwEvt) {
	case 0: 
		SndBuffer[0] = Buffer_Empty;
		B_FillSectionWithSilence(0);
		SndBuffer[1] = Buffer_Playing;
		B_FillBuffer(2);
		B_FillBuffer(0);
		break;
	case 1: 
		SndBuffer[1] = Buffer_Empty;
		B_FillSectionWithSilence(1);
		SndBuffer[2] = Buffer_Playing;
		B_FillBuffer(0);
		B_FillBuffer(1);
		break;
	case 2: 
		SndBuffer[2] = Buffer_Empty;
		B_FillSectionWithSilence(2);
		SndBuffer[0] = Buffer_Playing;
		B_FillBuffer(1);
		B_FillBuffer(2);		
	    IDirectSoundBuffer_Play(lpdsbuf, 0, 0, 0 );
		break;
	}
}

void _AUDIO_BASIC_CloseDLL (void) {
    if (lpdsbuf) { 
		IDirectSoundBuffer_Release(lpdsbuf);
	}
    if ( lpds ) {
		IDirectSound_Release(lpds);
	}
}

void B_DisplayError (char * Message, ...) {
	/*char Msg[400];
	va_list ap;

	va_start( ap, Message );
	vsprintf( Msg, Message, ap );
	va_end( ap );
	MessageBox(NULL,Msg,"Error",MB_OK|MB_ICONERROR);*/
	OutputDebugString(Message);
}

void _AUDIO_BASIC_DllAbout ( HWND hParent ) {
	B_DisplayError ("Basic Audio plugin by zilmar");
}

void B_FillBuffer ( int buffer ) {
    DWORD dwBytesLocked;
    VOID *lpvData;

	if (Snd1Len == 0) { return; }
	if (SndBuffer[buffer] == Buffer_Empty) {
		if (Snd1Len >= BufferSize) {
			if (FAILED( IDirectSoundBuffer_Lock(lpdsbuf, BufferSize * buffer,BufferSize, &lpvData, &dwBytesLocked,
				NULL, NULL, 0  ) ) )
			{
				IDirectSoundBuffer_Unlock(lpdsbuf, lpvData, dwBytesLocked, NULL, 0 );
				B_DisplayError("FAILED lock");
				return;
			}
			B_Soundmemcpy(lpvData,Snd1ReadPos,dwBytesLocked);
			SndBuffer[buffer] = Buffer_Full;
			Snd1ReadPos += dwBytesLocked;
			Snd1Len -= dwBytesLocked;
			IDirectSoundBuffer_Unlock(lpdsbuf, lpvData, dwBytesLocked, NULL, 0 );
		} else {
			if (FAILED( IDirectSoundBuffer_Lock(lpdsbuf, BufferSize * buffer,Snd1Len, &lpvData, &dwBytesLocked,
				NULL, NULL, 0  ) ) )
			{
				IDirectSoundBuffer_Unlock(lpdsbuf, lpvData, dwBytesLocked, NULL, 0 );
				B_DisplayError("FAILED lock");
				return;
			}
			B_Soundmemcpy(lpvData,Snd1ReadPos,dwBytesLocked);
			SndBuffer[buffer] = Buffer_HalfFull;
			Snd1ReadPos += dwBytesLocked;
			SpaceLeft = BufferSize - Snd1Len;
			Snd1Len = 0;
			IDirectSoundBuffer_Unlock(lpdsbuf, lpvData, dwBytesLocked, NULL, 0 );
		}
	} else if (SndBuffer[buffer] == Buffer_HalfFull) {
		if (Snd1Len >= SpaceLeft) {
			if (FAILED( IDirectSoundBuffer_Lock(lpdsbuf, (BufferSize * (buffer + 1)) - SpaceLeft ,SpaceLeft, &lpvData,
				&dwBytesLocked, NULL, NULL, 0  ) ) )
			{
				IDirectSoundBuffer_Unlock(lpdsbuf, lpvData, dwBytesLocked, NULL, 0 );
				B_DisplayError("FAILED lock");
				return;
			}
			B_Soundmemcpy(lpvData,Snd1ReadPos,dwBytesLocked);
			SndBuffer[buffer] = Buffer_Full;
			Snd1ReadPos += dwBytesLocked;
			Snd1Len -= dwBytesLocked;
			IDirectSoundBuffer_Unlock(lpdsbuf, lpvData, dwBytesLocked, NULL, 0 );
		} else {
			if (FAILED( IDirectSoundBuffer_Lock(lpdsbuf, (BufferSize * (buffer + 1)) - SpaceLeft,Snd1Len, &lpvData, &dwBytesLocked,
				NULL, NULL, 0  ) ) )
			{
				IDirectSoundBuffer_Unlock(lpdsbuf, lpvData, dwBytesLocked, NULL, 0 );
				B_DisplayError("FAILED lock");
				return;
			}
			B_Soundmemcpy(lpvData,Snd1ReadPos,dwBytesLocked);
			SndBuffer[buffer] = Buffer_HalfFull;
			Snd1ReadPos += dwBytesLocked;
			SpaceLeft = SpaceLeft - Snd1Len;
			Snd1Len = 0;
			IDirectSoundBuffer_Unlock(lpdsbuf, lpvData, dwBytesLocked, NULL, 0 );
		}
	}
	if (Snd1Len == 0) {
		*AudioInfo.AI_STATUS_REG &= ~AI_STATUS_FIFO_FULL;
		*AudioInfo.MI_INTR_REG |= MI_INTR_AI;
		AudioInfo.CheckInterrupts();
	}
}

BOOL B_FillBufferWithSilence( LPDIRECTSOUNDBUFFER lpDsb ) {
    WAVEFORMATEX    wfx;
    DWORD           dwSizeWritten;

    PBYTE   pb1;
    DWORD   cb1;
/*
    if ( FAILED( IDirectSoundBuffer_GetFormat(lpDsb, &wfx, sizeof( WAVEFORMATEX ), &dwSizeWritten ) ) ) {
        return FALSE;
	}
*/

	//freakdave - IDirectSoundBuffer_GetFormat wrapping
	memset( &wfx, &dwSizeWritten, sizeof(WAVEFORMATEX));
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = 2;
	wfx.nSamplesPerSec = Frequency;
	wfx.wBitsPerSample = 16;
	wfx.nBlockAlign = wfx.wBitsPerSample / 8 * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;


    if ( SUCCEEDED( IDirectSoundBuffer_Lock(lpDsb,0,0,(LPVOID*)&pb1,&cb1,NULL,NULL,DSBLOCK_ENTIREBUFFER))) {
        FillMemory( pb1, cb1, ( wfx.wBitsPerSample == 8 ) ? 128 : 0 );

        IDirectSoundBuffer_Unlock(lpDsb, pb1, cb1, NULL, 0 );
        return TRUE;
    }

    return FALSE;
}

void B_FillSectionWithSilence( int buffer ) {
    DWORD dwBytesLocked;
    VOID *lpvData;

	if (FAILED( IDirectSoundBuffer_Lock(lpdsbuf, BufferSize * buffer,BufferSize, &lpvData, &dwBytesLocked,
		NULL, NULL, 0  ) ) )
	{
		IDirectSoundBuffer_Unlock(lpdsbuf, lpvData, dwBytesLocked, NULL, 0 );
		B_DisplayError("FAILED lock");
		return;
	}
    FillMemory( lpvData, dwBytesLocked, 0 );
	IDirectSoundBuffer_Unlock(lpdsbuf, lpvData, dwBytesLocked, NULL, 0 );
}

void _AUDIO_BASIC_GetDllInfo ( PLUGIN_INFO * PluginInfo ){ 
	PluginInfo->Version = 0x0101;
	PluginInfo->Type    = PLUGIN_TYPE_AUDIO;
	sprintf(PluginInfo->Name,"Basic Audio Plugin");
	PluginInfo->NormalMemory  = TRUE;
	PluginInfo->MemoryBswaped = TRUE;
}

BOOL _AUDIO_BASIC_InitiateAudio (AUDIO_INFO Audio_Info) {
	HRESULT hr;
	int count;

	AudioInfo = Audio_Info;
 
    if ( FAILED( hr = DirectSoundCreate( NULL, &lpds, NULL ) ) ) {
        return FALSE;
	}

    if ( FAILED( hr = IDirectSound_SetCooperativeLevel(lpds, AudioInfo.hwnd, DSSCL_PRIORITY   ))) {
        return FALSE;
	}
    for ( count = 0; count < NUMCAPTUREEVENTS; count++ ) {
        rghEvent[count] = CreateEvent( NULL, FALSE, FALSE, NULL );
        if (rghEvent[count] == NULL ) { return FALSE; }
    }
	B_Dacrate = 0;
	Playing = FALSE;	
	SndBuffer[0] = Buffer_Empty;
	SndBuffer[1] = Buffer_Empty;
	SndBuffer[2] = Buffer_Empty;
	return TRUE;
}

void _AUDIO_BASIC_ProcessAList(void) {
}

void _AUDIO_BASIC_RomClosed (void) {
}

void B_SetupDSoundBuffers(void) {
	//LPDIRECTSOUNDBUFFER lpdsb;
    DSBUFFERDESC        dsPrimaryBuff;//, dsbdesc;
    WAVEFORMATEX        wfm;
    HRESULT             hr;

    if (lpdsbuf) { IDirectSoundBuffer_Release(lpdsbuf); }
	/*
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

	wfm.nSamplesPerSec = Frequency;
	wfm.wBitsPerSample = 16;
	wfm.nBlockAlign = wfm.wBitsPerSample / 8 * wfm.nChannels;
	wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;

    memset( &dsbdesc, 0, sizeof( DSBUFFERDESC ) ); 
    dsbdesc.dwSize = sizeof( DSBUFFERDESC ); 
    dsbdesc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY;
    dsbdesc.dwBufferBytes = BufferSize * 3;  
    dsbdesc.lpwfxFormat = &wfm; 

	if ( FAILED( hr = IDirectSound_CreateSoundBuffer(lpds, &dsbdesc, &lpdsbuf, NULL ) ) ) {
		DisplayError("Failed in creation of Play buffer 1");	
	}*/


	//freakdave - Set up Wave format structure
    memset( &wfm, 0, sizeof( WAVEFORMATEX ) );
	wfm.wFormatTag = WAVE_FORMAT_PCM;
	wfm.nChannels = 2;
	wfm.nSamplesPerSec = Frequency;
	wfm.wBitsPerSample = 16;
	wfm.nBlockAlign = wfm.wBitsPerSample / 8 * wfm.nChannels;
	wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;

	//freakdave - Set up DSBUFFERDESC structure
	memset( &dsPrimaryBuff, 0, sizeof( DSBUFFERDESC ) ); 
	dsPrimaryBuff.dwSize        = sizeof( DSBUFFERDESC ); 
    //dsPrimaryBuff.dwFlags       = DSBCAPS_PRIMARYBUFFER; //freakdave - There is no primary sound buffer on XBOX
	dsPrimaryBuff.dwFlags		= DSBCAPS_CTRLPOSITIONNOTIFY;
    dsPrimaryBuff.dwBufferBytes = BufferSize * 3;  
    dsPrimaryBuff.lpwfxFormat   = &wfm; 

	hr = IDirectSound8_CreateSoundBuffer(lpds, &dsPrimaryBuff, &lpdsbuf, NULL);

	if (SUCCEEDED ( hr ) ) {
		IDirectSoundBuffer_SetFormat(lpdsbuf, &wfm );
		IDirectSoundBuffer_Play(lpdsbuf, 0, 0, DSBPLAY_LOOPING );
	}
	else
	{
		OutputDebugString("Failed to create Play buffer\n");
	}


	B_FillBufferWithSilence( lpdsbuf );

    rgdscbpn[0].dwOffset = ( BufferSize ) - 1;
    rgdscbpn[0].hEventNotify = rghEvent[0];
    rgdscbpn[1].dwOffset = ( BufferSize * 2) - 1;
    rgdscbpn[1].hEventNotify = rghEvent[1];
    rgdscbpn[2].dwOffset = ( BufferSize * 3) - 1;
    rgdscbpn[2].hEventNotify = rghEvent[2];
    rgdscbpn[3].dwOffset = DSBPN_OFFSETSTOP;
    rgdscbpn[3].hEventNotify = rghEvent[3];
/*
    if ( FAILED( hr = IDirectSound_QueryInterface(lpdsbuf, &IID_IDirectSoundNotify, ( VOID ** )&lpdsNotify ) ) ) {
		DisplayError("IDirectSound_QueryInterface: Failed\n");
		return;
	}

    // Set capture buffer notifications.
    if ( FAILED( hr = IDirectSoundNotify_SetNotificationPositions(lpdsNotify, NUMCAPTUREEVENTS, rgdscbpn ) ) ) {
		DisplayError("IDirectSoundNotify_SetNotificationPositions: Failed");
		return;
    }*/

	if ( FAILED( hr = IDirectSoundBuffer_SetNotificationPositions(lpdsbuf, NUMCAPTUREEVENTS, rgdscbpn ) ) ) {
		OutputDebugString("IDirectSoundBuffer_SetNotificationPositions: Failed\n");
		return FALSE;
	}
}


void _AUDIO_BASIC_DllConfig ( HWND hParent )
{
}

void _AUDIO_BASIC_DllTest ( HWND hParent )
{
}


void B_Soundmemcpy(void * dest, const void * src, size_t count) {
	if (AudioInfo.MemoryBswaped) {
		_asm {
			mov edi, dest
			mov ecx, src
			mov edx, 0		
		memcpyloop1:
			mov ax, word ptr [ecx + edx]
			mov bx, word ptr [ecx + edx + 2]
			mov  word ptr [edi + edx + 2],ax
			mov  word ptr [edi + edx],bx
			add edx, 4
			mov ax, word ptr [ecx + edx]
			mov bx, word ptr [ecx + edx + 2]
			mov  word ptr [edi + edx + 2],ax
			mov  word ptr [edi + edx],bx
			add edx, 4
			cmp edx, count
			jb memcpyloop1
		}
	} else {
		_asm {
			mov edi, dest
			mov ecx, src
			mov edx, 0		
		memcpyloop2:
			mov ax, word ptr [ecx + edx]
			xchg ah,al
			mov  word ptr [edi + edx],ax
			add edx, 2
			mov ax, word ptr [ecx + edx]
			xchg ah,al
			mov  word ptr [edi + edx],ax
			add edx, 2
			mov ax, word ptr [ecx + edx]
			xchg ah,al
			mov  word ptr [edi + edx],ax
			add edx, 2
			mov ax, word ptr [ecx + edx]
			xchg ah,al
			mov  word ptr [edi + edx],ax
			add edx, 2
			cmp edx, count
			jb memcpyloop2
		}
	}
}
