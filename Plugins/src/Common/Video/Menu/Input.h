// not used

/*
#pragma once

#include "Common.h"
//freakdave
//#define XBINPUT_DEADZONE 0.24f
#define XBINPUT_PRESS_DELAY 200

struct XBGAMEPAD : public XINPUT_GAMEPAD
{
    // thumb stick values converted to range [-1,+1]
    float fX1;
    float fY1;
    float fX2;
    float fY2;
    
    // state of buttons tracked since last poll
    word wLastButtons;
    bool bLastAnalogButtons[8];
    word wPressedButtons;
    bool bPressedAnalogButtons[8];

    // rumble properties
    XINPUT_RUMBLE Rumble;
    XINPUT_FEEDBACK Feedback;

    // device properties
    XINPUT_CAPABILITIES caps;
    HANDLE hDevice;

    // flags for whether game pad was just inserted or removed
    bool bInserted;
    bool bRemoved;
};

#define XBOX_BUTTON_COUNT 23

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
	N64RightTrigger			
};

class Input
{
public:
	Input(void);
	~Input(void);

	bool Create();
	void GetInput();

	bool IsButtonPressed(XboxButton button);
	
	byte IsLTriggerPressed();
	byte IsRTriggerPressed();

	DWORD GetButtonDelay();
	void SetButtonDelay(DWORD milliseconds);

private:
	void RefreshDevices();

private:
	XINPUT_POLLING_PARAMETERS m_pollingParameters;
	XINPUT_STATE m_inputStates[4];
	XBGAMEPAD m_gamepads[4];

	bool m_buttonPressed;
	DWORD m_buttonDelay;
	DWORD m_lastTick;
};

extern Input g_input;
*/