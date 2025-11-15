#ifndef WIN_MESSAGE_MAP_H
#define WIN_MESSAGE_MAP_H

/* Automatically generated from the file C:/Program Files (x86)/Windows Kits/10/Include/10.0.22621.0/um/WinUser.h */

#include <map>
#include <string>
#include <windows.h>
#include <winuser.h>

#ifndef WM_ACTIVATE
#define WM_ACTIVATE 0x0006
#endif

#ifndef WM_ACTIVATEAPP
#define WM_ACTIVATEAPP 0x001c
#endif

#ifndef WM_AFXFIRST
#define WM_AFXFIRST 0x0360
#endif

#ifndef WM_AFXLAST
#define WM_AFXLAST 0x037f
#endif

#ifndef WM_APP
#define WM_APP 0x8000
#endif

#ifndef WM_APPCOMMAND
#define WM_APPCOMMAND 0x0319
#endif

#ifndef WM_ASKCBFORMATNAME
#define WM_ASKCBFORMATNAME 0x030c
#endif

#ifndef WM_CANCELJOURNAL
#define WM_CANCELJOURNAL 0x004b
#endif

#ifndef WM_CANCELMODE
#define WM_CANCELMODE 0x001f
#endif

#ifndef WM_CAPTURECHANGED
#define WM_CAPTURECHANGED 0x0215
#endif

#ifndef WM_CHANGECBCHAIN
#define WM_CHANGECBCHAIN 0x030d
#endif

#ifndef WM_CHANGEUISTATE
#define WM_CHANGEUISTATE 0x0127
#endif

#ifndef WM_CHAR
#define WM_CHAR 0x0102
#endif

#ifndef WM_CHARTOITEM
#define WM_CHARTOITEM 0x002f
#endif

#ifndef WM_CHILDACTIVATE
#define WM_CHILDACTIVATE 0x0022
#endif

#ifndef WM_CLEAR
#define WM_CLEAR 0x0303
#endif

#ifndef WM_CLIPBOARDUPDATE
#define WM_CLIPBOARDUPDATE 0x031d
#endif

#ifndef WM_CLOSE
#define WM_CLOSE 0x0010
#endif

#ifndef WM_COMMAND
#define WM_COMMAND 0x0111
#endif

#ifndef WM_COMMNOTIFY
#define WM_COMMNOTIFY 0x0044 / //* /no /longer /suported /*/
#endif

#ifndef WM_COMPACTING
#define WM_COMPACTING 0x0041
#endif

#ifndef WM_COMPAREITEM
#define WM_COMPAREITEM 0x0039
#endif

#ifndef WM_CONTEXTMENU
#define WM_CONTEXTMENU 0x007b
#endif

#ifndef WM_COPY
#define WM_COPY 0x0301
#endif

#ifndef WM_COPYDATA
#define WM_COPYDATA 0x004a
#endif

#ifndef WM_CREATE
#define WM_CREATE 0x0001
#endif

#ifndef WM_CTLCOLORBTN
#define WM_CTLCOLORBTN 0x0135
#endif

#ifndef WM_CTLCOLORDLG
#define WM_CTLCOLORDLG 0x0136
#endif

#ifndef WM_CTLCOLOREDIT
#define WM_CTLCOLOREDIT 0x0133
#endif

#ifndef WM_CTLCOLORLISTBOX
#define WM_CTLCOLORLISTBOX 0x0134
#endif

#ifndef WM_CTLCOLORMSGBOX
#define WM_CTLCOLORMSGBOX 0x0132
#endif

#ifndef WM_CTLCOLORSCROLLBAR
#define WM_CTLCOLORSCROLLBAR 0x0137
#endif

#ifndef WM_CTLCOLORSTATIC
#define WM_CTLCOLORSTATIC 0x0138
#endif

#ifndef WM_CUT
#define WM_CUT 0x0300
#endif

#ifndef WM_DEADCHAR
#define WM_DEADCHAR 0x0103
#endif

#ifndef WM_DELETEITEM
#define WM_DELETEITEM 0x002d
#endif

#ifndef WM_DESTROY
#define WM_DESTROY 0x0002
#endif

#ifndef WM_DESTROYCLIPBOARD
#define WM_DESTROYCLIPBOARD 0x0307
#endif

#ifndef WM_DEVICECHANGE
#define WM_DEVICECHANGE 0x0219
#endif

#ifndef WM_DEVMODECHANGE
#define WM_DEVMODECHANGE 0x001b
#endif

#ifndef WM_DISPLAYCHANGE
#define WM_DISPLAYCHANGE 0x007e
#endif

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02e0
#endif

#ifndef WM_DPICHANGED_AFTERPARENT
#define WM_DPICHANGED_AFTERPARENT 0x02e3
#endif

#ifndef WM_DPICHANGED_BEFOREPARENT
#define WM_DPICHANGED_BEFOREPARENT 0x02e2
#endif

#ifndef WM_DRAWCLIPBOARD
#define WM_DRAWCLIPBOARD 0x0308
#endif

#ifndef WM_DRAWITEM
#define WM_DRAWITEM 0x002b
#endif

#ifndef WM_DROPFILES
#define WM_DROPFILES 0x0233
#endif

#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED 0x0320
#endif

#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED 0x031e
#endif

#ifndef WM_DWMNCRENDERINGCHANGED
#define WM_DWMNCRENDERINGCHANGED 0x031f
#endif

