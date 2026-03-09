#pragma once

#include <xtl.h>
#include <string>
using namespace std;

#define NUMBER_OF_N64_BUTTONS		19
#define NUMBER_OF_XBOX_BUTTONS		24

#define NUMBER_OF_CONTROLLERS		4

//extern float XBOX_CONTROLLER_DEAD_ZONE;

#define IS_N64_AXIS(button)		(button < 4)
#define IS_XBOX_AXIS(button)	(button < 8)

enum XboxButton
{
	XboxLeftThumbLeft,
	XboxLeftThumbRight,		
	XboxLeftThumbUp,		
	XboxLeftThumbDown,		

	XboxRightThumbLeft,		
	XboxRightThumbRight,		
	XboxRightThumbUp,		
	XboxRightThumbDown,		

	XboxDPadLeft,			
	XboxDPadRight,			
	XboxDPadUp,				
	XboxDPadDown,			

	XboxStart,				
	XboxBack,				

	XboxLeftThumbButton,		
	XboxRightThumbButton,	

	XboxA,					
	XboxB,					
	XboxX,					
	XboxY,					

	XboxBlack,				
	XboxWhite,				

	XboxLeftTrigger,			
	XboxRightTrigger,		
};

enum N64Button
{
	N64ThumbLeft,			
	N64ThumbRight,			
	N64ThumbUp,				
	N64ThumbDown,			

	N64DPadLeft,				
	N64DPadRight,			
	N64DPadUp,				
	N64DPadDown,				

	N64CButtonLeft,			
	N64CButtonRight,		
	N64CButtonUp,			
	N64CButtonDown,			

	N64Start,				

	N64A,					
	N64B,					

	N64ZTrigger,				
	N64LeftTrigger,		
	N64RightTrigger,

	InGameMenu
};

class Config
{
public:
	Config(void);
	~Config(void);

	bool Load(int *cfgData);
	bool Load(string &szFilename);
	bool Save(string &szFilename);

	void MapN64ToXbox(DWORD controller, byte xboxButton, byte n64Button);

	DWORD GetXboxButtonValue(DWORD controller, byte xboxButton);
	DWORD GetN64ButtonValue(DWORD controller, byte n64Button);

	
private:
	byte m_buttonMap[NUMBER_OF_CONTROLLERS][NUMBER_OF_N64_BUTTONS];

	int m_cur64ThumbVal[NUMBER_OF_CONTROLLERS][4];
};
