#include "Panel.h"
#include <xbfont.h>
#include <xbapp.h>
#include "XBInput.h"
#include "config.h"

extern DWORD dwMenuItemColor;
extern DWORD dwMenuTitleColor;
extern DWORD dwSelectedRomColor;

extern DWORD GetFontAlign(int iAlign);

extern int menucenter;
extern int iControlConfigPosX;
extern int iControlConfigPosY;
extern int iControlConfigTxtPosX;
extern int iControlConfigTxtPosY;
extern int iControlConfigTxtPadLX;
extern int iControlConfigWidth;

extern CXBFont m_Font;
typedef unsigned char byte;
extern int controller;
extern int ControllerConfig[76];

WCHAR controlname[19][99] = {	
	L"Analog Left",
	L"Analog Right",
	L"Analog Up",
	L"Analog Down",

	L"DPad Left",
	L"DPad Right",
	L"DPad Up",
	L"DPad Down",

	L"C Left",
	L"C Right",
	L"C Up",
	L"C Down",

	L"Start Button",

	L"A Button",
	L"B Button",

	L"Z Trigger",
	L"Left Shoulder",
	L"Right Shoulder",
	
	L"In Game Menu",};

WCHAR controlxboxname[24][99] = {
	L"Left Analog Left \514",
	L"Left Analog Right \515",
	L"Left Analog Up \516",
	L"Left Analog Down \517",

	L"Right Analog Left \514",
	L"Right Analog Right \515",
	L"Right Analog Up \516",
	L"Right Analog Down \517",

	L"DPad Left \614",
	L"DPad Right \615",
	L"DPad Up \616",
	L"DPad Down \617",

	L"Start Button\410\411", //these take up 2 spaces, recenter with 2 ' '
	L"Back Button\412\413",

	L"Left Analog Button",
	L"Right Analog Button",

	L"A Button \400",
	L"B Button \401",
	L"X Button \402",
	L"Y Button \403",

	L"Black Button  \405 ",
	L"White Button  \404 ",

	L"Left Trigger  \406 ",
	L"Right Trigger  \407 ",};

WCHAR controlxboxcorrect[19][99];

void Fillcorrectcontrols(){
	for (int i=0;i<19;i++){
		swprintf(controlxboxcorrect[i],controlxboxname[ControllerConfig[(controller*19)+i]]);
	}}

extern CPanel m_BgPanel;
extern CPanel m_ControlConfigPanel;
extern CPanel m_ControlsPanel;
extern int iConControlsPosX;
extern int iConControlsPosY;
extern int iConControlsTxtPosX;
extern int iConControlsTxtPosY;
extern int iConControlsTxtAlign;
extern int itemh;
extern int iControlsSpacing;
int selectcontrol=1;
int selectedelement=0;
bool changebutton=false;

