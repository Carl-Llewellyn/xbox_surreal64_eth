#ifndef __XLMenu_H
#define __XLMenu_H

#include "XBFont.h"
#include "XBInput.h"

// menu item flags
#define MITEM_SEPARATOR        0x0001            // this item is a separator
#define MITEM_ROUTINE        0x0002            // this item calls a routine
#define MITEM_DISABLED        0x0004            // this item is grayed out

// menu routine commands
#define MROUTINE_INIT        0xffff            // first call to routine
#define MROUTINE_ABORT        0xfffe            // abort routine triggered

// menu routine return values
#define MROUTINE_RETURN        0x0001            // return to menu
#define MROUTINE_DIE        0x0002            // kill routine
#define MROUTINE_SLEEP        0x0003            // routine sleeps

// menu item definition
#define MITEM_STRINGLEN 256
typedef struct _XLMenuitem
{
    DWORD flags;
    WCHAR string[MITEM_STRINGLEN];            // text to display
    DWORD val1;                                // user defined values
    DWORD val2;
    void *action;                            // link to menu or routine
	void *action2;
    struct _XLMenu *menu;                    // menu that owns this item
    DWORD color;                            // item color
} XLMenuItem;

// menu commands
#define MENU_NEXTITEM        0x0001            // next menu item
#define MENU_PREVITEM        0x0002            // previous menu item
#define MENU_SELECTITEM        0x0003            // select this menu item
#define MENU_BACK            0x0004            // go back one level
#define MENU_ACTIVATE        0x0005            // activate menu
#define MENU_SELECTITEM2        0x0006            // select this menu item

// menu flags
/*#define MENU_LEFT            0x0001            // left justify menu text
#define MENU_RIGHT            0x0002            // right justify menu text*/
// like xbfont
#define MENU_LEFT            0x0000            // left justify menu text
#define MENU_RIGHT            0x0001            // right justify menu text
#define MENU_CENTER_X            0x0002            // center menu text
//#define MENU_CENTER_Y            0x0003            // center menu text

#define MENU_WRAP            0x0004            // wrap menu selection
#define MENU_NOBACK            0x0008            // cannot use back button

typedef struct _XLMenu
{
    float x, y;						// screen position
    float w, h;                      // menu size
    DWORD flags;                            // menu flags
    DWORD itemcolor;                        // menu item default color
    struct _XLMenu *parent;                    // parent menu

    WCHAR title[MITEM_STRINGLEN];            // menu title
    DWORD titlecolor;                        // menu title color
    DWORD maxshow;                            // max # of items to show at one time
    DWORD maxitems;                            // max # of items
    DWORD nitems;                            // # of items
    DWORD curitem;                            // current selected item
    XLMenuItem *items;                        // items

    DWORD (*abortroutine)(DWORD, XLMenuItem *);    // called when menu aborted
} XLMenu;

#ifdef __cplusplus
extern "C" {
#endif

extern XLMenu *XLMenu_CurMenu;
extern int (*XLMenu_CurRoutine)(DWORD cmd, XLMenuItem *mi);

void XLMenu_SetFont(CXBFont *font);
XLMenu *XLMenu_Init(float x, float y, DWORD maxitems, DWORD flags, DWORD (*abortroutine)(DWORD, XLMenuItem *));
void XLMenu_Shutdown();
void XLMenu_Delete(XLMenu *m);
XLMenuItem *XLMenu_AddItem(XLMenu *m, DWORD flags, WCHAR *string, void *action);
XLMenuItem *XLMenu_AddItem2(XLMenu *m, DWORD flags, WCHAR *string, void *action, void *action2);
void XLMenu_DeleteItem(XLMenuItem *mi);
void XLMenu_SetMaxShow(XLMenu *m, DWORD maxshow);
void XLMenu_SetTitle(XLMenu *m, WCHAR *string, DWORD color);
void XLMenu_SetItemText(XLMenuItem *mi, WCHAR *string);
void XLMenu_InitGraphics();
#define XLMenu_IsActive() (XLMenu_CurMenu||XLMenu_CurRoutine)

DWORD XLMenu_GetCommand(XBGAMEPAD *gamepad);    // gets command based on game pad
DWORD XLMenu_Activate(XLMenu *m);                // activates a menu
DWORD XLMenu_Routine(DWORD command);            // called each frame to update menu
DWORD getAllGamepadsCommand(XBGAMEPAD *defaultGamepad);

#ifdef __cplusplus
}
#endif
#endif