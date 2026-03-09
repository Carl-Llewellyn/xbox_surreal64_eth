#include <xtl.h>
#include "assert.h"
#include "XBUtil.h"
#include "XBInput.h"
#include "XLMenu.h"
#include "Panel.h"
#include <xbfont.h>


//weinerschnitzel - Skin Control
extern DWORD dwMenuItemColor;
extern DWORD dwNullItemColor;
extern DWORD dwSelectedRomColor;
extern char skinname[32];
extern int MenuTrunc;
extern int iLaunchMenuBgPosX;
extern int iLaunchMenuBgPosY;
extern int iLaunchHilightPosX;
extern int iIGMHilightPosX;
extern int iIGMHilightPosX_HD;
extern int iIGMMenuPosX;
extern int iIGMMenuPosY;
extern int iIGMMenuPosX_HD;
extern int iIGMMenuPosY_HD;
extern int iMainMenuPosX;
extern int iMainMenuPosY;
extern int iMainHilightPosX;
extern int iLineSpacing;
extern CPanel m_LaunchPanel;
extern CPanel m_MenuBgPanel;
extern CPanel m_MenuBg2Panel;
extern bool bEnableHDTV;
int isLaunchMenu = 0;
bool isIGM = false;
//bool g_HD_UHLE_720 = false;


extern LPDIRECT3DDEVICE8 g_pd3dDevice;

CXBFont *XLMenu_Font = NULL;                // pointer to font
DWORD XLMenu_FontLoaded = FALSE;            // font loaded flag
XLMenu *XLMenu_CurMenu = NULL;
int (*XLMenu_CurRoutine)(DWORD cmd, XLMenuItem *mi);            // called when item selected



void XLMenu_SetFont(CXBFont *font)
{
    XLMenu_Font = font;
}

XLMenu *XLMenu_Init(float x, float y, DWORD maxitems, DWORD flags, DWORD (*abortroutine)(DWORD, XLMenuItem *))
{
    XLMenu *m;
    BYTE *mem;   
	assert(XLMenu_Font!=NULL);                // make sure font is set

    // allocate memory for menu
    mem = new BYTE[sizeof(XLMenu) + maxitems*sizeof(XLMenuItem)];
    memset(mem, 0, sizeof(XLMenu) + maxitems*sizeof(XLMenuItem));

    m = (XLMenu *)mem;
    m->items = (XLMenuItem *)(mem+sizeof(XLMenu));
    m->flags = flags;
    m->maxitems = maxitems;
    m->abortroutine = abortroutine;
    m->itemcolor = dwMenuItemColor;

    m->x = x;
    m->y = y;
    m->w = 100.0f;

    XLMenu_SetMaxShow(m, m->maxitems);        // sets height

    return m;
}

void XLMenu_Delete(XLMenu *m)
{
    delete m;



}

void XLMenu_SetMaxShow(XLMenu *m, DWORD maxshow)
{
    m->maxshow = maxshow;
    m->h = (float)(m->maxshow+1) * XLMenu_Font->GetFontHeight();
}

// same as before except A and X do the same
XLMenuItem *XLMenu_AddItem(XLMenu *m, DWORD flags, WCHAR *string, void *action)
{
 XLMenu_AddItem2(m,flags,string, action,action);

 return &m->items[m->nitems-1];
}
// second function for X
XLMenuItem *XLMenu_AddItem2(XLMenu *m, DWORD flags, WCHAR *string, void *action, void *action2)
{
    XLMenuItem *mi;

    assert(m->nitems<m->maxitems);        // make sure we don't overstep our allocation
    mi = &m->items[m->nitems];

    mi->flags = flags;
    mi->action = action;
	mi->action2 = action2;
    mi->val1 = 0;
    mi->val2 = 0;
    mi->menu = m;
    mi->color = m->itemcolor;
    XLMenu_SetItemText(&m->items[m->nitems], string);

    m->nitems++;

    return &m->items[m->nitems-1];
}

void XLMenu_DeleteItem(XLMenuItem *mi)
{
    XLMenu *m;
    DWORD i;
    
    // get pointer to the menu that owns this item
    m = mi->menu;

    // find the item to delete
    for(i=0; i<m->nitems; i++)
        if(&m->items[i]==mi)
        {
            // shift em down
            memcpy(&m->items[i], &m->items[i+1], (m->nitems-i-1)*sizeof(XLMenuItem));
            m->nitems--;
            break;
        }
}

