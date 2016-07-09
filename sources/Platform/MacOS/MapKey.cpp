/*
 * MapKey.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MapKey.h"


namespace LLGL
{

#if 0
    /*
     *  Summary:
     *    Virtual keycodes
     *
     *  Discussion:
     *    These constants are the virtual keycodes defined originally in
     *    Inside Mac Volume V, pg. V-191. They identify physical keys on a
     *    keyboard. Those constants with "ANSI" in the name are labeled
     *    according to the key position on an ANSI-standard US keyboard.
     *    For example, kVK_ANSI_A indicates the virtual keycode for the key
     *    with the letter 'A' in the US keyboard layout. Other keyboard
     *    layouts may have the 'A' key label on a different physical key;
     *    in this case, pressing 'A' will generate a different virtual
     *    keycode.
     */
    enum {
        kVK_ANSI_A                    = 0x00,
        kVK_ANSI_S                    = 0x01,
        kVK_ANSI_D                    = 0x02,
        kVK_ANSI_F                    = 0x03,
        kVK_ANSI_H                    = 0x04,
        kVK_ANSI_G                    = 0x05,
        kVK_ANSI_Z                    = 0x06,
        kVK_ANSI_X                    = 0x07,
        kVK_ANSI_C                    = 0x08,
        kVK_ANSI_V                    = 0x09,
        kVK_ANSI_B                    = 0x0B,
        kVK_ANSI_Q                    = 0x0C,
        kVK_ANSI_W                    = 0x0D,
        kVK_ANSI_E                    = 0x0E,
        kVK_ANSI_R                    = 0x0F,
        kVK_ANSI_Y                    = 0x10,
        kVK_ANSI_T                    = 0x11,
        kVK_ANSI_1                    = 0x12,
        kVK_ANSI_2                    = 0x13,
        kVK_ANSI_3                    = 0x14,
        kVK_ANSI_4                    = 0x15,
        kVK_ANSI_6                    = 0x16,
        kVK_ANSI_5                    = 0x17,
        kVK_ANSI_Equal                = 0x18,
        kVK_ANSI_9                    = 0x19,
        kVK_ANSI_7                    = 0x1A,
        kVK_ANSI_Minus                = 0x1B,
        kVK_ANSI_8                    = 0x1C,
        kVK_ANSI_0                    = 0x1D,
        kVK_ANSI_RightBracket         = 0x1E,
        kVK_ANSI_O                    = 0x1F,
        kVK_ANSI_U                    = 0x20,
        kVK_ANSI_LeftBracket          = 0x21,
        kVK_ANSI_I                    = 0x22,
        kVK_ANSI_P                    = 0x23,
        kVK_ANSI_L                    = 0x25,
        kVK_ANSI_J                    = 0x26,
        kVK_ANSI_Quote                = 0x27,
        kVK_ANSI_K                    = 0x28,
        kVK_ANSI_Semicolon            = 0x29,
        kVK_ANSI_Backslash            = 0x2A,
        kVK_ANSI_Comma                = 0x2B,
        kVK_ANSI_Slash                = 0x2C,
        kVK_ANSI_N                    = 0x2D,
        kVK_ANSI_M                    = 0x2E,
        kVK_ANSI_Period               = 0x2F,
        kVK_ANSI_Grave                = 0x32,
        kVK_ANSI_KeypadDecimal        = 0x41,
        kVK_ANSI_KeypadMultiply       = 0x43,
        kVK_ANSI_KeypadPlus           = 0x45,
        kVK_ANSI_KeypadClear          = 0x47,
        kVK_ANSI_KeypadDivide         = 0x4B,
        kVK_ANSI_KeypadEnter          = 0x4C,
        kVK_ANSI_KeypadMinus          = 0x4E,
        kVK_ANSI_KeypadEquals         = 0x51,
        kVK_ANSI_Keypad0              = 0x52,
        kVK_ANSI_Keypad1              = 0x53,
        kVK_ANSI_Keypad2              = 0x54,
        kVK_ANSI_Keypad3              = 0x55,
        kVK_ANSI_Keypad4              = 0x56,
        kVK_ANSI_Keypad5              = 0x57,
        kVK_ANSI_Keypad6              = 0x58,
        kVK_ANSI_Keypad7              = 0x59,
        kVK_ANSI_Keypad8              = 0x5B,
        kVK_ANSI_Keypad9              = 0x5C
    };
    
    /* keycodes for keys that are independent of keyboard layout*/
    enum {
        kVK_Return                    = 0x24,
        kVK_Tab                       = 0x30,
        kVK_Space                     = 0x31,
        kVK_Delete                    = 0x33,
        kVK_Escape                    = 0x35,
        kVK_Command                   = 0x37,
        kVK_Shift                     = 0x38,
        kVK_CapsLock                  = 0x39,
        kVK_Option                    = 0x3A,
        kVK_Control                   = 0x3B,
        kVK_RightShift                = 0x3C,
        kVK_RightOption               = 0x3D,
        kVK_RightControl              = 0x3E,
        kVK_Function                  = 0x3F,
        kVK_F17                       = 0x40,
        kVK_VolumeUp                  = 0x48,
        kVK_VolumeDown                = 0x49,
        kVK_Mute                      = 0x4A,
        kVK_F18                       = 0x4F,
        kVK_F19                       = 0x50,
        kVK_F20                       = 0x5A,
        kVK_F5                        = 0x60,
        kVK_F6                        = 0x61,
        kVK_F7                        = 0x62,
        kVK_F3                        = 0x63,
        kVK_F8                        = 0x64,
        kVK_F9                        = 0x65,
        kVK_F11                       = 0x67,
        kVK_F13                       = 0x69,
        kVK_F16                       = 0x6A,
        kVK_F14                       = 0x6B,
        kVK_F10                       = 0x6D,
        kVK_F12                       = 0x6F,
        kVK_F15                       = 0x71,
        kVK_Help                      = 0x72,
        kVK_Home                      = 0x73,
        kVK_PageUp                    = 0x74,
        kVK_ForwardDelete             = 0x75,
        kVK_F4                        = 0x76,
        kVK_End                       = 0x77,
        kVK_F2                        = 0x78,
        kVK_PageDown                  = 0x79,
        kVK_F1                        = 0x7A,
        kVK_LeftArrow                 = 0x7B,
        kVK_RightArrow                = 0x7C,
        kVK_DownArrow                 = 0x7D,
        kVK_UpArrow                   = 0x7E
    };
    
    /* ISO keyboards only*/
    enum {
        kVK_ISO_Section               = 0x0A
    };
    
    /* JIS keyboards only*/
    enum {
        kVK_JIS_Yen                   = 0x5D,
        kVK_JIS_Underscore            = 0x5E,
        kVK_JIS_KeypadComma           = 0x5F,
        kVK_JIS_Eisu                  = 0x66,
        kVK_JIS_Kana                  = 0x68
    };