#ifndef WM_DWMSENDICONICLIVEPREVIEWBITMAP
#define WM_DWMSENDICONICLIVEPREVIEWBITMAP 0x0326
#endif

#ifndef WM_DWMSENDICONICTHUMBNAIL
#define WM_DWMSENDICONICTHUMBNAIL 0x0323
#endif

#ifndef WM_DWMWINDOWMAXIMIZEDCHANGE
#define WM_DWMWINDOWMAXIMIZEDCHANGE 0x0321
#endif

#ifndef WM_ENABLE
#define WM_ENABLE 0x000a
#endif

#ifndef WM_ENDSESSION
#define WM_ENDSESSION 0x0016
#endif

#ifndef WM_ENTERIDLE
#define WM_ENTERIDLE 0x0121
#endif

#ifndef WM_ENTERMENULOOP
#define WM_ENTERMENULOOP 0x0211
#endif

#ifndef WM_ENTERSIZEMOVE
#define WM_ENTERSIZEMOVE 0x0231
#endif

#ifndef WM_ERASEBKGND
#define WM_ERASEBKGND 0x0014
#endif

#ifndef WM_EXITMENULOOP
#define WM_EXITMENULOOP 0x0212
#endif

#ifndef WM_EXITSIZEMOVE
#define WM_EXITSIZEMOVE 0x0232
#endif

#ifndef WM_FONTCHANGE
#define WM_FONTCHANGE 0x001d
#endif

#ifndef WM_GESTURE
#define WM_GESTURE 0x0119
#endif

#ifndef WM_GESTURENOTIFY
#define WM_GESTURENOTIFY 0x011a
#endif

#ifndef WM_GETDLGCODE
#define WM_GETDLGCODE 0x0087
#endif

#ifndef WM_GETDPISCALEDSIZE
#define WM_GETDPISCALEDSIZE 0x02e4
#endif

#ifndef WM_GETFONT
#define WM_GETFONT 0x0031
#endif

#ifndef WM_GETHOTKEY
#define WM_GETHOTKEY 0x0033
#endif

#ifndef WM_GETICON
#define WM_GETICON 0x007f
#endif

#ifndef WM_GETMINMAXINFO
#define WM_GETMINMAXINFO 0x0024
#endif

#ifndef WM_GETOBJECT
#define WM_GETOBJECT 0x003d
#endif

#ifndef WM_GETTEXT
#define WM_GETTEXT 0x000d
#endif

#ifndef WM_GETTEXTLENGTH
#define WM_GETTEXTLENGTH 0x000e
#endif

#ifndef WM_GETTITLEBARINFOEX
#define WM_GETTITLEBARINFOEX 0x033f
#endif

#ifndef WM_HANDHELDFIRST
#define WM_HANDHELDFIRST 0x0358
#endif

#ifndef WM_HANDHELDLAST
#define WM_HANDHELDLAST 0x035f
#endif

#ifndef WM_HELP
#define WM_HELP 0x0053
#endif

#ifndef WM_HOTKEY
#define WM_HOTKEY 0x0312
#endif

#ifndef WM_HSCROLL
#define WM_HSCROLL 0x0114
#endif

#ifndef WM_HSCROLLCLIPBOARD
#define WM_HSCROLLCLIPBOARD 0x030e
#endif

#ifndef WM_ICONERASEBKGND
#define WM_ICONERASEBKGND 0x0027
#endif

#ifndef WM_IME_CHAR
#define WM_IME_CHAR 0x0286
#endif

#ifndef WM_IME_COMPOSITION
#define WM_IME_COMPOSITION 0x010f
#endif

#ifndef WM_IME_COMPOSITIONFULL
#define WM_IME_COMPOSITIONFULL 0x0284
#endif

#ifndef WM_IME_CONTROL
#define WM_IME_CONTROL 0x0283
#endif

#ifndef WM_IME_ENDCOMPOSITION
#define WM_IME_ENDCOMPOSITION 0x010e
#endif

#ifndef WM_IME_KEYDOWN
#define WM_IME_KEYDOWN 0x0290
#endif

#ifndef WM_IME_KEYLAST
#define WM_IME_KEYLAST 0x010f
#endif

#ifndef WM_IME_KEYUP
#define WM_IME_KEYUP 0x0291
#endif

#ifndef WM_IME_NOTIFY
#define WM_IME_NOTIFY 0x0282
#endif

#ifndef WM_IME_REQUEST
#define WM_IME_REQUEST 0x0288
#endif

#ifndef WM_IME_SELECT
#define WM_IME_SELECT 0x0285
#endif

#ifndef WM_IME_SETCONTEXT
#define WM_IME_SETCONTEXT 0x0281
#endif

#ifndef WM_IME_STARTCOMPOSITION
#define WM_IME_STARTCOMPOSITION 0x010d
#endif

#ifndef WM_INITDIALOG
#define WM_INITDIALOG 0x0110
#endif

#ifndef WM_INITMENU
#define WM_INITMENU 0x0116
#endif

#ifndef WM_INITMENUPOPUP
#define WM_INITMENUPOPUP 0x0117
#endif

#ifndef WM_INPUT
#define WM_INPUT 0x00ff
#endif

#ifndef WM_INPUT_DEVICE_CHANGE
#define WM_INPUT_DEVICE_CHANGE 0x00fe
#endif

#ifndef WM_INPUTLANGCHANGE
#define WM_INPUTLANGCHANGE 0x0051
#endif

