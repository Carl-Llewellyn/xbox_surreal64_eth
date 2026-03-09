/*
 * LinksBoks
 * Copyright (c) 2003-2005 ysbox
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef _XBOX

#ifndef LINKSBOKS_H
#define LINKSBOKS_H

#include <xtl.h>

typedef struct _LinksBoksViewport
{
	int width, height;
	int margin_left, margin_right;
	int margin_top, margin_bottom;
} LinksBoksViewPort;

#define LINKSBOKS_OPTION_GROUP		0
#define LINKSBOKS_OPTION_INT		1
#define LINKSBOKS_OPTION_BOOL		2
#define LINKSBOKS_OPTION_STRING		3



/* Keyboard constants. Renamed but still come from links.h */
#define LINKSBOKS_MOUSE_LEFT		0
#define LINKSBOKS_MOUSE_MIDDLE	1
#define LINKSBOKS_MOUSE_RIGHT		2
#define LINKSBOKS_MOUSE_WHEELUP	8
#define LINKSBOKS_MOUSE_WHEELDOWN	9
#define LINKSBOKS_MOUSE_WHEELUP1	10
#define LINKSBOKS_MOUSE_WHEELDOWN1	11
#define LINKSBOKS_MOUSE_WHEELLEFT	12
#define LINKSBOKS_MOUSE_WHEELRIGHT	13
#define LINKSBOKS_MOUSE_WHEELLEFT1	14
#define LINKSBOKS_MOUSE_WHEELRIGHT1	15

#define LINKSBOKS_BM_ACT		48
#define LINKSBOKS_MOUSE_DOWN		0
#define LINKSBOKS_MOUSE_UP		16
#define LINKSBOKS_MOUSE_DRAG		32
#define LINKSBOKS_MOUSE_MOVE		48
#define LINKSBOKS_MOUSE_CTRL		64

#define LINKSBOKS_KBD_ENTER	-0x100
#define LINKSBOKS_KBD_BS		-0x101
#define LINKSBOKS_KBD_TAB		-0x102
#define LINKSBOKS_KBD_ESC		-0x103
#define LINKSBOKS_KBD_LEFT	-0x104
#define LINKSBOKS_KBD_RIGHT	-0x105
#define LINKSBOKS_KBD_UP		-0x106
#define LINKSBOKS_KBD_DOWN	-0x107
#define LINKSBOKS_KBD_INS		-0x108
#define LINKSBOKS_KBD_DEL		-0x109
#define LINKSBOKS_KBD_HOME	-0x10a
#define LINKSBOKS_KBD_END		-0x10b
#define LINKSBOKS_KBD_PAGE_UP	-0x10c
#define LINKSBOKS_KBD_PAGE_DOWN	-0x10d

#define LINKSBOKS_KBD_F1		-0x120
#define LINKSBOKS_KBD_F2		-0x121
#define LINKSBOKS_KBD_F3		-0x122
#define LINKSBOKS_KBD_F4		-0x123
#define LINKSBOKS_KBD_F5		-0x124
#define LINKSBOKS_KBD_F6		-0x125
#define LINKSBOKS_KBD_F7		-0x126
#define LINKSBOKS_KBD_F8		-0x127
#define LINKSBOKS_KBD_F9		-0x128
#define LINKSBOKS_KBD_F10		-0x129
#define LINKSBOKS_KBD_F11		-0x12a
#define LINKSBOKS_KBD_F12		-0x12b

#define LINKSBOKS_KBD_CTRL_C	-0x200
#define LINKSBOKS_KBD_CLOSE	-0x201
#define LINKSBOKS_KBD_PASTE       -0x202

#define LINKSBOKS_KBD_SHIFT	1
#define LINKSBOKS_KBD_CTRL	2
#define LINKSBOKS_KBD_ALT		4