void XLMenu_SetTitle(XLMenu *m, WCHAR *string, DWORD color)
{
    float w, h;

    m->titlecolor = color;

    if(string)
    {
        // set string
        wcsncpy(m->title, string, MITEM_STRINGLEN-1);

        // calculate new menu width based on this item
        XLMenu_Font->GetTextExtent(string, &w, &h);
        if(m->w<(w+16.0f))
            m->w = w+16.0f;
    
        m->h = ((float)m->maxshow+2.5f) * XLMenu_Font->GetFontHeight();
    }
    else
        m->h = (float)(m->maxshow+1) * XLMenu_Font->GetFontHeight();
}

void XLMenu_SetItemText(XLMenuItem *mi, WCHAR *string)
{
	int k;
    float w, h;
	WCHAR MenuItemTrunc[(MITEM_STRINGLEN*2)];

    if(!string || (mi->flags&MITEM_SEPARATOR))
        return;

     //Truncate the text
	for(k = 0; k <= MenuTrunc; k++){
		MenuItemTrunc[k] = string[k];
	}
	MenuItemTrunc[(k+1)] = L'\0';
	
	// set string
	wcsncpy(mi->string, MenuItemTrunc, MITEM_STRINGLEN-1);

    // calculate new menu width based on this item
    XLMenu_Font->GetTextExtent(string, &w, &h);
    if(mi->menu->w<(w+16.0f))
        mi->menu->w = w+16.0f;
}

DWORD XLMenu_Activate(XLMenu *m)
{
    XLMenu_CurMenu = m;
	

    return 1;
}

DWORD XLMenu_GetCommand(XBGAMEPAD *gamepad)
{
    DWORD command;

    command = 0;

    // get menu command
    if(XLMenu_CurMenu || XLMenu_CurRoutine)
    {
        if(gamepad->bPressedAnalogButtons[XINPUT_GAMEPAD_A] || gamepad->wPressedButtons & XINPUT_GAMEPAD_DPAD_RIGHT || gamepad->fX1>0.8f)//gamepad->wPressedButtons&XINPUT_GAMEPAD_START)
            command = MENU_SELECTITEM;
		else if(gamepad->bPressedAnalogButtons[XINPUT_GAMEPAD_X] || gamepad->wPressedButtons & XINPUT_GAMEPAD_DPAD_LEFT|| gamepad->fX1<-0.8f)
            command = MENU_SELECTITEM2;
        else if(gamepad->bPressedAnalogButtons[XINPUT_GAMEPAD_B] ||
           gamepad->wPressedButtons&XINPUT_GAMEPAD_BACK)
            command = MENU_BACK;
        else if((gamepad->fY1>0.8f) || (gamepad->wPressedButtons & XINPUT_GAMEPAD_DPAD_UP))
            command = MENU_PREVITEM;
        else if((gamepad->fY1<-0.8f) || (gamepad->wPressedButtons & XINPUT_GAMEPAD_DPAD_DOWN))
            command = MENU_NEXTITEM;
    }
    else if(gamepad->wPressedButtons&XINPUT_GAMEPAD_START)
        command = MENU_ACTIVATE;

    return command;
}

