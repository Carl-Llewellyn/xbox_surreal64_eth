#include "RomList.h"
#include "../config.h"
#include "zlib/zlib.h"
#include "zlib/unzip.h"
#include "musicmanager.h"

#ifndef DEBUG
#define USE_OBF
#endif
extern CMusicManager music;
//extern int romcounter; // use romlist func instead
extern int actualrom;
#define BUFFER_SIZE 0x400000 // -> (4096 / 4KB pagesize) = 1MB
extern byte GetByteSwapType(byte *header);
extern bool ByteSwap(dword size, byte *image, byte bswapType);
extern int dw1964DynaMem;
extern int dw1964PagingMem;
extern int dwPJ64DynaMem;
extern int dwPJ64PagingMem;
extern bool bUseLLERSP; // use iAudioPlugin instead, but leave this in case it's set in ini

// ultrahle mem settings
extern int dwUltraCodeMem;
extern int dwUltraGroupMem;

//extern int m_emulator; // Ez0n3 - why was this used at all?
extern int preferedemu;

extern void CreateProgress();
extern void RenderProgress(int progress);
extern int ConfigAppSaveTemp();
extern int videoplugin;
//extern bool onhd;

// Ez0n3 - use iAudioPlugin instead to determine which audio plugin is used
extern int iAudioPlugin;
//extern bool usePageOriginal;
extern int iPagingMethod;
// Ez0n3 - reinstate max video mem
extern int dwMaxVideoMem;
extern int iRspPlugin;
extern bool bUseRspAudio; // control a listing