#ifndef WM_INPUTLANGCHANGEREQUEST
#define WM_INPUTLANGCHANGEREQUEST 0x0050
#endif

#ifndef WM_KEYDOWN
#define WM_KEYDOWN 0x0100
#endif

#ifndef WM_KEYFIRST
#define WM_KEYFIRST 0x0100
#endif

#ifndef WM_KEYLAST
#define WM_KEYLAST 0x0108
#endif

#ifndef WM_KEYLAST
#define WM_KEYLAST 0x0109
#endif

#ifndef WM_KEYUP
#define WM_KEYUP 0x0101
#endif

#ifndef WM_KILLFOCUS
#define WM_KILLFOCUS 0x0008
#endif

#ifndef WM_LBUTTONDBLCLK
#define WM_LBUTTONDBLCLK 0x0203
#endif

#ifndef WM_LBUTTONDOWN
#define WM_LBUTTONDOWN 0x0201
#endif

#ifndef WM_LBUTTONUP
#define WM_LBUTTONUP 0x0202
#endif

#ifndef WM_MBUTTONDBLCLK
#define WM_MBUTTONDBLCLK 0x0209
#endif

#ifndef WM_MBUTTONDOWN
#define WM_MBUTTONDOWN 0x0207
#endif

#ifndef WM_MBUTTONUP
#define WM_MBUTTONUP 0x0208
#endif

#ifndef WM_MDIACTIVATE
#define WM_MDIACTIVATE 0x0222
#endif

#ifndef WM_MDICASCADE
#define WM_MDICASCADE 0x0227
#endif

#ifndef WM_MDICREATE
#define WM_MDICREATE 0x0220
#endif

#ifndef WM_MDIDESTROY
#define WM_MDIDESTROY 0x0221
#endif

#ifndef WM_MDIGETACTIVE
#define WM_MDIGETACTIVE 0x0229
#endif

#ifndef WM_MDIICONARRANGE
#define WM_MDIICONARRANGE 0x0228
#endif

#ifndef WM_MDIMAXIMIZE
#define WM_MDIMAXIMIZE 0x0225
#endif

#ifndef WM_MDINEXT
#define WM_MDINEXT 0x0224
#endif

#ifndef WM_MDIREFRESHMENU
#define WM_MDIREFRESHMENU 0x0234
#endif

#ifndef WM_MDIRESTORE
#define WM_MDIRESTORE 0x0223
#endif

#ifndef WM_MDISETMENU
#define WM_MDISETMENU 0x0230
#endif

#ifndef WM_MDITILE
#define WM_MDITILE 0x0226
#endif

#ifndef WM_MEASUREITEM
#define WM_MEASUREITEM 0x002c
#endif

#ifndef WM_MENUCHAR
#define WM_MENUCHAR 0x0120
#endif

#ifndef WM_MENUCOMMAND
#define WM_MENUCOMMAND 0x0126
#endif

#ifndef WM_MENUDRAG
#define WM_MENUDRAG 0x0123
#endif

#ifndef WM_MENUGETOBJECT
#define WM_MENUGETOBJECT 0x0124
#endif

#ifndef WM_MENURBUTTONUP
#define WM_MENURBUTTONUP 0x0122
#endif

#ifndef WM_MENUSELECT
#define WM_MENUSELECT 0x011f
#endif

#ifndef WM_MOUSEACTIVATE
#define WM_MOUSEACTIVATE 0x0021
#endif

#ifndef WM_MOUSEFIRST
#define WM_MOUSEFIRST 0x0200
#endif

#ifndef WM_MOUSEHOVER
#define WM_MOUSEHOVER 0x02a1
#endif

#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020e
#endif

#ifndef WM_MOUSELAST
#define WM_MOUSELAST 0x0209
#endif

#ifndef WM_MOUSELAST
#define WM_MOUSELAST 0x020a
#endif

#ifndef WM_MOUSELAST
#define WM_MOUSELAST 0x020d
#endif

#ifndef WM_MOUSELAST
#define WM_MOUSELAST 0x020e
#endif

#ifndef WM_MOUSELEAVE
#define WM_MOUSELEAVE 0x02a3
#endif

#ifndef WM_MOUSEMOVE
#define WM_MOUSEMOVE 0x0200
#endif

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020a
#endif

#ifndef WM_MOVE
#define WM_MOVE 0x0003
#endif

#ifndef WM_MOVING
#define WM_MOVING 0x0216
#endif

#ifndef WM_NCACTIVATE
#define WM_NCACTIVATE 0x0086
#endif

#ifndef WM_NCCALCSIZE
#define WM_NCCALCSIZE 0x0083
#endif

#ifndef WM_NCCREATE
#define WM_NCCREATE 0x0081
#endif

#ifndef WM_NCDESTROY
#define WM_NCDESTROY 0x0082
#endif

#ifndef WM_NCHITTEST
#define WM_NCHITTEST 0x0084
#endif

#ifndef WM_NCLBUTTONDBLCLK
#define WM_NCLBUTTONDBLCLK 0x00a3
#endif

#ifndef WM_NCLBUTTONDOWN
#define WM_NCLBUTTONDOWN 0x00a1
#endif

#ifndef WM_NCLBUTTONUP
#define WM_NCLBUTTONUP 0x00a2
#endif

#ifndef WM_NCMBUTTONDBLCLK
#define WM_NCMBUTTONDBLCLK 0x00a9
#endif

#ifndef WM_NCMBUTTONDOWN
#define WM_NCMBUTTONDOWN 0x00a7
#endif