DWORD XLMenu_Routine(DWORD command)
{
    float menux, menuy;
    float itemy, itemh;
    DWORD i, color;
    static float menutime = 0.0f;
    float time;
    XLMenu *m;
    XLMenuItem *mi;
    DWORD firstitem, nitems;
    WCHAR ddd[16] = L"[--MORE--]";
    WCHAR *str;
    DWORD status;
	

    // check for menu routine first
    if(XLMenu_CurRoutine)
    {
        status = XLMenu_CurRoutine(command, NULL);
        if(status==MROUTINE_DIE)
            XLMenu_CurRoutine = NULL;
        return 0;
    }

    // no routine so do menu processing
    m = XLMenu_CurMenu;
    if(!m)
        return 0;

    // get height of font
    itemh = (float)XLMenu_Font->GetFontHeight() + 1;

    // draw menu background

	if(isLaunchMenu == 1){
		m_LaunchPanel.Render((float)iLaunchMenuBgPosX,(float)iLaunchMenuBgPosY);
	}else if(isIGM == true){
		if(bEnableHDTV)
			m_MenuBgPanel.Render((float)iIGMMenuPosX_HD, (float)iIGMMenuPosY_HD);
		else
			m_MenuBgPanel.Render((float)iIGMMenuPosX, (float)iIGMMenuPosY);
	}else{
		m_MenuBgPanel.Render((float)iMainMenuPosX, (float)iMainMenuPosY);
	}

    menux = m->x;
    menuy = m->y;

    // determine which menu items to show (firstitem, nitems)
    if(m->nitems>m->maxshow)
    {
        if(m->curitem<(m->maxshow/2))
            firstitem = 0;
        else if(m->curitem<(m->nitems-(m->maxshow/2)))
            firstitem = m->curitem-(m->maxshow/2)+1;
        else
            firstitem = m->nitems-m->maxshow;
    }
    else
        firstitem = 0;

    if(m->nitems>m->maxshow)
        nitems = m->maxshow;
    else
        nitems = m->nitems;

    itemy = menuy;

    // draw the menu title
    if(m->title[0])
    {
        /*
        if(m->flags&MENU_LEFT)
            XLMenu_Font->DrawText(menux, itemy, m->titlecolor, m->title);
        else if(m->flags&MENU_RIGHT)
            XLMenu_Font->DrawText(menux, itemy, m->titlecolor, m->title, XBFONT_RIGHT);
        else
            XLMenu_Font->DrawText(menux, itemy, m->titlecolor, m->title, XBFONT_CENTER_X);
        */
        if(m->flags&MENU_RIGHT)
            XLMenu_Font->DrawText(menux, itemy, m->titlecolor, m->title, XBFONT_RIGHT);
        else if(m->flags&MENU_CENTER_X)
            XLMenu_Font->DrawText(menux, itemy, m->titlecolor, m->title, XBFONT_CENTER_X);
		else
			XLMenu_Font->DrawText(menux, itemy, m->titlecolor, m->title); // XBFONT_LEFT
            
        itemy += 1.5f*itemh;
    }

    // draw the menu items
    for(i=firstitem; i<nitems+firstitem; i++)
    {
        mi = &m->items[i];

        if(!(mi->flags&MITEM_SEPARATOR))
        {
            // do menu scroll
            str = mi->string;
            if(m->nitems>m->maxshow)
            {
                if((i==firstitem) && (firstitem!=0))
                    str = ddd;
                if((i==(firstitem+nitems-1)) && (firstitem!=(m->nitems-m->maxshow)))
                    str = ddd;
            }

            // draw menu highlight bar


            if(i==m->curitem)
            {
					if(isLaunchMenu == 1){
						m_MenuBg2Panel.Render((float)iLaunchHilightPosX, (float)itemy);
					}else if (isIGM == true){
						if(bEnableHDTV)
							m_MenuBg2Panel.Render((float)iIGMHilightPosX_HD, (float)itemy);
						else
							m_MenuBg2Panel.Render((float)iIGMHilightPosX, (float)itemy);
					}else{
						m_MenuBg2Panel.Render((float)iMainHilightPosX, (float)itemy);
					}
            }

            // set item color
            if(mi->flags&MITEM_DISABLED)
                color = dwNullItemColor;   
			else if(i ==m->curitem)
				color = dwSelectedRomColor;
            else
                color = mi->color;
    
            // draw the menu text
            /*
            if(!(m->flags&(MENU_LEFT|MENU_RIGHT)))
                XLMenu_Font->DrawText(menux, itemy, color, str, XBFONT_CENTER_X);
            else if(m->flags&MENU_LEFT)
                XLMenu_Font->DrawText(menux, itemy, color, str);
            else if(m->flags&MENU_RIGHT)
                XLMenu_Font->DrawText(menux, itemy, color, str, XBFONT_RIGHT);
            */
			if(m->flags&MENU_RIGHT)
                XLMenu_Font->DrawText(menux, itemy, color, str, XBFONT_RIGHT);
            else if(m->flags&MENU_CENTER_X)
                XLMenu_Font->DrawText(menux, itemy, color, str, XBFONT_CENTER_X);
			else
				XLMenu_Font->DrawText(menux, itemy, color, str); // XBFONT_LEFT
        }

        itemy += XLMenu_Font->GetFontHeight() + iLineSpacing + 1;
    }

    time = XBUtil_Timer(TIMER_GETABSOLUTETIME);

    switch(command)
    {
        case MENU_NEXTITEM:
            if((time-menutime)<0.2f)
                return 0;
            menutime = time;

            do
            {
                m->curitem++;
                if(m->curitem==m->nitems)
                    if(m->flags&MENU_WRAP)
                        m->curitem = 0;
                    else
                        m->curitem = m->nitems-1;
            } while(m->items[m->curitem].flags&(MITEM_SEPARATOR|MITEM_DISABLED));
            break;

        case MENU_PREVITEM:
            if((time-menutime)<0.2f)
                return 0;
            menutime = time;

            do 
            {
                m->curitem--;
                if(m->curitem==-1)
                    if(m->flags&MENU_WRAP)
                        m->curitem = m->nitems-1;
                    else
                        m->curitem = 0;
            } while(m->items[m->curitem].flags&(MITEM_SEPARATOR|MITEM_DISABLED));

            menutime = time;
            break;

        case MENU_SELECTITEM:
            mi = &m->items[m->curitem];

            if(mi->flags&MITEM_ROUTINE)
            {
                // do routine
                XLMenu_CurRoutine = (int (*)(DWORD, XLMenuItem *))mi->action;
                status = XLMenu_CurRoutine(MROUTINE_INIT, mi);

                // check return value... 
                if(status==MROUTINE_DIE)
                {
                    XLMenu_Activate(NULL);            // kill the current menu
                    XLMenu_CurRoutine = NULL;        // and the routine
                }
                else if(status==MROUTINE_RETURN)    // just kill the routine
                    XLMenu_CurRoutine = NULL;
                else if(status==MROUTINE_SLEEP)
                    XLMenu_Activate(NULL);            // just kill current menu
            }
            else
            {    
                // activate next menu if it exists
                if(mi->action)
                {
                    XLMenu_Activate((XLMenu *)mi->action);
                    XLMenu_CurMenu->parent = m;        // the menu that called me
                }
                else
                {
                    if(m->abortroutine)
                        m->abortroutine(MROUTINE_ABORT, NULL);

                    XLMenu_Activate(NULL);
                }
            }
            break;

        case MENU_SELECTITEM2:
            mi = &m->items[m->curitem];

            if(mi->flags&MITEM_ROUTINE)
            {
                // do routine
                XLMenu_CurRoutine = (int (*)(DWORD, XLMenuItem *))mi->action2;
                status = XLMenu_CurRoutine(MROUTINE_INIT, mi);

                // check return value... 
                if(status==MROUTINE_DIE)
                {
                    XLMenu_Activate(NULL);            // kill the current menu
                    XLMenu_CurRoutine = NULL;        // and the routine
                }
                else if(status==MROUTINE_RETURN)    // just kill the routine
                    XLMenu_CurRoutine = NULL;
                else if(status==MROUTINE_SLEEP)
                    XLMenu_Activate(NULL);            // just kill current menu
            }
            else
            {    
                // activate next menu if it exists
                if(mi->action2)
                {
                    XLMenu_Activate((XLMenu *)mi->action2);
                    XLMenu_CurMenu->parent = m;        // the menu that called me
                }
                else
                {
                    if(m->abortroutine)
                        m->abortroutine(MROUTINE_ABORT, NULL);

                    XLMenu_Activate(NULL);
                }
            }
            break;

        case MENU_BACK:
            if(!(m->flags&MENU_NOBACK))
            {
                if(m->abortroutine)                        // call abort routine if it exists
                    m->abortroutine(MROUTINE_ABORT, NULL);

                if(m->parent)                            // activate parent menu if it exists
                    XLMenu_Activate(m->parent);
				else{
                    XLMenu_Activate(NULL);
						/*m_MenuBgPanel.Destroy();
						m_MenuBg2Panel.Destroy();

						menuBgTexture->Release();
						menuBg2Texture->Release();*/

					
				}
            }
            break;

        default:
            break;
    }

    return 1;
}

