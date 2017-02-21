/*
 * MapKey.cpp (Win32)
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MapKey.h"


namespace LLGL
{


#define KEY(c) Key::c
#define DUMMY KEY(Pause) // <-- any key, a dummy will never be used

static const Key win32KeyCodeMap[256] =
{
    DUMMY,                     // 0x00
    KEY(LButton             ), // 0x01
    KEY(RButton             ), // 0x02
    KEY(Cancel              ), // 0x03
    KEY(MButton             ), // 0x04
    KEY(XButton1            ), // 0x05
    KEY(XButton2            ), // 0x06
    DUMMY,                     // 0x07
    KEY(Back                ), // 0x08
    KEY(Tab                 ), // 0x09
    DUMMY,                     // 0x0a
    DUMMY,                     // 0x0b
    KEY(Clear               ), // 0x0c
    KEY(Return              ), // 0x0d
    DUMMY,                     // 0x0e
    DUMMY,                     // 0x0f
    KEY(Shift               ), // 0x10
    KEY(Control             ), // 0x11
    KEY(Menu                ), // 0x12
    KEY(Pause               ), // 0x13
    KEY(Capital             ), // 0x14
    DUMMY,                     // 0x15
    DUMMY,                     // 0x16
    DUMMY,                     // 0x17
    DUMMY,                     // 0x18
    DUMMY,                     // 0x19
    DUMMY,                     // 0x1a
    KEY(Escape              ), // 0x1b
    DUMMY,                     // 0x1c
    DUMMY,                     // 0x1d
    DUMMY,                     // 0x1e
    DUMMY,                     // 0x1f
    KEY(Space               ), // 0x20
    KEY(PageUp              ), // 0x21
    KEY(PageDown            ), // 0x22
    KEY(End                 ), // 0x23
    KEY(Home                ), // 0x24
    KEY(Left                ), // 0x25
    KEY(Up                  ), // 0x26
    KEY(Right               ), // 0x27
    KEY(Down                ), // 0x28
    KEY(Select              ), // 0x29
    KEY(Print               ), // 0x2a
    KEY(Exe                 ), // 0x2b
    KEY(Snapshot            ), // 0x2c
    KEY(Insert              ), // 0x2d
    KEY(Delete              ), // 0x2e
    KEY(Help                ), // 0x2f
    
    KEY(D0                  ), // 0x30
    KEY(D1                  ), // 0x31
    KEY(D2                  ), // 0x32
    KEY(D3                  ), // 0x33
    KEY(D4                  ), // 0x34
    KEY(D5                  ), // 0x35
    KEY(D6                  ), // 0x36
    KEY(D7                  ), // 0x37
    KEY(D8                  ), // 0x38
    KEY(D9                  ), // 0x39
    
    DUMMY,                     // 0x3a
    DUMMY,                     // 0x3b
    DUMMY,                     // 0x3c
    DUMMY,                     // 0x3d
    DUMMY,                     // 0x3e
    DUMMY,                     // 0x3f
    DUMMY,                     // 0x40

    KEY(A                   ), // 0x41
    KEY(B                   ), // 0x42
    KEY(C                   ), // 0x43
    KEY(D                   ), // 0x44
    KEY(E                   ), // 0x45
    KEY(F                   ), // 0x46
    KEY(G                   ), // 0x47
    KEY(H                   ), // 0x48
    KEY(I                   ), // 0x49
    KEY(J                   ), // 0x4a
    KEY(K                   ), // 0x4b
    KEY(L                   ), // 0x4c
    KEY(M                   ), // 0x4d
    KEY(N                   ), // 0x4e
    KEY(O                   ), // 0x4f
    KEY(P                   ), // 0x50
    KEY(Q                   ), // 0x51
    KEY(R                   ), // 0x52
    KEY(S                   ), // 0x53
    KEY(T                   ), // 0x54
    KEY(U                   ), // 0x55
    KEY(V                   ), // 0x56
    KEY(W                   ), // 0x57
    KEY(X                   ), // 0x58
    KEY(Y                   ), // 0x59
    KEY(Z                   ), // 0x5a
    
    KEY(LWin                ), // 0x5b
    KEY(RWin                ), // 0x5c
    KEY(Apps                ), // 0x5d
    DUMMY,                     // 0x5e
    KEY(Sleep               ), // 0x5f

    KEY(Keypad0             ), // 0x60
    KEY(Keypad1             ), // 0x61
    KEY(Keypad2             ), // 0x62
    KEY(Keypad3             ), // 0x63
    KEY(Keypad4             ), // 0x64
    KEY(Keypad5             ), // 0x65
    KEY(Keypad6             ), // 0x66
    KEY(Keypad7             ), // 0x67
    KEY(Keypad8             ), // 0x68
    KEY(Keypad9             ), // 0x69
    
    KEY(KeypadMultiply      ), // 0x6a
    KEY(KeypadPlus          ), // 0x6b
    KEY(KeypadSeparator     ), // 0x6c
    KEY(KeypadMinus         ), // 0x6d
    KEY(KeypadDecimal       ), // 0x6e
    KEY(KeypadDivide        ), // 0x6f
    
    KEY(F1                  ), // 0x70
    KEY(F2                  ), // 0x71
    KEY(F3                  ), // 0x72
    KEY(F4                  ), // 0x73
    KEY(F5                  ), // 0x74
    KEY(F6                  ), // 0x75
    KEY(F7                  ), // 0x76
    KEY(F8                  ), // 0x77
    KEY(F9                  ), // 0x78
    KEY(F10                 ), // 0x79
    KEY(F11                 ), // 0x7a
    KEY(F12                 ), // 0x7b
    KEY(F13                 ), // 0x7c
    KEY(F14                 ), // 0x7d
    KEY(F15                 ), // 0x7e
    KEY(F16                 ), // 0x7f
    KEY(F17                 ), // 0x80
    KEY(F18                 ), // 0x81
    KEY(F19                 ), // 0x82
    KEY(F20                 ), // 0x83
    KEY(F21                 ), // 0x84
    KEY(F22                 ), // 0x85
    KEY(F23                 ), // 0x86
    KEY(F24                 ), // 0x87
    
    DUMMY,                     // 0x88
    DUMMY,                     // 0x89
    DUMMY,                     // 0x8a
    DUMMY,                     // 0x8b
    DUMMY,                     // 0x8c
    DUMMY,                     // 0x8d
    DUMMY,                     // 0x8e
    DUMMY,                     // 0x8f

    KEY(NumLock             ), // 0x90
    KEY(ScrollLock          ), // 0x91
    
    DUMMY,                     // 0x92
    DUMMY,                     // 0x93
    DUMMY,                     // 0x94
    DUMMY,                     // 0x95
    DUMMY,                     // 0x96
    DUMMY,                     // 0x97
    DUMMY,                     // 0x98
    DUMMY,                     // 0x99
    DUMMY,                     // 0x9a
    DUMMY,                     // 0x9b
    DUMMY,                     // 0x9c
    DUMMY,                     // 0x9d
    DUMMY,                     // 0x9e
    DUMMY,                     // 0x9f
    
    KEY(LShift              ), // 0xa0
    KEY(RShift              ), // 0xa1
    KEY(LControl            ), // 0xa2
    KEY(RControl            ), // 0xa3
    KEY(LMenu               ), // 0xa4
    KEY(RMenu               ), // 0xa5
    
    KEY(BrowserBack         ), // 0xa6
    KEY(BrowserForward      ), // 0xa7
    KEY(BrowserRefresh      ), // 0xa8
    KEY(BrowserStop         ), // 0xa9
    KEY(BrowserSearch       ), // 0xaa
    KEY(BrowserFavorits     ), // 0xab
    KEY(BrowserHome         ), // 0xac

    KEY(VolumeMute          ), // 0xad
    KEY(VolumeDown          ), // 0xae
    KEY(VolumeUp            ), // 0xaf

    KEY(MediaNextTrack      ), // 0xb0
    KEY(MediaPrevTrack      ), // 0xb1
    KEY(MediaStop           ), // 0xb2
    KEY(MediaPlayPause      ), // 0xb3
    
    KEY(LaunchMail          ), // 0xb4
    KEY(LaunchMediaSelect   ), // 0xb5
    KEY(LaunchApp1          ), // 0xb6
    KEY(LaunchApp2          ), // 0xb7

    DUMMY,                     // 0xb8
    DUMMY,                     // 0xb9
    DUMMY,                     // 0xba

    KEY(Plus                ), // 0xbb
    KEY(Comma               ), // 0xbc
    KEY(Minus               ), // 0xbd
    KEY(Period              ), // 0xbe
    
    DUMMY,                     // 0xbf
    DUMMY,                     // 0xc0

    DUMMY,                     // 0xc1
    DUMMY,                     // 0xc2
    DUMMY,                     // 0xc3
    DUMMY,                     // 0xc4
    DUMMY,                     // 0xc5
    DUMMY,                     // 0xc6
    DUMMY,                     // 0xc7
    DUMMY,                     // 0xc8
    DUMMY,                     // 0xc9
    DUMMY,                     // 0xca
    DUMMY,                     // 0xcb
    DUMMY,                     // 0xcc
    DUMMY,                     // 0xcd
    DUMMY,                     // 0xce
    DUMMY,                     // 0xcf
    DUMMY,                     // 0xd0
    DUMMY,                     // 0xd1
    DUMMY,                     // 0xd2
    DUMMY,                     // 0xd3
    DUMMY,                     // 0xd4
    DUMMY,                     // 0xd5
    DUMMY,                     // 0xd6
    DUMMY,                     // 0xd7
    DUMMY,                     // 0xd8
    DUMMY,                     // 0xd9
    DUMMY,                     // 0xda
    DUMMY,                     // 0xdb

    KEY(Exponent            ), // 0xdc
    
    DUMMY,                     // 0xdd
    DUMMY,                     // 0xde
    DUMMY,                     // 0xdf
    DUMMY,                     // 0xe0
    DUMMY,                     // 0xe1
    DUMMY,                     // 0xe2
    DUMMY,                     // 0xe3
    DUMMY,                     // 0xe4
    DUMMY,                     // 0xe5
    DUMMY,                     // 0xe6
    DUMMY,                     // 0xe7
    DUMMY,                     // 0xe8
    DUMMY,                     // 0xe9
    DUMMY,                     // 0xea
    DUMMY,                     // 0xeb
    DUMMY,                     // 0xec
    DUMMY,                     // 0xed
    DUMMY,                     // 0xee
    DUMMY,                     // 0xef
    DUMMY,                     // 0xf0
    DUMMY,                     // 0xf1
    DUMMY,                     // 0xf2
    DUMMY,                     // 0xf3
    DUMMY,                     // 0xf4
    DUMMY,                     // 0xf5

    KEY(Attn                ), // 0xf6
    KEY(CrSel               ), // 0xf7
    KEY(ExSel               ), // 0xf8
    KEY(ErEOF               ), // 0xf9
    KEY(Play                ), // 0xfa
    KEY(Zoom                ), // 0xfb
    KEY(NoName              ), // 0xfc
    KEY(PA1                 ), // 0xfd
    KEY(OEMClear            ), // 0xfe
    DUMMY,                     // 0xff
};

#undef KEY
#undef DUMMY


Key MapKey(unsigned char sysKeyCode)
{
    return win32KeyCodeMap[sysKeyCode];
}



} // /namespace LLGL



// ================================================================================