class LinksBoksOption
{
public:
	LinksBoksOption(const char *name, const char *caption, int type, int depth, unsigned char *default_value);
	LinksBoksOption(const char *name, const char *caption, int type, int depth, int default_value = 0);
	VOID Register();
	/* Normally you just have to implement the version of BeforeChange you need,
	   depending on your option type. Return FALSE if you don't accept the change */
	virtual BOOL OnBeforeChange(void *session, unsigned char *oldvalue, unsigned char *newvalue);
	virtual BOOL OnBeforeChange(void *session, int oldvalue, int newvalue);
	virtual VOID OnAfterChange(void *session);

	const char *m_sName;
	const char *m_sCaption;
	int m_iType;
	int m_iDepth;
	unsigned char *m_sDefaultValue;
	int m_iDefaultValue;

protected:
	// Used during OnBeforeChange, use the void *session you get from there
	void MsgBox(void *session, unsigned char *title, unsigned char *msg);
};


/* Don't derive this class! Use LinksBoksExternalProtocol or LinksBoksInternalProtocol */
class LinksBoksProtocol
{
public:
	unsigned char *m_sName;
	int m_iPort;
	BOOL m_bFreeSyntax;
	BOOL m_bNeedSlashes;
	BOOL m_bNeedSlashAfterHost;

	virtual VOID Register() {}

	virtual int OnCall(unsigned char *url, void *) { return 0; }

protected:
	LinksBoksProtocol(unsigned char *name, int port, BOOL free_syntax, BOOL need_slashes, BOOL need_slash_after_host) :
		m_sName(name),
		m_iPort(port),
		m_bFreeSyntax(free_syntax),
		m_bNeedSlashes(need_slashes),
		m_bNeedSlashAfterHost(need_slash_after_host) {}

};

/* External protocol: the function will be triggered as soon as the user clicks the link.
You get the URL, you do whatever you want with it and that's it. You can however display
a little messagebox. */
class LinksBoksExternalProtocol : public LinksBoksProtocol
{
public:
	LinksBoksExternalProtocol(unsigned char *name, int port, BOOL free_syntax, BOOL need_slashes, BOOL need_slash_after_host) :
	  LinksBoksProtocol(name, port, free_syntax, need_slashes, need_slash_after_host) {}
	VOID Register();
	virtual int OnCall(unsigned char *url, void *session) { return 0; }

protected:
	// Used during OnCall, use the void *session you get from there
	void MsgBox(void *session, unsigned char *title, unsigned char *msg);

};

#define LINKSBOKS_RESPONSE_OK				-1000
#define LINKSBOKS_RESPONSE_INTERRUPTED		-1001
#define LINKSBOKS_RESPONSE_EXCEPTION		-1002
#define LINKSBOKS_RESPONSE_INTERNAL			-1003
#define LINKSBOKS_RESPONSE_OUT_OF_MEMORY	-1004
#define LINKSBOKS_RESPONSE_NO_DNS			-1005
#define LINKSBOKS_RESPONSE_CANT_WRITE		-1006
#define LINKSBOKS_RESPONSE_CANT_READ		-1007
#define LINKSBOKS_RESPONSE_MODIFIED			-1008
#define LINKSBOKS_RESPONSE_BAD_URL			-1009
#define LINKBOKSE_RESPONSE_TIMEOUT			-1010

/* Internal protocol: this is more like a regular protocol like HTTP; you are expected to
call (once) the SendResponse() function with a content-type, some data (ie. HTML or image
data). Additionally, the return code is important because if it's not LINKSBOKS_RESPONSE_OK,
a generic error message will be displayed instead. */
class LinksBoksInternalProtocol : public LinksBoksProtocol
{
public:
	LinksBoksInternalProtocol(unsigned char *name, int port, BOOL free_syntax, BOOL need_slashes, BOOL need_slash_after_host) :
	  LinksBoksProtocol(name, port, free_syntax, need_slashes, need_slash_after_host) {}
	VOID Register();
	virtual int OnCall(unsigned char *url, void *connection) { return LINKSBOKS_RESPONSE_EXCEPTION; }

protected:
	// Used during OnCall, use the void *session you get from there
	int SendResponse(void *connection, unsigned char *content_type, unsigned char *data, int data_size);
};


