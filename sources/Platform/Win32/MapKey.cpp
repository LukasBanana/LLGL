/*
 * MapKey.cpp (Win32)
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MapKey.h"


namespace LLGL
{


static const Key g_win32KeyCodeDummy    = Key::Any;

static const Key g_win32KeyCodeMap[256] =
{
    g_win32KeyCodeDummy,    // 0x00
    Key::LButton,           // 0x01
    Key::RButton,           // 0x02
    Key::Cancel,            // 0x03
    Key::MButton,           // 0x04
    Key::XButton1,          // 0x05
    Key::XButton2,          // 0x06
    g_win32KeyCodeDummy,    // 0x07
    Key::Back,              // 0x08
    Key::Tab,               // 0x09
    g_win32KeyCodeDummy,    // 0x0a
    g_win32KeyCodeDummy,    // 0x0b
    Key::Clear,             // 0x0c
    Key::Return,            // 0x0d
    g_win32KeyCodeDummy,    // 0x0e
    g_win32KeyCodeDummy,    // 0x0f
    Key::Shift,             // 0x10
    Key::Control,           // 0x11
    Key::Menu,              // 0x12
    Key::Pause,             // 0x13
    Key::Capital,           // 0x14
    g_win32KeyCodeDummy,    // 0x15
    g_win32KeyCodeDummy,    // 0x16
    g_win32KeyCodeDummy,    // 0x17
    g_win32KeyCodeDummy,    // 0x18
    g_win32KeyCodeDummy,    // 0x19
    g_win32KeyCodeDummy,    // 0x1a
    Key::Escape,            // 0x1b
    g_win32KeyCodeDummy,    // 0x1c
    g_win32KeyCodeDummy,    // 0x1d
    g_win32KeyCodeDummy,    // 0x1e
    g_win32KeyCodeDummy,    // 0x1f
    Key::Space,             // 0x20
    Key::PageUp,            // 0x21
    Key::PageDown,          // 0x22
    Key::End,               // 0x23
    Key::Home,              // 0x24
    Key::Left,              // 0x25
    Key::Up,                // 0x26
    Key::Right,             // 0x27
    Key::Down,              // 0x28
    Key::Select,            // 0x29
    Key::Print,             // 0x2a
    Key::Exe,               // 0x2b
    Key::Snapshot,          // 0x2c
    Key::Insert,            // 0x2d
    Key::Delete,            // 0x2e
    Key::Help,              // 0x2f

    Key::D0,                // 0x30
    Key::D1,                // 0x31
    Key::D2,                // 0x32
    Key::D3,                // 0x33
    Key::D4,                // 0x34
    Key::D5,                // 0x35
    Key::D6,                // 0x36
    Key::D7,                // 0x37
    Key::D8,                // 0x38
    Key::D9,                // 0x39

    g_win32KeyCodeDummy,    // 0x3a
    g_win32KeyCodeDummy,    // 0x3b
    g_win32KeyCodeDummy,    // 0x3c
    g_win32KeyCodeDummy,    // 0x3d
    g_win32KeyCodeDummy,    // 0x3e
    g_win32KeyCodeDummy,    // 0x3f
    g_win32KeyCodeDummy,    // 0x40

    Key::A,                 // 0x41
    Key::B,                 // 0x42
    Key::C,                 // 0x43
    Key::D,                 // 0x44
    Key::E,                 // 0x45
    Key::F,                 // 0x46
    Key::G,                 // 0x47
    Key::H,                 // 0x48
    Key::I,                 // 0x49
    Key::J,                 // 0x4a
    Key::K,                 // 0x4b
    Key::L,                 // 0x4c
    Key::M,                 // 0x4d
    Key::N,                 // 0x4e
    Key::O,                 // 0x4f
    Key::P,                 // 0x50
    Key::Q,                 // 0x51
    Key::R,                 // 0x52
    Key::S,                 // 0x53
    Key::T,                 // 0x54
    Key::U,                 // 0x55
    Key::V,                 // 0x56
    Key::W,                 // 0x57
    Key::X,                 // 0x58
    Key::Y,                 // 0x59
    Key::Z,                 // 0x5a

    Key::LWin,              // 0x5b
    Key::RWin,              // 0x5c
    Key::Apps,              // 0x5d
    g_win32KeyCodeDummy,    // 0x5e
    Key::Sleep,             // 0x5f

    Key::Keypad0,           // 0x60
    Key::Keypad1,           // 0x61
    Key::Keypad2,           // 0x62
    Key::Keypad3,           // 0x63
    Key::Keypad4,           // 0x64
    Key::Keypad5,           // 0x65
    Key::Keypad6,           // 0x66
    Key::Keypad7,           // 0x67
    Key::Keypad8,           // 0x68
    Key::Keypad9,           // 0x69

    Key::KeypadMultiply,    // 0x6a
    Key::KeypadPlus,        // 0x6b
    Key::KeypadSeparator,   // 0x6c
    Key::KeypadMinus,       // 0x6d
    Key::KeypadDecimal,     // 0x6e
    Key::KeypadDivide,      // 0x6f

    Key::F1,                // 0x70
    Key::F2,                // 0x71
    Key::F3,                // 0x72
    Key::F4,                // 0x73
    Key::F5,                // 0x74
    Key::F6,                // 0x75
    Key::F7,                // 0x76
    Key::F8,                // 0x77
    Key::F9,                // 0x78
    Key::F10,               // 0x79
    Key::F11,               // 0x7a
    Key::F12,               // 0x7b
    Key::F13,               // 0x7c
    Key::F14,               // 0x7d
    Key::F15,               // 0x7e
    Key::F16,               // 0x7f
    Key::F17,               // 0x80
    Key::F18,               // 0x81
    Key::F19,               // 0x82
    Key::F20,               // 0x83
    Key::F21,               // 0x84
    Key::F22,               // 0x85
    Key::F23,               // 0x86
    Key::F24,               // 0x87

    g_win32KeyCodeDummy,    // 0x88
    g_win32KeyCodeDummy,    // 0x89
    g_win32KeyCodeDummy,    // 0x8a
    g_win32KeyCodeDummy,    // 0x8b
    g_win32KeyCodeDummy,    // 0x8c
    g_win32KeyCodeDummy,    // 0x8d
    g_win32KeyCodeDummy,    // 0x8e
    g_win32KeyCodeDummy,    // 0x8f

    Key::NumLock,           // 0x90
    Key::ScrollLock,        // 0x91

    g_win32KeyCodeDummy,    // 0x92
    g_win32KeyCodeDummy,    // 0x93
    g_win32KeyCodeDummy,    // 0x94
    g_win32KeyCodeDummy,    // 0x95
    g_win32KeyCodeDummy,    // 0x96
    g_win32KeyCodeDummy,    // 0x97
    g_win32KeyCodeDummy,    // 0x98
    g_win32KeyCodeDummy,    // 0x99
    g_win32KeyCodeDummy,    // 0x9a
    g_win32KeyCodeDummy,    // 0x9b
    g_win32KeyCodeDummy,    // 0x9c
    g_win32KeyCodeDummy,    // 0x9d
    g_win32KeyCodeDummy,    // 0x9e
    g_win32KeyCodeDummy,    // 0x9f

    Key::LShift,            // 0xa0
    Key::RShift,            // 0xa1
    Key::LControl,          // 0xa2
    Key::RControl,          // 0xa3
    Key::LMenu,             // 0xa4
    Key::RMenu,             // 0xa5

    Key::BrowserBack,       // 0xa6
    Key::BrowserForward,    // 0xa7
    Key::BrowserRefresh,    // 0xa8
    Key::BrowserStop,       // 0xa9
    Key::BrowserSearch,     // 0xaa
    Key::BrowserFavorits,   // 0xab
    Key::BrowserHome,       // 0xac

    Key::VolumeMute,        // 0xad
    Key::VolumeDown,        // 0xae
    Key::VolumeUp,          // 0xaf

    Key::MediaNextTrack,    // 0xb0
    Key::MediaPrevTrack,    // 0xb1
    Key::MediaStop,         // 0xb2
    Key::MediaPlayPause,    // 0xb3

    Key::LaunchMail,        // 0xb4
    Key::LaunchMediaSelect, // 0xb5
    Key::LaunchApp1,        // 0xb6
    Key::LaunchApp2,        // 0xb7

    g_win32KeyCodeDummy,    // 0xb8
    g_win32KeyCodeDummy,    // 0xb9
    g_win32KeyCodeDummy,    // 0xba

    Key::Plus,              // 0xbb
    Key::Comma,             // 0xbc
    Key::Minus,             // 0xbd
    Key::Period,            // 0xbe

    g_win32KeyCodeDummy,    // 0xbf
    g_win32KeyCodeDummy,    // 0xc0

    g_win32KeyCodeDummy,    // 0xc1
    g_win32KeyCodeDummy,    // 0xc2
    g_win32KeyCodeDummy,    // 0xc3
    g_win32KeyCodeDummy,    // 0xc4
    g_win32KeyCodeDummy,    // 0xc5
    g_win32KeyCodeDummy,    // 0xc6
    g_win32KeyCodeDummy,    // 0xc7
    g_win32KeyCodeDummy,    // 0xc8
    g_win32KeyCodeDummy,    // 0xc9
    g_win32KeyCodeDummy,    // 0xca
    g_win32KeyCodeDummy,    // 0xcb
    g_win32KeyCodeDummy,    // 0xcc
    g_win32KeyCodeDummy,    // 0xcd
    g_win32KeyCodeDummy,    // 0xce
    g_win32KeyCodeDummy,    // 0xcf
    g_win32KeyCodeDummy,    // 0xd0
    g_win32KeyCodeDummy,    // 0xd1
    g_win32KeyCodeDummy,    // 0xd2
    g_win32KeyCodeDummy,    // 0xd3
    g_win32KeyCodeDummy,    // 0xd4
    g_win32KeyCodeDummy,    // 0xd5
    g_win32KeyCodeDummy,    // 0xd6
    g_win32KeyCodeDummy,    // 0xd7
    g_win32KeyCodeDummy,    // 0xd8
    g_win32KeyCodeDummy,    // 0xd9
    g_win32KeyCodeDummy,    // 0xda
    g_win32KeyCodeDummy,    // 0xdb

    Key::Exponent,          // 0xdc

    g_win32KeyCodeDummy,    // 0xdd
    g_win32KeyCodeDummy,    // 0xde
    g_win32KeyCodeDummy,    // 0xdf
    g_win32KeyCodeDummy,    // 0xe0
    g_win32KeyCodeDummy,    // 0xe1
    g_win32KeyCodeDummy,    // 0xe2
    g_win32KeyCodeDummy,    // 0xe3
    g_win32KeyCodeDummy,    // 0xe4
    g_win32KeyCodeDummy,    // 0xe5
    g_win32KeyCodeDummy,    // 0xe6
    g_win32KeyCodeDummy,    // 0xe7
    g_win32KeyCodeDummy,    // 0xe8
    g_win32KeyCodeDummy,    // 0xe9
    g_win32KeyCodeDummy,    // 0xea
    g_win32KeyCodeDummy,    // 0xeb
    g_win32KeyCodeDummy,    // 0xec
    g_win32KeyCodeDummy,    // 0xed
    g_win32KeyCodeDummy,    // 0xee
    g_win32KeyCodeDummy,    // 0xef
    g_win32KeyCodeDummy,    // 0xf0
    g_win32KeyCodeDummy,    // 0xf1
    g_win32KeyCodeDummy,    // 0xf2
    g_win32KeyCodeDummy,    // 0xf3
    g_win32KeyCodeDummy,    // 0xf4
    g_win32KeyCodeDummy,    // 0xf5

    Key::Attn,              // 0xf6
    Key::CrSel,             // 0xf7
    Key::ExSel,             // 0xf8
    Key::ErEOF,             // 0xf9
    Key::Play,              // 0xfa
    Key::Zoom,              // 0xfb
    Key::NoName,            // 0xfc
    Key::PA1,               // 0xfd
    Key::OEMClear,          // 0xfe
    g_win32KeyCodeDummy,    // 0xff
};


Key MapKey(std::uint8_t sysKeyCode)
{
    return g_win32KeyCodeMap[sysKeyCode];
}



} // /namespace LLGL



// ================================================================================
