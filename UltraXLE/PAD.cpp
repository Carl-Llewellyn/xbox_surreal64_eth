//#include <windows.h>
#include "ultra.h"
#include "ultrahle.h"
#include "XBInput.h"

DWORD  m_xiInsertions = 0;
DWORD  m_xiRemovals   = 0;
HANDLE m_hXPort[4];

BYTE g_padType = 0;

/*typedef struct
{
    word button;
    char stickx;
    char sticky;
} PadStructData;

PadStructData g_Pads[4];*/
word          lastbutton;
int           xnarrow;
int           xcenter;
int           ycenter;
int           joyactive;
int           selectpad;

int           mouseactive;
int           mousedisablecnt;

int           lastjoybuttons;

int  wire=0;
int  info=0;

//#define AVERAGE 2

//#define MM_PADSTRUCTDATA 0x8033afa8

void pad_detect(void)
{
/*	for (int i = 0; i < 4; i++)
	{
		if (m_xiInsertions & (1<<i))
		{
			m_hXPort[i] = XInputOpen(XDEVICE_TYPE_GAMEPAD, i, XDEVICE_NO_SLOT, NULL);
		}
		else if (m_xiRemovals & (1<<i))
		{
			XInputClose(m_hXPort[i]);
			m_hXPort[i] = NULL;
		}
	}*/
}

void pad_buttons(void)
{
/*    g_Pads[0].button=0;
	g_Pads[1].button=0;
	g_Pads[2].button=0;
	g_Pads[3].button=0;

	BOOL xiChange = XGetDeviceChanges(XDEVICE_TYPE_GAMEPAD, &m_xiInsertions, &m_xiRemovals);

	if (xiChange)
	{
		// redetect which pads are connected
		pad_detect();
	}

	XINPUT_STATE xIS;

	for (int i = 0; i < 4; i++)
	{
		if (!m_hXPort[i])
			continue;

		XInputGetState(m_hXPort[i], &xIS);

		if (xIS.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)
		{
			g_menuCmd = true;
		}

		if (xIS.Gamepad.wButtons & XINPUT_GAMEPAD_START)
			g_Pads[i].button|=START;

		if (xIS.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)
			g_Pads[i].button|=L_TRIG;

		if (xIS.Gamepad.bAnalogButtons[XINPUT_GAMEPAD_LEFT_TRIGGER])
			g_Pads[i].button|=Z_TRIG;

		if (xIS.Gamepad.bAnalogButtons[XINPUT_GAMEPAD_RIGHT_TRIGGER])
			g_Pads[i].button|=R_TRIG;

		if (g_padType == 0)
		{
			if (xIS.Gamepad.bAnalogButtons[XINPUT_GAMEPAD_A])
				g_Pads[i].button|=A_BUTTON;

			if (xIS.Gamepad.bAnalogButtons[XINPUT_GAMEPAD_B])
				g_Pads[i].button|=D_CBUT;

			if (xIS.Gamepad.bAnalogButtons[XINPUT_GAMEPAD_X])
				g_Pads[i].button|=B_BUTTON;

			if (xIS.Gamepad.bAnalogButtons[XINPUT_GAMEPAD_Y])
				g_Pads[i].button|=L_CBUT;

			if (xIS.Gamepad.bAnalogButtons[XINPUT_GAMEPAD_WHITE])
				g_Pads[i].button|=U_CBUT;

			if (xIS.Gamepad.bAnalogButtons[XINPUT_GAMEPAD_BLACK])
				g_Pads[i].button|=R_CBUT;
		} 
		else
		{
			if (xIS.Gamepad.bAnalogButtons[XINPUT_GAMEPAD_A])
				g_Pads[i].button|=D_CBUT;

			if (xIS.Gamepad.bAnalogButtons[XINPUT_GAMEPAD_B])
				g_Pads[i].button|=R_CBUT;

			if (xIS.Gamepad.bAnalogButtons[XINPUT_GAMEPAD_X])
				g_Pads[i].button|=L_CBUT;

			if (xIS.Gamepad.bAnalogButtons[XINPUT_GAMEPAD_Y])
				g_Pads[i].button|=U_CBUT;

			if (xIS.Gamepad.bAnalogButtons[XINPUT_GAMEPAD_WHITE])
				g_Pads[i].button|=A_BUTTON;

			if (xIS.Gamepad.bAnalogButtons[XINPUT_GAMEPAD_BLACK])
				g_Pads[i].button|=B_BUTTON;
		}

		if (xIS.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)
			g_Pads[i].button|=U_JPAD;

		if (xIS.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
			g_Pads[i].button|=D_JPAD;

		if (xIS.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
			g_Pads[i].button|=L_JPAD;

		if (xIS.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
			g_Pads[i].button|=R_JPAD;

		char xval = xIS.Gamepad.sThumbLX/258;
		char yval = xIS.Gamepad.sThumbLY/258;

		if (xval > 20 || xval < -20)
		{
			g_Pads[i].stick_x = xval;
		}
		else
		{
			g_Pads[i].stick_x = 0;
		}
		
		if (yval > 20 || yval < -20)
		{
			g_Pads[i].stick_y = yval;
		}
		else
		{
			g_Pads[i].stick_y = 0;
		}
	}*/
}

void pad_init(void)
{
/*	FILE *file = fopen("T:\\Data\\Settings.dat", "rb");

	if (file)				
	{
		fread(&g_padType, sizeof(BYTE), sizeof(BYTE), file);
		fclose(file);
	}

	// initialise 4 controllers
	XDEVICE_PREALLOC_TYPE deviceTypes[] = {{XDEVICE_TYPE_GAMEPAD, 4}};

	XInitDevices(sizeof(deviceTypes)/sizeof(XDEVICE_PREALLOC_TYPE), deviceTypes);
	
	// get which controllers are currently connected
	m_xiInsertions = XGetDevices(XDEVICE_TYPE_GAMEPAD);
	m_xiRemovals   = m_xiInsertions;

	// initialise the detected pads
	pad_detect();*/
}

void pad_drawframe(void)
{
    if(!rdp_gfxactive() || st.keyboarddisable) return;

    pad_buttons();

	/*for (int i = 0; i < 4; i++)
	{
		if(g_Pads[i].stickx> 80) g_Pads[i].stickx= 80;
		if(g_Pads[i].stickx<-80) g_Pads[i].stickx=-80;
		if(g_Pads[i].sticky> 80) g_Pads[i].sticky= 80;
		if(g_Pads[i].sticky<-80) g_Pads[i].sticky=-80;
	}*/
}

void pad_frame(void)
{
}

dword pad_getdata(int pad)
{
    dword state;
    if(pad == 0) 
		state=*(dword *)&g_Pads[0];
    else 
		state=0;

    if(!pad) 
		st.padstate = state;

    state = FLIP32(state);

    return(state);
}

void pad_writedata(dword addr)
{
    dword state = pad_getdata(0);
    mem_write32(addr,state);
}

