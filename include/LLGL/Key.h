/*
 * Key.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_KEY_H__
#define __LLGL_KEY_H__


namespace LLGL
{


//! Input key codes.
enum class Key
{
    LButton,            //!< Left mouse button.
    RButton,            //!< Right mouse button.
    Cancel,             //!< Control-break processing.
    MButton,            //!< Middle mouse button (three-button mouse).
    XButton1,           //!< Windows 2000/XP: X1 mouse button.
    XButton2,           //!< Windows 2000/XP: X2 mouse button.

    Back,               //!< BACKSPACE key.
    Tab,                //!< TAB key.
    Clear,              //!< CLEAR key.
    Return,             //!< RETURN (or ENTER) key.
    Shift,              //!< SHIFT key.
    Control,            //!< CTRL key.

    Menu,               //!< ALT key.
    Pause,              //!< PAUSE key.
    Capital,            //!< CAPS LOCK key.

    Escape,
    Space,
    PageUp,
    PageDown,
    End,                //!< END key.
    Home,               //!< HOME (or POS1) key.
    
    Left,               //!< Left arrow key.
    Up,                 //!< Up arrow key.
    Right,              //!< Right arrow key.
    Down,               //!< Down arrow key.

    Select,
    Print,              //!< Print key.
    Exe,                //!< Execute key.
    Snapshot,
    Insert,
    Delete,
    Help,

    Num0,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,

    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,

    WinLeft,
    WinRight,
    Apps,
    Sleep,

    NumPad0,
    NumPad1,
    NumPad2,
    NumPad3,
    NumPad4,
    NumPad5,
    NumPad6,
    NumPad7,
    NumPad8,
    NumPad9,

    Multiply,
    Add,
    Separator,
    Subtract,
    Decimal,
    Divide,

    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,

    NumLock,
    Scroll,

    LShift,
    RShift,
    LControl,
    RControl,
    LMenu,
    RMenu,

    BrowserBack,
    BrowserForward,
    BrowserRefresh,
    BrowserStop,
    BrowserSearch,
    BrowserFavorits,
    BrowserHome,

    VolumeMute,
    VolumeDown,
    VolumeUp,

    MediaNextTrack,
    MediaPrevTrack,
    MediaStop,
    MediaPlayPause,

    LaunchMail,
    LaunchMediaSelect,
    LaunchApp1,
    LaunchApp2,

    Plus,               //!< '+'
    Comma,              //!< ','
    Minus,              //!< '-'
    Period,             //!< '.'

    Exponent,           //!< '^'

    Attn,
    CrSel,
    ExSel,
    ErEOF,
    Play,
    Zoom,
    NoName,
    PA1,
    OEMClear,
};


} // /namespace LLGL


#endif



// ================================================================================