/* The LinksBoksWindow class represents an actual browser "window". Have more than one is not
very tested (at all) at this time, but it should be theorically possible. */
class LinksBoksWindow
{
public:
	//
	// INITIALIZATION FUNCTIONS
	//

	LinksBoksWindow(LinksBoksViewPort viewport);
	int Initialize(void *grdev);
	/* These functions are automatically called. Use the global function LinksBoks_CreateWindow()
	if you want to create a new window */

	//
	// RENDERING FUNCTIONS
	//

	int FlipSurface(void);
	/* This function copies the current content of the back-buffer surface into the main front-buffer
	surface you can access. It is not advised to flip every single loop iteration. A common way to do
	is to register a timer every nth milliseconds within LinksBoks and to flip in the timer callback
	(you can even do a Present() there if you don't have your own way of doing it). Check out the host
	application and the EmbeddedSample for examples */

	LPDIRECT3DSURFACE8 GetSurface(void);
	/* Returns a pointer to the LinksBoks front-buffer surface */

	/* Retrieve dimensions of the viewport */
	int GetViewPortWidth(void);
	int GetViewPortHeight(void);

	/* Resizes the viewport, using the new provided LinksBoksViewPort structure.
	WARNING: only change the margins for now! The surface size must remain the same (TODO) */
	VOID ResizeWindow(LinksBoksViewPort viewport);

	/* These functions are used by the Links graphics driver to manipulate the data
	(ie. D3D surfaces...) stored as class members; You shouldn't have to use them, but it's
	not forbidden (eg. if you want to draw something on top of the browser window) */
	VOID RegisterFlip(int x, int y, int w, int h);
	VOID Blit(LPDIRECT3DSURFACE8 pSurface, int x, int y, int w, int h );
	HRESULT CreatePrimitive(int x, int y, int w, int h, int color);
	VOID RenderPrimitive(LPDIRECT3DSURFACE8 pTargetSurface);
	VOID FillArea(int x1, int y1, int x2, int y2, long color);
	VOID DrawHLine(int x1, int y, int x2, long color);
	VOID DrawVLine(int x, int y1, int y2, long color);
	VOID SetClipArea(int x1, int y1, int x2, int y2);
	VOID ScrollBackBuffer(int x1, int y1, int x2, int y2, int offx, int offy);

	//
	// KEYBOARD/MOUSE FUNCTIONS
	//

	/* Sends an ASCII character or a special key or combination of keys to be handled
	by the Links engine */
	VOID KeyboardAction(int key, int flags);

	/* Sends a new mouse position and/or button status change */
	VOID MouseAction(int x, int y, int buttons);

	//
	// SESSION ACTIONS
	//

	/* These are to be polished up */
	VOID GoToURL(unsigned char *url);
	VOID GoToURLInNewTab(unsigned char *url);
	int NumberOfTabs();
	VOID SwitchToTab(int index);
	VOID CloseCurrentTab();
	/* and the rest is to be implemented:
	VOID GetCurrentURL(unsigned char *buffer, int size);
	VOID GetCurrentTitle(unsigned char *buffer, int size);
	VOID GoBack();
	VOID GoForward();
	VOID Stop();
	VOID Reload();
	...
	*/

	//
	// TERMINATION
	//
	/* This only queries the current window to be closed,
	if it's the last one, a confirmation dialog is displayed, in this case
	the engine is terminated too. If you call LinksBoks_Terminate(), all
	windows are automatically terminated.
	The effective termination is not immediate, it may be happen during
	a further LinksBoks_FrameMove call. Since we don't know when, you need
	to forget the object (that is, not accessing it anymore) as soon as you
	call this function */
	void Close(void);