#ifndef WM_NCMBUTTONUP
#define WM_NCMBUTTONUP 0x00a8
#endif

#ifndef WM_NCMOUSEHOVER
#define WM_NCMOUSEHOVER 0x02a0
#endif

#ifndef WM_NCMOUSELEAVE
#define WM_NCMOUSELEAVE 0x02a2
#endif

#ifndef WM_NCMOUSEMOVE
#define WM_NCMOUSEMOVE 0x00a0
#endif

#ifndef WM_NCPAINT
#define WM_NCPAINT 0x0085
#endif

#ifndef WM_NCPOINTERDOWN
#define WM_NCPOINTERDOWN 0x0242
#endif

#ifndef WM_NCPOINTERUP
#define WM_NCPOINTERUP 0x0243
#endif

#ifndef WM_NCPOINTERUPDATE
#define WM_NCPOINTERUPDATE 0x0241
#endif

#ifndef WM_NCRBUTTONDBLCLK
#define WM_NCRBUTTONDBLCLK 0x00a6
#endif

#ifndef WM_NCRBUTTONDOWN
#define WM_NCRBUTTONDOWN 0x00a4
#endif

#ifndef WM_NCRBUTTONUP
#define WM_NCRBUTTONUP 0x00a5
#endif

#ifndef WM_NCXBUTTONDBLCLK
#define WM_NCXBUTTONDBLCLK 0x00ad
#endif

#ifndef WM_NCXBUTTONDOWN
#define WM_NCXBUTTONDOWN 0x00ab
#endif

#ifndef WM_NCXBUTTONUP
#define WM_NCXBUTTONUP 0x00ac
#endif

#ifndef WM_NEXTDLGCTL
#define WM_NEXTDLGCTL 0x0028
#endif

#ifndef WM_NEXTMENU
#define WM_NEXTMENU 0x0213
#endif

#ifndef WM_NOTIFY
#define WM_NOTIFY 0x004e
#endif

#ifndef WM_NOTIFYFORMAT
#define WM_NOTIFYFORMAT 0x0055
#endif

#ifndef WM_NULL
#define WM_NULL 0x0000
#endif

#ifndef WM_PAINT
#define WM_PAINT 0x000f
#endif

#ifndef WM_PAINTCLIPBOARD
#define WM_PAINTCLIPBOARD 0x0309
#endif

#ifndef WM_PAINTICON
#define WM_PAINTICON 0x0026
#endif

#ifndef WM_PALETTECHANGED
#define WM_PALETTECHANGED 0x0311
#endif

#ifndef WM_PALETTEISCHANGING
#define WM_PALETTEISCHANGING 0x0310
#endif

#ifndef WM_PARENTNOTIFY
#define WM_PARENTNOTIFY 0x0210
#endif

#ifndef WM_PASTE
#define WM_PASTE 0x0302
#endif

#ifndef WM_PENWINFIRST
#define WM_PENWINFIRST 0x0380
#endif

#ifndef WM_PENWINLAST
#define WM_PENWINLAST 0x038f
#endif

#ifndef WM_POINTERACTIVATE
#define WM_POINTERACTIVATE 0x024b
#endif

#ifndef WM_POINTERCAPTURECHANGED
#define WM_POINTERCAPTURECHANGED 0x024c
#endif

#ifndef WM_POINTERDEVICECHANGE
#define WM_POINTERDEVICECHANGE 0x238
#endif

#ifndef WM_POINTERDEVICEINRANGE
#define WM_POINTERDEVICEINRANGE 0x239
#endif

#ifndef WM_POINTERDEVICEOUTOFRANGE
#define WM_POINTERDEVICEOUTOFRANGE 0x23a
#endif

#ifndef WM_POINTERDOWN
#define WM_POINTERDOWN 0x0246
#endif

#ifndef WM_POINTERENTER
#define WM_POINTERENTER 0x0249
#endif

#ifndef WM_POINTERHWHEEL
#define WM_POINTERHWHEEL 0x024f
#endif

#ifndef WM_POINTERLEAVE
#define WM_POINTERLEAVE 0x024a
#endif

#ifndef WM_POINTERROUTEDAWAY
#define WM_POINTERROUTEDAWAY 0x0252
#endif

#ifndef WM_POINTERROUTEDRELEASED
#define WM_POINTERROUTEDRELEASED 0x0253
#endif

#ifndef WM_POINTERROUTEDTO
#define WM_POINTERROUTEDTO 0x0251
#endif

#ifndef WM_POINTERUP
#define WM_POINTERUP 0x0247
#endif

#ifndef WM_POINTERUPDATE
#define WM_POINTERUPDATE 0x0245
#endif

#ifndef WM_POINTERWHEEL
#define WM_POINTERWHEEL 0x024e
#endif

#ifndef WM_POWER
#define WM_POWER 0x0048
#endif

#ifndef WM_POWERBROADCAST
#define WM_POWERBROADCAST 0x0218
#endif

#ifndef WM_PRINT
#define WM_PRINT 0x0317
#endif

#ifndef WM_PRINTCLIENT
#define WM_PRINTCLIENT 0x0318
#endif

#ifndef WM_QUERYDRAGICON
#define WM_QUERYDRAGICON 0x0037
#endif

#ifndef WM_QUERYENDSESSION
#define WM_QUERYENDSESSION 0x0011
#endif

#ifndef WM_QUERYNEWPALETTE
#define WM_QUERYNEWPALETTE 0x030f
#endif