#endif

#define KEY(c) Key::c
#define DUMMY KEY(Pause) // <-- any key, a dummy will never be used

static Key macOSKeyCodeMap[256] =
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
    
    KEY(Num0                ), // 0x30
    KEY(Num1                ), // 0x31
    KEY(Num2                ), // 0x32
    KEY(Num3                ), // 0x33
    KEY(Num4                ), // 0x34
    KEY(Num5                ), // 0x35
    KEY(Num6                ), // 0x36
    KEY(Num7                ), // 0x37
    KEY(Num8                ), // 0x38
    KEY(Num9                ), // 0x39
    
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
    
    KEY(WinLeft             ), // 0x5b
    KEY(WinRight            ), // 0x5c
    KEY(Apps                ), // 0x5d
    DUMMY,                     // 0x5e
    KEY(Sleep               ), // 0x5f

    KEY(NumPad0             ), // 0x60
    KEY(NumPad1             ), // 0x61
    KEY(NumPad2             ), // 0x62
    KEY(NumPad3             ), // 0x63
    KEY(NumPad4             ), // 0x64
    KEY(NumPad5             ), // 0x65
    KEY(NumPad6             ), // 0x66
    KEY(NumPad7             ), // 0x67
    KEY(NumPad8             ), // 0x68
    KEY(NumPad9             ), // 0x69
    
    KEY(Multiply            ), // 0x6a
    KEY(Add                 ), // 0x6b
    KEY(Separator           ), // 0x6c
    KEY(Subtract            ), // 0x6d
    KEY(Decimal             ), // 0x6e
    KEY(Divide              ), // 0x6f
    
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
    KEY(Scroll              ), // 0x91
    
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


Key MapKey(unsigned short sysKeyCode)
{
    //TODO: remove cast and use correct key mapping for unsigned short!!!
    return macOSKeyCodeMap[(unsigned char)sysKeyCode];
}



} // /namespace LLGL



// ================================================================================