DWORD WINAPI PrepareRomThread(Rom *rom)
{

	//Ez0n3 - skip the loading screen if the temporary rom is the same as the rom that is loading (its the exact same rom file).
	char currom[256];
	sprintf(currom, "%08X-%08X-C:%02X", rom->GetCrc1(), rom->GetCrc2(), rom->GetCountry());
	
	ifstream romTmpFile;
	romTmpFile.open("Z:\\TemporaryRom.dat", ios_base::in | ios_base::binary);
	//romTmpFile.open("T:\\Data\\TemporaryRom.dat", ios_base::in | ios_base::binary);
	
	#ifdef USE_OBF
	// create a temporary file just in case something goes wrong
	HANDLE hFile = CreateFile( "T:\\Temp\\codetemp.dat", GENERIC_WRITE, FILE_SHARE_READ, NULL, 
                               OPEN_ALWAYS, 0, NULL );
	CloseHandle(hFile);
	#endif

	if (romTmpFile.is_open()) {
		byte header[0x40];
		romTmpFile.read(reinterpret_cast<char *>(header), 0x40);
		romTmpFile.close();
		
		byte byByteSwapType = GetByteSwapType(header);
		if (byByteSwapType != -1) {
			char tmprom[256];
			dword dwCrc1;
			dword dwCrc2;
			byte byCountry;
		
			ByteSwap(0x40, header, byByteSwapType);
			dwCrc1 = *(reinterpret_cast<dword *>(header + 0x10));
			dwCrc2 = *(reinterpret_cast<dword *>(header + 0x14));
			byCountry = *(reinterpret_cast<byte *>(header + 0x3D));
			sprintf(tmprom, "%08X-%08X-C:%02X", dwCrc1, dwCrc2, byCountry);
			
			if (strcmp(currom, tmprom) == 0) {
			
				// show the boxart with 100% progress for persist display
				CreateProgress();
				RenderProgress(100);

				return 0;
			}
		}
	}
	

	CreateProgress();
	int progress=0;
	RenderProgress(progress);
	// the rom to prepare
	string szFileName = rom->GetFileName();
	string romName = rom->GetProperName();
	// allocate 1 meg of memory for temporary rom loading
	byte *pBuffer = static_cast<byte *>(VirtualAlloc(NULL, BUFFER_SIZE, MEM_COMMIT, PAGE_READWRITE));
	

	// open the destination rom
	ofstream destRom;
	destRom.open("Z:\\TemporaryRom.dat", ios_base::out | ios_base::binary);
	//destRom.open("T:\\Data\\TemporaryRom.dat", ios_base::out | ios_base::binary);

	if (!destRom.is_open())
	{
		goto _FAIL;
	}

	// check to see if the source file is a zip file. if it is, use the zlib to extract the rom
	if (szFileName.substr(szFileName.length() - 4) == ".zip")
	{
		int retVal;
		unzFile	fp;
		fp = unzOpen(szFileName.c_str());

		retVal = unzGoToFirstFile(fp);

		if (retVal != UNZ_OK)
		{
			goto _FAIL;
		}

		do
		{
			unz_file_info fileInfo;
			char szUnzFileName[_MAX_FNAME];
			
			retVal = unzGetCurrentFileInfo(fp, &fileInfo, szUnzFileName, _MAX_FNAME, NULL, 0, NULL, 0);

			if (retVal != UNZ_OK)
			{
				goto _FAIL;
			}

			string szFn(szUnzFileName);

			// find a rom file in the zip file
			if (szFn.substr(szFn.length() - 4) == ".bin"
			||	szFn.substr(szFn.length() - 4) == ".v64"
			||	szFn.substr(szFn.length() - 4) == ".rom"
			||	szFn.substr(szFn.length() - 4) == ".usa"
			||	szFn.substr(szFn.length() - 4) == ".z64"
			||	szFn.substr(szFn.length() - 4) == ".j64"
			||	szFn.substr(szFn.length() - 4) == ".pal"
			||	szFn.substr(szFn.length() - 4) == ".n64")
			{
				retVal = unzOpenCurrentFile(fp);

				if (retVal != UNZ_OK)
				{
					unzClose(fp);

					goto _FAIL;
				}

				// read the first meg
				unzReadCurrentFile(fp, pBuffer, BUFFER_SIZE);

				// get the byte order of the rom
				byte byBswapType = GetByteSwapType(pBuffer);

				// byte swap the first meg. at this stage, we can check whether it is a n64 rom
				if (!ByteSwap(BUFFER_SIZE, pBuffer, byBswapType))
				{
					// not a rom file
					unzCloseCurrentFile(fp);
					unzClose(fp);
					
					goto _FAIL;
				}

				// write the pBuffer to the destination rom
				destRom.write(reinterpret_cast<char *>(pBuffer), BUFFER_SIZE);

				for(qword i = BUFFER_SIZE; i < fileInfo.uncompressed_size; i += BUFFER_SIZE)
				{
					progress =  static_cast<dword>((i) * 100 / fileInfo.uncompressed_size) ;
					RenderProgress(progress);

					// read a meg
					unzReadCurrentFile(fp, pBuffer, BUFFER_SIZE);

					// byte swap the meg
					ByteSwap(BUFFER_SIZE, pBuffer, byBswapType);

					// write the pBuffer to the destination rom
					destRom.write(reinterpret_cast<char *>(pBuffer), BUFFER_SIZE);
				}

				progress =  static_cast<dword> ((i) * 100 / fileInfo.uncompressed_size) ;
                RenderProgress(progress);

				unzCloseCurrentFile(fp);
				unzClose(fp);

				goto _SUCCESS;
			}
		} while (unzGoToNextFile(fp) == UNZ_OK);

		unzClose(fp);
	}
	else
	{
		ifstream fp;

		// open the rom
		fp.open(szFileName.c_str(), ios_base::in | ios_base::binary);

		if (!fp.is_open())
		{
			goto _FAIL;
		}

		// get the size of the rom
		fp.seekg(0, ios_base::end);
		qword qwFileSize = fp.tellg();
		fp.seekg(0, ios_base::beg);

		// read the first meg
		fp.read(reinterpret_cast<char *>(pBuffer), BUFFER_SIZE);

		// get the byte order of the rom
		byte byBswapType = GetByteSwapType(pBuffer);

		// byte swap the first meg. at this stage, we can check whether it is a n64 rom
		if (!ByteSwap(BUFFER_SIZE, pBuffer, byBswapType))
		{
			// not a rom file
			fp.close();

			goto _FAIL;
		}

		destRom.write(reinterpret_cast<char *>(pBuffer), BUFFER_SIZE);

		for(qword i = BUFFER_SIZE; i < qwFileSize; i += BUFFER_SIZE)
		{
			progress =  static_cast<dword>((i) * 100 / qwFileSize) ;
            RenderProgress(progress);

			// seek to the next meg
			fp.seekg(static_cast<dword>(i), ios_base::beg);

			// read a meg
			fp.read(reinterpret_cast<char *>(pBuffer), BUFFER_SIZE);

			// byte swap the meg
			ByteSwap(BUFFER_SIZE, pBuffer, byBswapType);

			// write the pBuffer to the destination rom
			destRom.write(reinterpret_cast<char *>(pBuffer), BUFFER_SIZE);
		}

		progress =  static_cast<dword>((i) * 100 / qwFileSize) ;
        RenderProgress(progress);

		fp.close();

		goto _SUCCESS;
	}

_SUCCESS:
	goto _END;

_FAIL:
	goto _END;

_END:
	destRom.close();
	VirtualFree(pBuffer, BUFFER_SIZE, MEM_DECOMMIT);

	return 0;
}