	/* !Don't call this!
	It is supposed to be only accessed by the graphics driver.
	It releases all the D3D surfaces and stuff */
	void Terminate(void);

protected:
	void *m_grdev;							/* Graphics device */

	LPDIRECT3D8             m_pD3D;			/* Provided d3d object */
	LPDIRECT3DDEVICE8       m_pd3dDevice;	/* Provided d3d device object */
	LPDIRECT3DSURFACE8      m_pdSurface;	/* Front-buffer surface */
	LPDIRECT3DSURFACE8      m_pdBkBuffer;	/* Back-buffer surface */

	RECT					m_ClipArea;
	LinksBoksViewPort		m_ViewPort;
	RECT					m_FlipRegion;
	BOOL					m_bWantFlip;
	BOOL					m_bResized;

};




int LinksBoks_InitCore(LinksBoksOption *options[], LinksBoksProtocol *protocols[]);
/* This function loads the LinksBoks section and initializes the main stuff you may be needing before
creating the actual d3d surface. Next you have to call Init_Loop but this is your chance to
retrieve/set LinksBoks options, register external protocols, file associations timers, various
callbacks and so on, before the graphics engine initialization */

LinksBoksWindow *LinksBoks_CreateWindow(LPDIRECT3D8 pD3D, LPDIRECT3DDEVICE8 pd3dDevice, LinksBoksViewPort viewport);
/* This function initializes the last modules, including the graphics-related stuff. The d3d surface
will be created and an initial rendering will be done */

extern "C" int LinksBoks_FrameMove(void);
/* Call this function to run the LinksBoks engine's "main loop" once. Be warned that this function
doesn't usually blocks too long but it can last up to several seconds during a page's loading and/or
rendering... Bear with it for now, multithreading is not supported.
Call LinksBoks_InitCore() then LinksBoks_InitLoop() before or it will (of course) crash. */

/* Shutdowns the LinksBoks engine's subsystems and tries to free the memory resources it uses */
void LinksBoks_Terminate(BOOL bFreeXBESections);

/* Registers a new internal timer which will be called at most every 't' ms,
during LinksBoks_FrameMove(). Returns an integer id of the timer or -1 on error */
int LinksBoks_RegisterNewTimer(long t, void (*func)(void *), void *data);

/* Use this to provide a callback function for when to launch some external viewer for a mime type.
For compatibility reasons the way of doing things in Links has been left almost untouched.
Here's how it works:
1) The user associates himself a mime type (ie. "application/x-shockwave-flash" with a commandline
(by editing the links.cfg file or interactively in the Associations Manager in the Setup menu).
2) In case the mime type isn't directly provided by the server (for example when trying to open a
file on the local disk), there's also a list of file extensions <=> mime types available.
3) When asked to open a file, when this file of a given mime type, and a commandline is associated
with this mime type, Links asks whether the file has to be opened in the external viewer, and in
that case it downloads the file to some temporary location, runs that commandline, and when the
program returns, deletes the temporary file.

On Xbox, running commandlines makes no sense, but you can provide a function with takes the command-
line as a parameter (and the temp file name), and call what you wish from there. A few reminders:
- the engine will block when in that function as it is called in the same thread
- when the function returns the file may be deleted from disk, so if you create another thread to
  do your background work and prevent blocking, you better read the file into memory first before
  returning */
VOID LinksBoks_SetExecFunction(int (*exec_function)(LinksBoksWindow *pLB, unsigned char *cmdline, unsigned char *filepath, int fg));

/* LinksBoks options subsystem bindings */
BOOL LinksBoks_GetOptionBool(const char *key);
INT LinksBoks_GetOptionInt(const char *key);
unsigned char *LinksBoks_GetOptionString(const char *key);
void LinksBoks_SetOptionBool(const char *key, BOOL value);
void LinksBoks_SetOptionInt(const char *key, INT value);
void LinksBoks_SetOptionString(const char *key, unsigned char *value);

#endif

#endif