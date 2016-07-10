/*
 * MapKey.cpp (MacOS)
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MapKey.h"


namespace LLGL
{


#define KEY(c) Key::c
#define DUMMY KEY(Pause) // <-- any key, a dummy will never be used

static Key macOSKeyCodeMap[256] =
{
    KEY(A                   ), // 0x00
    KEY(S                   ), // 0x01
    KEY(D                   ), // 0x02
    KEY(F                   ), // 0x03
    KEY(H                   ), // 0x04
    KEY(G                   ), // 0x05
    KEY(Z                   ), // 0x06
    KEY(X                   ), // 0x07
    KEY(C                   ), // 0x08
    KEY(V                   ), // 0x09
    DUMMY,//KEY(ISO_Section         ), // 0x0a
    KEY(B                   ), // 0x0b
    KEY(Q                   ), // 0x0c
    KEY(W                   ), // 0x0d
    KEY(E                   ), // 0x0e
    KEY(R                   ), // 0x0f
    KEY(Y                   ), // 0x10
    KEY(T                   ), // 0x11
    KEY(Num1                ), // 0x12
    KEY(Num2                ), // 0x13
    KEY(Num3                ), // 0x14
    KEY(Num4                ), // 0x15
    KEY(Num6                ), // 0x16
    KEY(Num5                ), // 0x17
    DUMMY,//KEY(Equal               ), // 0x18
    KEY(Num9                ), // 0x19
    KEY(Num7                ), // 0x1a
    KEY(Minus               ), // 0x1b
    KEY(Num8                ), // 0x1c
    KEY(Num0                ), // 0x1d
    DUMMY,//KEY(RightBracket        ), // 0x1e
    KEY(O                   ), // 0x1f
    KEY(U                   ), // 0x20
    DUMMY,//KEY(LeftBracket         ), // 0x21
    KEY(I                   ), // 0x22
    KEY(P                   ), // 0x23
    KEY(Return              ), // 0x24
    KEY(L                   ), // 0x25
    KEY(J                   ), // 0x26
    DUMMY,//KEY(Quote               ), // 0x27
    KEY(K                   ), // 0x28
    DUMMY,//KEY(Semicolon           ), // 0x29
    DUMMY,//KEY(Backslash           ), // 0x2a
    KEY(Comma               ), // 0x2b
    DUMMY,//KEY(Slash               ), // 0x2c
    KEY(N                   ), // 0x2d
    KEY(M                   ), // 0x2e
    KEY(Period              ), // 0x2f
    KEY(Tab                 ), // 0x30
    KEY(Space               ), // 0x31
    DUMMY,//KEY(Grave               ), // 0x32
    KEY(Back                ), // 0x33
    DUMMY                    , // 0x34
    KEY(Escape              ), // 0x35
    DUMMY                    , // 0x36
    DUMMY,//KEY(Command             ), // 0x37
    KEY(Shift               ), // 0x38
    DUMMY,//KEY(CapsLock            ), // 0x39
    DUMMY,//KEY(Option              ), // 0x3a
    KEY(Control             ), // 0x3b
    DUMMY,//KEY(RightShift          ), // 0x3c
    DUMMY,//KEY(RightOption         ), // 0x3d
    DUMMY,//KEY(RightControl        ), // 0x3e
    DUMMY,//KEY(Function            ), // 0x3f
    KEY(F17                 ), // 0x40
    KEY(Decimal             ), // 0x41
    DUMMY                    , // 0x42
    KEY(Multiply            ), // 0x43
    DUMMY                    , // 0x44
    KEY(Plus                ), // 0x45
    DUMMY                    , // 0x46
    DUMMY,//KEY(KeypadClear         ), // 0x47
    KEY(VolumeUp            ), // 0x48
    KEY(VolumeDown          ), // 0x49
    DUMMY,//KEY(Mute                ), // 0x4a
    KEY(Divide              ), // 0x4b
    KEY(Return              ), // 0x4c
    DUMMY                    , // 0x4d
    KEY(Minus               ), // 0x4e
    KEY(F18                 ), // 0x4f
    KEY(F19                 ), // 0x50
    DUMMY,//KEY(KeypadEquals        ), // 0x51
    KEY(NumPad0             ), // 0x52
    KEY(NumPad1             ), // 0x53
    KEY(NumPad2             ), // 0x54
    KEY(NumPad3             ), // 0x55
    KEY(NumPad4             ), // 0x56
    KEY(NumPad5             ), // 0x57
    KEY(NumPad6             ), // 0x58
    KEY(NumPad7             ), // 0x59
    KEY(F20                 ), // 0x5a
    KEY(NumPad8             ), // 0x5b
    KEY(NumPad9             ), // 0x5c
    DUMMY,//KEY(JIS_Yen             ), // 0x5d
    DUMMY,//KEY(JIS_Underscore      ), // 0x5e
    DUMMY,//KEY(JIS_KeypadComma     ), // 0x5f
    KEY(F5                  ), // 0x60
    KEY(F6                  ), // 0x61
    KEY(F7                  ), // 0x62
    KEY(F3                  ), // 0x63
    KEY(F8                  ), // 0x64
    KEY(F9                  ), // 0x65
    DUMMY,//KEY(JIS_Eisu            ), // 0x66
    KEY(F11                 ), // 0x67
    DUMMY,//KEY(JIS_Kana            ), // 0x68
    KEY(F13                 ), // 0x69
    KEY(F16                 ), // 0x6a
    KEY(F14                 ), // 0x6b
    DUMMY                    , // 0x6c
    KEY(F10                 ), // 0x6d
    DUMMY                    , // 0x6e
    KEY(F12                 ), // 0x6f
    DUMMY                    , // 0x70
    KEY(F15                 ), // 0x71
    KEY(Help                ), // 0x72
    KEY(Home                ), // 0x73
    KEY(PageUp              ), // 0x74
    KEY(Delete              ), // 0x75
    KEY(F4                  ), // 0x76
    KEY(End                 ), // 0x77
    KEY(F2                  ), // 0x78
    KEY(PageDown            ), // 0x79
    KEY(F1                  ), // 0x7a
    KEY(Left                ), // 0x7b
    KEY(Right               ), // 0x7c
    KEY(Down                ), // 0x7d
    KEY(Up                  ), // 0x7e
};

#undef KEY
#undef DUMMY


Key MapKey(unsigned short sysKeyCode)
{
    return (sysKeyCode <= 0x7e ? macOSKeyCodeMap[sysKeyCode] : Key::Pause);
}



} // /namespace LLGL



// ================================================================================