void Launch()
{
	Rom *m_pRom = g_romList.GetRomAt(actualrom);
	music.Stop();
	PrepareRomThread(m_pRom);

	ConfigAppSaveTemp();

	char szLaunchXBE[128];
	
	if (preferedemu == _UltraHLE)
	{
		sprintf(szLaunchXBE, "D:\\UltraHLE.xbe");
	}
	else
	{
		char szEmulator[16];
		char szAudioPlugin[16];
		char szVideoPlugin[16];
		//char szPagingMethod[16];
		
		switch (preferedemu) {
			case _1964: 		sprintf(szEmulator, "1964"); break;
			case _Project64: 	sprintf(szEmulator, "PJ64"); break;
			//case _UltraHLE: 	sprintf(szEmulator, "UltraHLE"); break;
			default:
				sprintf(szEmulator, "1964"); break;
		}
		
		switch (videoplugin) {
			case _VideoPluginRice510: 	sprintf(szVideoPlugin, "-510"); break;
			case _VideoPluginRice531: 	sprintf(szVideoPlugin, "-531"); break;
			case _VideoPluginRice560: 	sprintf(szVideoPlugin, "-560"); break;
			case _VideoPluginRice611: 	sprintf(szVideoPlugin, "-611"); break;
			case _VideoPluginRice612: 	sprintf(szVideoPlugin, "-612"); break;
			default:
				sprintf(szVideoPlugin, "-560"); break;
		}
		
		switch (iAudioPlugin) {
			//case _AudioPluginNone: 	sprintf(szAudioPlugin, ""); break;
			//case _AudioPluginBasic: 	sprintf(szAudioPlugin, ""); break;
			//case _AudioPluginJttl: 	sprintf(szAudioPlugin, ""); break;
			//case _AudioPluginAzimer: 	sprintf(szAudioPlugin, ""); break;
			case _AudioPluginMusyX: 	sprintf(szAudioPlugin, "M"); break;
			default:
				sprintf(szAudioPlugin, ""); break;
		}

		sprintf(szLaunchXBE, "D:\\%s%s%s.xbe", szEmulator, szVideoPlugin, szAudioPlugin);
	}

	OutputDebugString("Launching: ");
	OutputDebugString(szLaunchXBE);
	OutputDebugString("\n");
	//Sleep(100); // to see debug string

	D3DDevice::PersistDisplay();
	
	XLaunchNewImage(szLaunchXBE, NULL);
}