#ifndef WM_QUERYOPEN
#define WM_QUERYOPEN 0x0013
#endif

#ifndef WM_QUERYUISTATE
#define WM_QUERYUISTATE 0x0129
#endif

#ifndef WM_QUEUESYNC
#define WM_QUEUESYNC 0x0023
#endif

#ifndef WM_QUIT
#define WM_QUIT 0x0012
#endif

#ifndef WM_RBUTTONDBLCLK
#define WM_RBUTTONDBLCLK 0x0206
#endif

#ifndef WM_RBUTTONDOWN
#define WM_RBUTTONDOWN 0x0204
#endif

#ifndef WM_RBUTTONUP
#define WM_RBUTTONUP 0x0205
#endif

#ifndef WM_RENDERALLFORMATS
#define WM_RENDERALLFORMATS 0x0306
#endif

#ifndef WM_RENDERFORMAT
#define WM_RENDERFORMAT 0x0305
#endif

#ifndef WM_SETCURSOR
#define WM_SETCURSOR 0x0020
#endif

#ifndef WM_SETFOCUS
#define WM_SETFOCUS 0x0007
#endif

#ifndef WM_SETFONT
#define WM_SETFONT 0x0030
#endif

#ifndef WM_SETHOTKEY
#define WM_SETHOTKEY 0x0032
#endif

#ifndef WM_SETICON
#define WM_SETICON 0x0080
#endif

#ifndef WM_SETREDRAW
#define WM_SETREDRAW 0x000b
#endif

#ifndef WM_SETTEXT
#define WM_SETTEXT 0x000c
#endif

#ifndef WM_SETTINGCHANGE
#define WM_SETTINGCHANGE wm_wininichange
#endif

#ifndef WM_SHOWWINDOW
#define WM_SHOWWINDOW 0x0018
#endif

#ifndef WM_SIZE
#define WM_SIZE 0x0005
#endif

#ifndef WM_SIZECLIPBOARD
#define WM_SIZECLIPBOARD 0x030b
#endif

#ifndef WM_SIZING
#define WM_SIZING 0x0214
#endif

#ifndef WM_SPOOLERSTATUS
#define WM_SPOOLERSTATUS 0x002a
#endif

#ifndef WM_STYLECHANGED
#define WM_STYLECHANGED 0x007d
#endif

#ifndef WM_STYLECHANGING
#define WM_STYLECHANGING 0x007c
#endif

#ifndef WM_SYNCPAINT
#define WM_SYNCPAINT 0x0088
#endif

#ifndef WM_SYSCHAR
#define WM_SYSCHAR 0x0106
#endif

#ifndef WM_SYSCOLORCHANGE
#define WM_SYSCOLORCHANGE 0x0015
#endif

#ifndef WM_SYSCOMMAND
#define WM_SYSCOMMAND 0x0112
#endif

#ifndef WM_SYSDEADCHAR
#define WM_SYSDEADCHAR 0x0107
#endif

#ifndef WM_SYSKEYDOWN
#define WM_SYSKEYDOWN 0x0104
#endif

#ifndef WM_SYSKEYUP
#define WM_SYSKEYUP 0x0105
#endif

#ifndef WM_TABLET_FIRST
#define WM_TABLET_FIRST 0x02c0
#endif

#ifndef WM_TABLET_LAST
#define WM_TABLET_LAST 0x02df
#endif

#ifndef WM_TCARD
#define WM_TCARD 0x0052
#endif

#ifndef WM_THEMECHANGED
#define WM_THEMECHANGED 0x031a
#endif

#ifndef WM_TIMECHANGE
#define WM_TIMECHANGE 0x001e
#endif

#ifndef WM_TIMER
#define WM_TIMER 0x0113
#endif

#ifndef WM_TOOLTIPDISMISS
#define WM_TOOLTIPDISMISS 0x0345
#endif

#ifndef WM_TOUCH
#define WM_TOUCH 0x0240
#endif

#ifndef WM_TOUCHHITTESTING
#define WM_TOUCHHITTESTING 0x024d
#endif

#ifndef WM_UNDO
#define WM_UNDO 0x0304
#endif

#ifndef WM_UNICHAR
#define WM_UNICHAR 0x0109
#endif

#ifndef WM_UNINITMENUPOPUP
#define WM_UNINITMENUPOPUP 0x0125
#endif

#ifndef WM_UPDATEUISTATE
#define WM_UPDATEUISTATE 0x0128
#endif

#ifndef WM_USER
#define WM_USER 0x0400
#endif

#ifndef WM_USERCHANGED
#define WM_USERCHANGED 0x0054
#endif

#ifndef WM_VKEYTOITEM
#define WM_VKEYTOITEM 0x002e
#endif

#ifndef WM_VSCROLL
#define WM_VSCROLL 0x0115
#endif

#ifndef WM_VSCROLLCLIPBOARD
#define WM_VSCROLLCLIPBOARD 0x030a
#endif

#ifndef WM_WINDOWPOSCHANGED
#define WM_WINDOWPOSCHANGED 0x0047
#endif

#ifndef WM_WINDOWPOSCHANGING
#define WM_WINDOWPOSCHANGING 0x0046
#endif

#ifndef WM_WININICHANGE
#define WM_WININICHANGE 0x001a
#endif

#ifndef WM_WTSSESSION_CHANGE
#define WM_WTSSESSION_CHANGE 0x02b1
#endif