void Drawcontrol()
{
	m_BgPanel.Render(0,0);

	m_ControlConfigPanel.Render((float)iControlConfigPosX, (float)iControlConfigPosY);
		//menucenter = (int)(iControlConfigWidth/2);
		WCHAR m_currentname[99];
		Fillcorrectcontrols();
		/*m_Font.Begin();
		m_Font.DrawText((float)(iControlConfigTxtPosX+menucenter-(iControlConfigWidth/4)), (float)(iControlConfigTxtPosY-2), dwMenuTitleColor,L"Nintendo 64" , XBFONT_CENTER_X);
		m_Font.DrawText((float)(iControlConfigTxtPosX+menucenter+(iControlConfigWidth/4)), (float)(iControlConfigTxtPosY-2), dwMenuTitleColor,L"XBOX" , XBFONT_CENTER_X);
		m_Font.End();*/
		for (int j=0;j<3;j++){
			int y=iControlConfigTxtPosY;
			int x,center;
		for (int i = 0;i<19;i++,y=y+itemh+iControlsSpacing) {
			switch (j){
			case 0:  swprintf(m_currentname,L"%s",controlname[i]);
					x=(iControlConfigTxtPosX-(iControlConfigWidth/2))+iControlConfigTxtPadLX; //28
					center = XBFONT_LEFT;
					break;
			case 1:  swprintf(m_currentname,L"");
					x=iControlConfigTxtPosX;
					center = XBFONT_CENTER_X;
					break;
			case 2:  if ((i== selectedelement)&&(changebutton))
					swprintf(m_currentname,L"Press a Button",controlname[i]);
					else 
					swprintf(m_currentname,L"%s",controlxboxcorrect[i]);
					x=iControlConfigTxtPosX+(iControlConfigWidth/2);
					center = XBFONT_RIGHT;
					break;}

		m_Font.Begin();
		if (i== selectedelement){
			m_Font.DrawText((float)x, (float)y, dwSelectedRomColor, m_currentname, center);}
		else {
		m_Font.DrawText((float)x, (float)y, dwMenuItemColor, m_currentname, center);}
		m_Font.End();
		}
	
	m_ControlsPanel.Render((float)iConControlsPosX, (float)iConControlsPosY);
		m_Font.Begin();
		m_Font.DrawText((float)iConControlsTxtPosX, (float)iConControlsTxtPosY, dwMenuItemColor,L"\402 Reset" , GetFontAlign(iConControlsTxtAlign));
		m_Font.DrawText((float)iConControlsTxtPosX, (float)(iConControlsTxtPosY+itemh), dwMenuItemColor,L"\400 Change" , GetFontAlign(iConControlsTxtAlign));
		m_Font.DrawText((float)iConControlsTxtPosX, (float)(iConControlsTxtPosY+(2*itemh)), dwMenuItemColor,L"\401 Back" , GetFontAlign(iConControlsTxtAlign));
		m_Font.End();}
}