//-----------------------------------------------------------------------------
// Name: DrawRect
// Desc: Draws a gradient filled rectangle
//-----------------------------------------------------------------------------

//static LPDIRECT3DVERTEXBUFFER8 XLMenu_VB = NULL;
LPDIRECT3DVERTEXBUFFER8 XLMenu_VB;

void DrawRect(float x, float y, float w, float h, DWORD dwTopColor, DWORD dwBottomColor)
{
    
    struct BACKGROUNDVERTEX 
    { 
        D3DXVECTOR4 p; 
        D3DCOLOR color; 
    } *v;

    if(XLMenu_VB == NULL)
        g_pd3dDevice->CreateVertexBuffer(4*sizeof(BACKGROUNDVERTEX), D3DUSAGE_WRITEONLY, 
                                          0L, D3DPOOL_DEFAULT, &XLMenu_VB);

    // Setup vertices for a background-covering quad
    XLMenu_VB->Lock(0, 0, (BYTE **)&v, 0L);
    v[0].p = D3DXVECTOR4(x  -0.5f, y  -0.5f, 1.0f, 1.0f);  v[0].color = dwTopColor/2;
    v[1].p = D3DXVECTOR4(x+w-0.5f, y  -0.5f, 1.0f, 1.0f);  v[1].color = dwTopColor;
    v[2].p = D3DXVECTOR4(x  -0.5f, y+h-0.5f, 1.0f, 1.0f);  v[2].color = (DWORD)(dwBottomColor*1.2);
    v[3].p = D3DXVECTOR4(x+w-0.5f, y+h-0.5f, 1.0f, 1.0f);  v[3].color = dwBottomColor;
    XLMenu_VB->Unlock();

    // set render states
    g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE); 
    g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

    // Set states
    g_pd3dDevice->SetTexture(0, NULL);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    g_pd3dDevice->SetVertexShader(D3DFVF_XYZRHW|D3DFVF_DIFFUSE);
    g_pd3dDevice->SetStreamSource(0, XLMenu_VB, sizeof(BACKGROUNDVERTEX));

    // Render the quad
    g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

    // restore render state
    g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE); 
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

}