#ifndef WM_XBUTTONDBLCLK
#define WM_XBUTTONDBLCLK 0x020d
#endif

#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN 0x020b
#endif

#ifndef WM_XBUTTONUP
#define WM_XBUTTONUP 0x020c
#endif

std::map<DWORD, std::string> wm_map = {
  {WM_ACTIVATE, "activate"},
  {WM_ACTIVATEAPP, "activateapp"},
  {WM_AFXFIRST, "afxfirst"},
  {WM_AFXLAST, "afxlast"},
  {WM_APP, "app"},
  {WM_APPCOMMAND, "appcommand"},
  {WM_ASKCBFORMATNAME, "askcbformatname"},
  {WM_CANCELJOURNAL, "canceljournal"},
  {WM_CANCELMODE, "cancelmode"},
  {WM_CAPTURECHANGED, "capturechanged"},
  {WM_CHANGECBCHAIN, "changecbchain"},
  {WM_CHANGEUISTATE, "changeuistate"},
  {WM_CHAR, "char"},
  {WM_CHARTOITEM, "chartoitem"},
  {WM_CHILDACTIVATE, "childactivate"},
  {WM_CLEAR, "clear"},
  {WM_CLIPBOARDUPDATE, "clipboardupdate"},
  {WM_CLOSE, "close"},
  {WM_COMMAND, "command"},
  {WM_COMMNOTIFY, "commnotify"},
  {WM_COMPACTING, "compacting"},
  {WM_COMPAREITEM, "compareitem"},
  {WM_CONTEXTMENU, "contextmenu"},
  {WM_COPY, "copy"},
  {WM_COPYDATA, "copydata"},
  {WM_CREATE, "create"},
  {WM_CTLCOLORBTN, "ctlcolorbtn"},
  {WM_CTLCOLORDLG, "ctlcolordlg"},
  {WM_CTLCOLOREDIT, "ctlcoloredit"},
  {WM_CTLCOLORLISTBOX, "ctlcolorlistbox"},
  {WM_CTLCOLORMSGBOX, "ctlcolormsgbox"},
  {WM_CTLCOLORSCROLLBAR, "ctlcolorscrollbar"},
  {WM_CTLCOLORSTATIC, "ctlcolorstatic"},
  {WM_CUT, "cut"},
  {WM_DEADCHAR, "deadchar"},
  {WM_DELETEITEM, "deleteitem"},
  {WM_DESTROY, "destroy"},
  {WM_DESTROYCLIPBOARD, "destroyclipboard"},
  {WM_DEVICECHANGE, "devicechange"},
  {WM_DEVMODECHANGE, "devmodechange"},
  {WM_DISPLAYCHANGE, "displaychange"},
  {WM_DPICHANGED, "dpichanged"},
  {WM_DPICHANGED_AFTERPARENT, "dpichanged afterparent"},
  {WM_DPICHANGED_BEFOREPARENT, "dpichanged beforeparent"},
  {WM_DRAWCLIPBOARD, "drawclipboard"},
  {WM_DRAWITEM, "drawitem"},
  {WM_DROPFILES, "dropfiles"},
  {WM_DWMCOLORIZATIONCOLORCHANGED, "dwmcolorizationcolorchanged"},
  {WM_DWMCOMPOSITIONCHANGED, "dwmcompositionchanged"},
  {WM_DWMNCRENDERINGCHANGED, "dwmncrenderingchanged"},
  {WM_DWMSENDICONICLIVEPREVIEWBITMAP, "dwmsendiconiclivepreviewbitmap"},
  {WM_DWMSENDICONICTHUMBNAIL, "dwmsendiconicthumbnail"},
  {WM_DWMWINDOWMAXIMIZEDCHANGE, "dwmwindowmaximizedchange"},
  {WM_ENABLE, "enable"},
  {WM_ENDSESSION, "endsession"},
  {WM_ENTERIDLE, "enteridle"},
  {WM_ENTERMENULOOP, "entermenuloop"},
  {WM_ENTERSIZEMOVE, "entersizemove"},
  {WM_ERASEBKGND, "erasebkgnd"},
  {WM_EXITMENULOOP, "exitmenuloop"},
  {WM_EXITSIZEMOVE, "exitsizemove"},
  {WM_FONTCHANGE, "fontchange"},
  {WM_GESTURE, "gesture"},
  {WM_GESTURENOTIFY, "gesturenotify"},
  {WM_GETDLGCODE, "getdlgcode"},
  {WM_GETDPISCALEDSIZE, "getdpiscaledsize"},
  {WM_GETFONT, "getfont"},
  {WM_GETHOTKEY, "gethotkey"},
  {WM_GETICON, "geticon"},
  {WM_GETMINMAXINFO, "getminmaxinfo"},
  {WM_GETOBJECT, "getobject"},
  {WM_GETTEXT, "gettext"},
  {WM_GETTEXTLENGTH, "gettextlength"},
  {WM_GETTITLEBARINFOEX, "gettitlebarinfoex"},
  {WM_HANDHELDFIRST, "handheldfirst"},
  {WM_HANDHELDLAST, "handheldlast"},
  {WM_HELP, "help"},
  {WM_HOTKEY, "hotkey"},
  {WM_HSCROLL, "hscroll"},
  {WM_HSCROLLCLIPBOARD, "hscrollclipboard"},
  {WM_ICONERASEBKGND, "iconerasebkgnd"},
  {WM_IME_CHAR, "ime char"},
  {WM_IME_COMPOSITION, "ime composition"},
  {WM_IME_COMPOSITIONFULL, "ime compositionfull"},
  {WM_IME_CONTROL, "ime control"},
  {WM_IME_ENDCOMPOSITION, "ime endcomposition"},
  {WM_IME_KEYDOWN, "ime keydown"},
  {WM_IME_KEYLAST, "ime keylast"},
  {WM_IME_KEYUP, "ime keyup"},
  {WM_IME_NOTIFY, "ime notify"},
  {WM_IME_REQUEST, "ime request"},
  {WM_IME_SELECT, "ime select"},
  {WM_IME_SETCONTEXT, "ime setcontext"},
  {WM_IME_STARTCOMPOSITION, "ime startcomposition"},
  {WM_INITDIALOG, "initdialog"},
  {WM_INITMENU, "initmenu"},
  {WM_INITMENUPOPUP, "initmenupopup"},
  {WM_INPUT, "input"},
  {WM_INPUT_DEVICE_CHANGE, "input device change"},
  {WM_INPUTLANGCHANGE, "inputlangchange"},
  {WM_INPUTLANGCHANGEREQUEST, "inputlangchangerequest"},
  {WM_KEYDOWN, "keydown"},
  {WM_KEYFIRST, "keyfirst"},
  {WM_KEYLAST, "keylast"},
  {WM_KEYLAST, "keylast"},
  {WM_KEYUP, "keyup"},
  {WM_KILLFOCUS, "killfocus"},
  {WM_LBUTTONDBLCLK, "lbuttondblclk"},
  {WM_LBUTTONDOWN, "lbuttondown"},
  {WM_LBUTTONUP, "lbuttonup"},
  {WM_MBUTTONDBLCLK, "mbuttondblclk"},
  {WM_MBUTTONDOWN, "mbuttondown"},
  {WM_MBUTTONUP, "mbuttonup"},
  {WM_MDIACTIVATE, "mdiactivate"},
  {WM_MDICASCADE, "mdicascade"},
  {WM_MDICREATE, "mdicreate"},
  {WM_MDIDESTROY, "mdidestroy"},
  {WM_MDIGETACTIVE, "mdigetactive"},
  {WM_MDIICONARRANGE, "mdiiconarrange"},
  {WM_MDIMAXIMIZE, "mdimaximize"},
  {WM_MDINEXT, "mdinext"},
  {WM_MDIREFRESHMENU, "mdirefreshmenu"},
  {WM_MDIRESTORE, "mdirestore"},
  {WM_MDISETMENU, "mdisetmenu"},
  {WM_MDITILE, "mditile"},
  {WM_MEASUREITEM, "measureitem"},
  {WM_MENUCHAR, "menuchar"},
  {WM_MENUCOMMAND, "menucommand"},
  {WM_MENUDRAG, "menudrag"},
  {WM_MENUGETOBJECT, "menugetobject"},
  {WM_MENURBUTTONUP, "menurbuttonup"},
  {WM_MENUSELECT, "menuselect"},
  {WM_MOUSEACTIVATE, "mouseactivate"},
  {WM_MOUSEFIRST, "mousefirst"},
  {WM_MOUSEHOVER, "mousehover"},
  {WM_MOUSEHWHEEL, "mousehwheel"},
  {WM_MOUSELAST, "mouselast"},
  {WM_MOUSELAST, "mouselast"},
  {WM_MOUSELAST, "mouselast"},
  {WM_MOUSELAST, "mouselast"},
  {WM_MOUSELEAVE, "mouseleave"},
  {WM_MOUSEMOVE, "mousemove"},
  {WM_MOUSEWHEEL, "mousewheel"},
  {WM_MOVE, "move"},
  {WM_MOVING, "moving"},
  {WM_NCACTIVATE, "ncactivate"},
  {WM_NCCALCSIZE, "nccalcsize"},
  {WM_NCCREATE, "nccreate"},
  {WM_NCDESTROY, "ncdestroy"},
  {WM_NCHITTEST, "nchittest"},
  {WM_NCLBUTTONDBLCLK, "nclbuttondblclk"},
  {WM_NCLBUTTONDOWN, "nclbuttondown"},
  {WM_NCLBUTTONUP, "nclbuttonup"},
  {WM_NCMBUTTONDBLCLK, "ncmbuttondblclk"},
  {WM_NCMBUTTONDOWN, "ncmbuttondown"},
  {WM_NCMBUTTONUP, "ncmbuttonup"},
  {WM_NCMOUSEHOVER, "ncmousehover"},
  {WM_NCMOUSELEAVE, "ncmouseleave"},
  {WM_NCMOUSEMOVE, "ncmousemove"},
  {WM_NCPAINT, "ncpaint"},
  {WM_NCPOINTERDOWN, "ncpointerdown"},
  {WM_NCPOINTERUP, "ncpointerup"},
  {WM_NCPOINTERUPDATE, "ncpointerupdate"},
  {WM_NCRBUTTONDBLCLK, "ncrbuttondblclk"},
  {WM_NCRBUTTONDOWN, "ncrbuttondown"},
  {WM_NCRBUTTONUP, "ncrbuttonup"},
  {WM_NCXBUTTONDBLCLK, "ncxbuttondblclk"},
  {WM_NCXBUTTONDOWN, "ncxbuttondown"},
  {WM_NCXBUTTONUP, "ncxbuttonup"},
  {WM_NEXTDLGCTL, "nextdlgctl"},
  {WM_NEXTMENU, "nextmenu"},
  {WM_NOTIFY, "notify"},
  {WM_NOTIFYFORMAT, "notifyformat"},
  {WM_NULL, "null"},
  {WM_PAINT, "paint"},
  {WM_PAINTCLIPBOARD, "paintclipboard"},
  {WM_PAINTICON, "painticon"},
  {WM_PALETTECHANGED, "palettechanged"},
  {WM_PALETTEISCHANGING, "paletteischanging"},
  {WM_PARENTNOTIFY, "parentnotify"},
  {WM_PASTE, "paste"},
  {WM_PENWINFIRST, "penwinfirst"},
  {WM_PENWINLAST, "penwinlast"},
  {WM_POINTERACTIVATE, "pointeractivate"},
  {WM_POINTERCAPTURECHANGED, "pointercapturechanged"},
  {WM_POINTERDEVICECHANGE, "pointerdevicechange"},
  {WM_POINTERDEVICEINRANGE, "pointerdeviceinrange"},
  {WM_POINTERDEVICEOUTOFRANGE, "pointerdeviceoutofrange"},
  {WM_POINTERDOWN, "pointerdown"},
  {WM_POINTERENTER, "pointerenter"},
  {WM_POINTERHWHEEL, "pointerhwheel"},
  {WM_POINTERLEAVE, "pointerleave"},
  {WM_POINTERROUTEDAWAY, "pointerroutedaway"},
  {WM_POINTERROUTEDRELEASED, "pointerroutedreleased"},
  {WM_POINTERROUTEDTO, "pointerroutedto"},
  {WM_POINTERUP, "pointerup"},
  {WM_POINTERUPDATE, "pointerupdate"},
  {WM_POINTERWHEEL, "pointerwheel"},
  {WM_POWER, "power"},
  {WM_POWERBROADCAST, "powerbroadcast"},
  {WM_PRINT, "print"},
  {WM_PRINTCLIENT, "printclient"},
  {WM_QUERYDRAGICON, "querydragicon"},
  {WM_QUERYENDSESSION, "queryendsession"},
  {WM_QUERYNEWPALETTE, "querynewpalette"},
  {WM_QUERYOPEN, "queryopen"},
  {WM_QUERYUISTATE, "queryuistate"},
  {WM_QUEUESYNC, "queuesync"},
  {WM_QUIT, "quit"},
  {WM_RBUTTONDBLCLK, "rbuttondblclk"},
  {WM_RBUTTONDOWN, "rbuttondown"},
  {WM_RBUTTONUP, "rbuttonup"},
  {WM_RENDERALLFORMATS, "renderallformats"},
  {WM_RENDERFORMAT, "renderformat"},
  {WM_SETCURSOR, "setcursor"},
  {WM_SETFOCUS, "setfocus"},
  {WM_SETFONT, "setfont"},
  {WM_SETHOTKEY, "sethotkey"},
  {WM_SETICON, "seticon"},
  {WM_SETREDRAW, "setredraw"},
  {WM_SETTEXT, "settext"},
  {WM_SETTINGCHANGE, "settingchange"},
  {WM_SHOWWINDOW, "showwindow"},
  {WM_SIZE, "size"},
  {WM_SIZECLIPBOARD, "sizeclipboard"},
  {WM_SIZING, "sizing"},
  {WM_SPOOLERSTATUS, "spoolerstatus"},
  {WM_STYLECHANGED, "stylechanged"},
  {WM_STYLECHANGING, "stylechanging"},
  {WM_SYNCPAINT, "syncpaint"},
  {WM_SYSCHAR, "syschar"},
  {WM_SYSCOLORCHANGE, "syscolorchange"},
  {WM_SYSCOMMAND, "syscommand"},
  {WM_SYSDEADCHAR, "sysdeadchar"},
  {WM_SYSKEYDOWN, "syskeydown"},
  {WM_SYSKEYUP, "syskeyup"},
  {WM_TABLET_FIRST, "tablet first"},
  {WM_TABLET_LAST, "tablet last"},
  {WM_TCARD, "tcard"},
  {WM_THEMECHANGED, "themechanged"},
  {WM_TIMECHANGE, "timechange"},
  {WM_TIMER, "timer"},
  {WM_TOOLTIPDISMISS, "tooltipdismiss"},
  {WM_TOUCH, "touch"},
  {WM_TOUCHHITTESTING, "touchhittesting"},
  {WM_UNDO, "undo"},
  {WM_UNICHAR, "unichar"},
  {WM_UNINITMENUPOPUP, "uninitmenupopup"},
  {WM_UPDATEUISTATE, "updateuistate"},
  {WM_USER, "user"},
  {WM_USERCHANGED, "userchanged"},
  {WM_VKEYTOITEM, "vkeytoitem"},
  {WM_VSCROLL, "vscroll"},
  {WM_VSCROLLCLIPBOARD, "vscrollclipboard"},
  {WM_WINDOWPOSCHANGED, "windowposchanged"},
  {WM_WINDOWPOSCHANGING, "windowposchanging"},
  {WM_WININICHANGE, "wininichange"},
  {WM_WTSSESSION_CHANGE, "wtssession change"},
  {WM_XBUTTONDBLCLK, "xbuttondblclk"},
  {WM_XBUTTONDOWN, "xbuttondown"},
  {WM_XBUTTONUP, "xbuttonup"},
};

#endif /* WIN_MESSAGE_MAP_H */

