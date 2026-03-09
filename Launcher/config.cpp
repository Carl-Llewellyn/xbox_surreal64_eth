
// lets try relying on ./config.cpp instead
/*
#include "config.h"

//freakdave
int dw1964DynaMem=8;
int dw1964PagingMem=4;
int dwPJ64DynaMem=16;
int dwPJ64PagingMem=4;
bool bUseLLERSP=false; // use iAudioPlugin instead, but leave this in case it's set in ini
int dwMaxVideoMem=4;
bool bUseRspAudio=true; // control a listing

// audio plugins
int iAudioPlugin=_AudioPluginJttl; // 2=JttL
// video plugins
int videoplugin=_VideoPluginRice560; // 2=Rice560
// rsp plugins
int iRspPlugin=_RSPPluginNone;

int FlickerFilter = 1;
bool SoftDisplayFilter = 0;
int TextureMode = 3;
int VertexMode = 1;
float XBINPUT_DEADZONE = 0.24;
float XBINPUT_SUMMAND = 0.5;
float XBOX_CONTROLLER_DEAD_ZONE = 8600;
float Deadzone = 0.26;
int Sensitivity = 0;

enum Pak
{
MemPak,
RumblePak,
};

int DefaultPak = RumblePak;

bool EnableController1 = true;
bool EnableController2 = true;
bool EnableController3 = false;
bool EnableController4 = false;

#include "RomList.h"
extern int actualrom;
extern int preferedemu;



// FIXME
// Ez0n3 - these ConfigAppLoad and ConfigAppSave functions don't seem to be used
// the file "d:\\ini\\surreal-ce.ini" never gets written out and therefore never read
// all current calls to ConfigAppLoad and ConfigAppSave seem to be calling .\config.cpp
// IE: the only file on the hd is: "D:\\ini\\surreal-xxx.ini" from .\config.cpp
// are two separate functions with the same name needed anymore?

// Read in the config file for the whole application
int ConfigAppLoad()
{
	char szLine[256];
	FILE *h;


	if ((h = fopen("d:\\ini\\surreal-ce.ini", "rt")) == NULL) {
		return 1;
	}

	// Go through each line of the config file
	while (1) {
		int nLen;
		if (fgets(szLine, sizeof(szLine), h) == NULL) {				// End of config file
			break;
		}

		nLen = strlen(szLine);
		
		// Get rid of the linefeed at the end
		if (szLine[nLen - 1] == 10) {
			szLine[nLen - 1] = 0;
			nLen--;
		}

#define BOL(x) { char *szValue = LabelCheck(szLine,#x);		\
	if (szValue) x = strtol(szValue, NULL, 0) != 0; }
#define VAR(x) { char *szValue = LabelCheck(szLine,#x);		\
	if (szValue) x = strtol(szValue, NULL, 0); }
#define FLT(x) { char *szValue = LabelCheck(szLine,#x);		\
	if (szValue) x = atof(szValue); }
#define STR(x) { char *szValue = LabelCheck(szLine,#x " ");	\
	if (szValue) strcpy(x,szValue); }

        VAR(VertexMode);
		VAR(FlickerFilter);
		BOL(SoftDisplayFilter);
		VAR(TextureMode);
		FLT(XBINPUT_DEADZONE);
		FLT(XBINPUT_SUMMAND);
		FLT(XBOX_CONTROLLER_DEAD_ZONE);
		FLT(Deadzone);
		VAR(Sensitivity);
		VAR(DefaultPak);
		BOL(EnableController1);
		BOL(EnableController2);
		BOL(EnableController3);
		BOL(EnableController4);

		VAR(dw1964DynaMem);
		VAR(dw1964PagingMem);
		VAR(dwPJ64DynaMem);
		VAR(dwPJ64PagingMem);
		BOL(bUseLLERSP); // Ez0n3 - use iAudioPlugin instead, but leave this in case it's set in ini
		VAR(dwMaxVideoMem);
		
		// Ez0n3 - audio plugins
		VAR(iAudioPlugin);
		VAR(iRspPlugin);
		BOL(bUseRspAudio);


#undef STR
#undef FLT
#undef VAR
#undef BOL
	}

	fclose(h);
	return 0;
}

// Write out the config file for the whole application

int ConfigAppSave()
{

	//
	FILE *h;

	if ((h = fopen("d:\\ini\\surreal-ce.ini", "wt")) == NULL) return 1;

	// Write title
	fprintf(h,"// Surreal64 CE Config File\n\n");
	fprintf(h,"// Don't edit this file manually unless you know what you're doing\n");
	fprintf(h,"// Surreal will restore default settings when this file is deleted\n");

#define BOL(x) fprintf(h, #x " %d\n", (int)(x != 0))
#define VAR(x) fprintf(h, #x " %d\n", x)
#define FLT(x) fprintf(h, #x " %f\n", x)
#define STR(x) fprintf(h, #x " %s\n", x)
	
	fprintf(h,"\n\n\n");
    
	VAR(VertexMode);
	VAR(FlickerFilter);
	BOL(SoftDisplayFilter);
	VAR(TextureMode);
	FLT(XBINPUT_DEADZONE);
	FLT(XBINPUT_SUMMAND);
	FLT(XBOX_CONTROLLER_DEAD_ZONE);
	FLT(Deadzone);
	VAR(Sensitivity);
	VAR(DefaultPak);
	BOL(EnableController1);
	BOL(EnableController2);
	BOL(EnableController3);
	BOL(EnableController4);

	VAR(dw1964DynaMem);
	VAR(dw1964PagingMem);
	VAR(dwPJ64DynaMem);
	VAR(dwPJ64PagingMem);
	BOL(bUseLLERSP); // Ez0n3 - use iAudioPlugin instead
	VAR(dwMaxVideoMem);

	// Ez0n3 - audio plugins
	VAR(iAudioPlugin);
	VAR(iRspPlugin);
	BOL(bUseRspAudio);
	

#undef STR
#undef FLT
#undef VAR
#undef BOL
	fclose(h);
	return 0;
}

//
int ConfigAppSave2()
{
	FILE *h;
	char szConfig[260];

	Rom *m_pRom = g_romList.GetRomAt(actualrom);
	
	//Ez0n3 - looking for alternate title when it should be looking for crc1? But I don't think this func is used - so leave it.
	sprintf(szConfig, "D:\\ini\\games\\%s.ini", m_pRom->GetProperName().c_str());
	//sprintf(szConfig, "D:\\ini\\games\\%08X.ini", m_pRom->GetCrc1()); // this is the way it should be?

	if ((h = fopen(szConfig, "wt")) == NULL) return 1;

#define BOL(x) fprintf(h, #x " %d\n", (int)(x != 0))
#define VAR(x) fprintf(h, #x " %d\n", x)
#define FLT(x) fprintf(h, #x " %f\n", x)
#define STR(x) fprintf(h, #x " %s\n", x)
	
	fprintf(h,"\n\n\n");
    
	VAR(preferedemu);

#undef STR
#undef FLT
#undef VAR
#undef BOL
	fclose(h);
	return 0;
}

//

int ConfigAppLoad2()
{
	FILE *h;
	char szConfig[260];
	char szLine[256];

	Rom *m_pRom = g_romList.GetRomAt(actualrom);

	//Ez0n3 - looking for alternate title when it should be looking for crc1? But I don't think this func is used - so leave it.
	sprintf(szConfig, "D:\\ini\\games\\%s.ini", m_pRom->GetProperName().c_str());
	//sprintf(szConfig, "D:\\ini\\games\\%08X.ini", m_pRom->GetCrc1()); // this is the way it should be?
	
	if ((h = fopen(szConfig, "rt")) == NULL) return 1;

	// Go through each line of the config file
	while (1) {
		int nLen;
		if (fgets(szLine, sizeof(szLine), h) == NULL) {				// End of config file
			break;
		}

		nLen = strlen(szLine);
		
		// Get rid of the linefeed at the end
		if (szLine[nLen - 1] == 10) {
			szLine[nLen - 1] = 0;
			nLen--;
		}

#define BOL(x) { char *szValue = LabelCheck(szLine,#x);		\
	if (szValue) x = strtol(szValue, NULL, 0) != 0; }
#define VAR(x) { char *szValue = LabelCheck(szLine,#x);		\
	if (szValue) x = strtol(szValue, NULL, 0); }
#define FLT(x) { char *szValue = LabelCheck(szLine,#x);		\
	if (szValue) x = atof(szValue); }
#define STR(x) { char *szValue = LabelCheck(szLine,#x " ");	\
	if (szValue) strcpy(x,szValue); }

	VAR(preferedemu);

#undef STR
#undef FLT
#undef VAR
#undef BOL
	}

	fclose(h);
	return 0;
}

//
char* LabelCheck(char *s,char *szLabel)
{
	int nLen;
	if (s == NULL) {
		return NULL;
	}
	if (szLabel == NULL) {
		return NULL;
	}
	nLen = strlen(szLabel);
	
	SKIP_WS(s);													// Skip whitespace

	if (strncmp(s, szLabel, nLen)){								// Doesn't match
		return NULL;
	}
	return s + nLen;
}

// Skin
char* LabelCheckNew( char *s, char *szLabel )
{
	char	*returnvalue = NULL;
	int		nLen;

	if ( s == NULL )
		return NULL;

	if ( szLabel == NULL )
		return NULL;

	// Longueur de ce que l'on recherche
	nLen = strlen( szLabel );
	
	// On saute les blancs
	SKIP_WS( s );

	// On ne tiens pas compte de la casse
	if ( _strnicmp( s, szLabel, nLen ) )
		return NULL;

	// On passe à la suite
	s += nLen;

	// On saute les blancs
	SKIP_WS( s );

	// On cherche le "="
	if ( *s != '=' )
		return NULL;

	// On passe à la suite
	s++;

	// On saute les blancs
	SKIP_WS( s );

	// On lis ce qu'il y a entre les guillemets
	if ( QuoteRead( &returnvalue, NULL, s ) ) return NULL;
	// On retourne la valeur
	return returnvalue;
}
//
int QuoteRead(char** pszQuote, char** pszEnd, char* szSrc)		// Read a quoted string from szSrc and point to the end
{
	static char szQuote[QUOTE_MAX];
	char *s = szSrc;
	int i;
	
	SKIP_WS(s);													// Skip whitespace

	if (*s != '\"')	{											// Didn't start with a quote
		return 1;
	}
	s++;

	// Copy the characters into szQuote
	for (i = 0; i < QUOTE_MAX - 1; i++, s++) {
		if (*s == '\"') {
			s++;
			break;
		}
		if (*s == 0) {											// Stop copying if end of line or end of string
			break;
		}
		szQuote[i] = *s;										// Copy character to szQuote
	}
	szQuote[i] = 0;												// Zero-terminate szQuote

	if (pszQuote) {
		*pszQuote = szQuote;
	}
	if (pszEnd)	{
		*pszEnd = s;
	}
	
	return 0;
}
*/