void ChangeControl()
{
	bool quitcontrol=false;
	Sleep(500);
	XBInput_GetInput();
    float tempdeadzone = XBOX_CONTROLLER_DEAD_ZONE;
//	XBOX_CONTROLLER_DEAD_ZONE = 15000;
	while (!quitcontrol){
	XBInput_GetInput();
	if (g_Gamepads[controller].sThumbLX < -15000)
	    {
        ControllerConfig[(controller*19)+selectedelement]=0;
		quitcontrol=true;}
	if (g_Gamepads[controller].sThumbLX > 15000)
	    {
        ControllerConfig[(controller*19)+selectedelement]=1;
		quitcontrol=true;}
	if (g_Gamepads[controller].sThumbLY > 15000)
	    {
        ControllerConfig[(controller*19)+selectedelement]=2;
		quitcontrol=true;}
	if (g_Gamepads[controller].sThumbLY < -15000)
	    {
        ControllerConfig[(controller*19)+selectedelement]=3;
		quitcontrol=true;}
	if (g_Gamepads[controller].sThumbRX < -15000)
	    {
        ControllerConfig[(controller*19)+selectedelement]=4;
		quitcontrol=true;}
	if (g_Gamepads[controller].sThumbRX > 15000)
	    {
        ControllerConfig[(controller*19)+selectedelement]=5;
		quitcontrol=true;}
	if (g_Gamepads[controller].sThumbRY > 15000)
	    {
        ControllerConfig[(controller*19)+selectedelement]=6;
		quitcontrol=true;}
	if (g_Gamepads[controller].sThumbRY < -15000)
	    {
        ControllerConfig[(controller*19)+selectedelement]=7;
		quitcontrol=true;}
	if (g_Gamepads[controller].wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
	    {
        ControllerConfig[(controller*19)+selectedelement]=8;
		quitcontrol=true;}
	if (g_Gamepads[controller].wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
	    {
        ControllerConfig[(controller*19)+selectedelement]=9;
		quitcontrol=true;}
	if (g_Gamepads[controller].wButtons & XINPUT_GAMEPAD_DPAD_UP)
	    {
        ControllerConfig[(controller*19)+selectedelement]=10;
		quitcontrol=true;}
	if (g_Gamepads[controller].wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
	    {
        ControllerConfig[(controller*19)+selectedelement]=11;
		quitcontrol=true;}
	if (g_Gamepads[controller].wButtons & XINPUT_GAMEPAD_START)
	    {
        ControllerConfig[(controller*19)+selectedelement]=12;
		quitcontrol=true;}
	if (g_Gamepads[controller].wButtons & XINPUT_GAMEPAD_BACK)
	    {
        ControllerConfig[(controller*19)+selectedelement]=13;
		quitcontrol=true;}
	if (g_Gamepads[controller].wButtons & XINPUT_GAMEPAD_LEFT_THUMB)
	    {
        ControllerConfig[(controller*19)+selectedelement]=14;
		quitcontrol=true;}
	if (g_Gamepads[controller].wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
	    {
        ControllerConfig[(controller*19)+selectedelement]=15;
		quitcontrol=true;}
	if(g_Gamepads[controller].bAnalogButtons[XINPUT_GAMEPAD_A])
	    {
        ControllerConfig[(controller*19)+selectedelement]=16;
		quitcontrol=true;}
	if(g_Gamepads[controller].bAnalogButtons[XINPUT_GAMEPAD_B])
	    {
        ControllerConfig[(controller*19)+selectedelement]=17;
		quitcontrol=true;}
	if(g_Gamepads[controller].bAnalogButtons[XINPUT_GAMEPAD_X])
	    {
        ControllerConfig[(controller*19)+selectedelement]=18;
		quitcontrol=true;}
	if(g_Gamepads[controller].bAnalogButtons[XINPUT_GAMEPAD_Y])
	    {
        ControllerConfig[(controller*19)+selectedelement]=19;
		quitcontrol=true;}
	if(g_Gamepads[controller].bAnalogButtons[XINPUT_GAMEPAD_BLACK])
	    {
        ControllerConfig[(controller*19)+selectedelement]=20;
		quitcontrol=true;}
	if(g_Gamepads[controller].bAnalogButtons[XINPUT_GAMEPAD_WHITE])
	    {
        ControllerConfig[(controller*19)+selectedelement]=21;
		quitcontrol=true;}
	if(g_Gamepads[controller].bAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER])
	    {
        ControllerConfig[(controller*19)+selectedelement]=22;
		quitcontrol=true;}
	if(g_Gamepads[controller].bAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER])
	    {
        ControllerConfig[(controller*19)+selectedelement]=23;
	    quitcontrol=true;}
	}
	changebutton=false;
	//XBOX_CONTROLLER_DEAD_ZONE = tempdeadzone;
}

void ControllerReset(){
ControllerConfig[(controller*19)+0]=0x00;
ControllerConfig[(controller*19)+1]=0x01;
ControllerConfig[(controller*19)+2]=0x02;
ControllerConfig[(controller*19)+3]=0x03;
ControllerConfig[(controller*19)+4]=0x08;
ControllerConfig[(controller*19)+5]=0x09;
ControllerConfig[(controller*19)+6]=0x0A;
ControllerConfig[(controller*19)+7]=0x0B;
ControllerConfig[(controller*19)+8]=0x04;
ControllerConfig[(controller*19)+9]=0x05;
ControllerConfig[(controller*19)+10]=0x06;
ControllerConfig[(controller*19)+11]=0x07;
ControllerConfig[(controller*19)+12]=0x0C;
ControllerConfig[(controller*19)+13]=0x10;
ControllerConfig[(controller*19)+14]=0x12;
ControllerConfig[(controller*19)+15]=0x16;
ControllerConfig[(controller*19)+16]=0x0D;
ControllerConfig[(controller*19)+17]=0x17;
ControllerConfig[(controller*19)+18]=0x0F;
}