const SHORT XINPUT_DEADZONE = (SHORT)(0.24f * FLOAT(0x7FFF));
void ComposeGamepad(XBGAMEPAD *defaultGamepad, XBGAMEPAD *gamepads)
{

	ZeroMemory(defaultGamepad, sizeof(XBGAMEPAD));
	INT nThumbLX = 0;
	INT nThumbLY = 0;
	INT nThumbRX = 0;
	INT nThumbRY = 0;

	for (DWORD i = 0; i < 4; i++) {
		if (gamepads[i].hDevice) {              
			// Only account for thumbstick info beyond the deadzone
			if (gamepads[i].sThumbLX > XINPUT_DEADZONE ||
				gamepads[i].sThumbLX < -XINPUT_DEADZONE)
				nThumbLX += gamepads[i].sThumbLX;
			if (gamepads[i].sThumbLY > XINPUT_DEADZONE ||
				gamepads[i].sThumbLY < -XINPUT_DEADZONE)
				nThumbLY += gamepads[i].sThumbLY;
			if (gamepads[i].sThumbRX > XINPUT_DEADZONE ||
				gamepads[i].sThumbRX < -XINPUT_DEADZONE)
				nThumbRX += gamepads[i].sThumbRX;
			if (gamepads[i].sThumbRY > XINPUT_DEADZONE ||
				gamepads[i].sThumbRY < -XINPUT_DEADZONE)
				nThumbRY += gamepads[i].sThumbRY;

			defaultGamepad->fX1 += gamepads[i].fX1;
			defaultGamepad->fY1 += gamepads[i].fY1;
			defaultGamepad->fX2 += gamepads[i].fX2;
			defaultGamepad->fY2 += gamepads[i].fY2;
			defaultGamepad->wButtons		+= gamepads[i].wButtons;
			defaultGamepad->wPressedButtons	+= gamepads[i].wPressedButtons;
			defaultGamepad->wLastButtons	+= gamepads[i].wLastButtons;

			if (gamepads[i].Event)
				defaultGamepad->Event = gamepads[i].Event;

			for (DWORD b = 0; b < 8; b++) {
				defaultGamepad->bAnalogButtons[b]			+= gamepads[i].bAnalogButtons[b];
				defaultGamepad->bPressedAnalogButtons[b]	+= gamepads[i].bPressedAnalogButtons[b];
				defaultGamepad->bLastAnalogButtons[b]		+= gamepads[i].bLastAnalogButtons[b];
			}
		}
	}

	// Clamp summed thumbstick values to proper range
	defaultGamepad->sThumbLX = (SHORT)nThumbLX;
	defaultGamepad->sThumbLY = (SHORT)nThumbLY;
	defaultGamepad->sThumbRX = (SHORT)nThumbRX;
	defaultGamepad->sThumbRY = (SHORT)nThumbRY;

}

DWORD getAllGamepadsCommand(XBGAMEPAD *defaultGamepad)
{
	XBInput_GetInput();
	ComposeGamepad(defaultGamepad, g_Gamepads);

	return XLMenu_GetCommand(defaultGamepad);
}
