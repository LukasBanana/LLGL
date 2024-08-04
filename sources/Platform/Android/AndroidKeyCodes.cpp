/*
 * AndroidKeyCodes.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "AndroidKeyCodes.h"
#include <android/input.h>


namespace LLGL
{


Key MapAndroidKeyCode(std::int32_t keycode)
{
    switch (keycode)
    {
        //case :                        return Key::LButton;
        //case :                        return Key::RButton;
        //case :                        return Key::Cancel;
        //case :                        return Key::MButton;
        //case :                        return Key::XButton1;
        //case :                        return Key::XButton2;

        //case :                        return Key::Back;
        //case :                        return Key::Tab;
        //case :                        return Key::Clear;
        //case :                        return Key::Return;
        //case :                        return Key::Shift;
        //case :                        return Key::Control;

        case AKEYCODE_MENU:             return Key::Menu;
        //case :                        return Key::Pause;
        //case :                        return Key::Capital;

        case AKEYCODE_ESCAPE:           return Key::Escape;
        case AKEYCODE_SPACE:            return Key::Space;
        case AKEYCODE_PAGE_UP:          return Key::PageUp;
        case AKEYCODE_PAGE_DOWN:        return Key::PageDown;
        //case :                        return Key::End;
        //case :                        return Key::Home;

        case AKEYCODE_DPAD_LEFT:        return Key::Left;
        case AKEYCODE_DPAD_UP:          return Key::Up;
        case AKEYCODE_DPAD_RIGHT:       return Key::Right;
        case AKEYCODE_DPAD_DOWN:        return Key::Down;

        //case :                        return Key::Select;
        //case :                        return Key::Print;
        //case :                        return Key::Exe;
        //case :                        return Key::Snapshot;
        //case :                        return Key::Insert;
        //case :                        return Key::Delete;
        //case :                        return Key::Help;

        case AKEYCODE_0:                return Key::D0;
        case AKEYCODE_1:                return Key::D1;
        case AKEYCODE_2:                return Key::D2;
        case AKEYCODE_3:                return Key::D3;
        case AKEYCODE_4:                return Key::D4;
        case AKEYCODE_5:                return Key::D5;
        case AKEYCODE_6:                return Key::D6;
        case AKEYCODE_7:                return Key::D7;
        case AKEYCODE_8:                return Key::D8;
        case AKEYCODE_9:                return Key::D9;

        case AKEYCODE_A:                return Key::A;
        case AKEYCODE_B:                return Key::B;
        case AKEYCODE_C:                return Key::C;
        case AKEYCODE_D:                return Key::D;
        case AKEYCODE_E:                return Key::E;
        case AKEYCODE_F:                return Key::F;
        case AKEYCODE_G:                return Key::G;
        case AKEYCODE_H:                return Key::H;
        case AKEYCODE_I:                return Key::I;
        case AKEYCODE_J:                return Key::J;
        case AKEYCODE_K:                return Key::K;
        case AKEYCODE_L:                return Key::L;
        case AKEYCODE_M:                return Key::M;
        case AKEYCODE_N:                return Key::N;
        case AKEYCODE_O:                return Key::O;
        case AKEYCODE_P:                return Key::P;
        case AKEYCODE_Q:                return Key::Q;
        case AKEYCODE_R:                return Key::R;
        case AKEYCODE_S:                return Key::S;
        case AKEYCODE_T:                return Key::T;
        case AKEYCODE_U:                return Key::U;
        case AKEYCODE_V:                return Key::V;
        case AKEYCODE_W:                return Key::W;
        case AKEYCODE_X:                return Key::X;
        case AKEYCODE_Y:                return Key::Y;
        case AKEYCODE_Z:                return Key::Z;

        //case :                        return Key::LWin;
        //case :                        return Key::RWin;
        //case :                        return Key::Apps;
        //case :                        return Key::Sleep;

        case AKEYCODE_NUMPAD_0:         return Key::Keypad0;
        case AKEYCODE_NUMPAD_1:         return Key::Keypad1;
        case AKEYCODE_NUMPAD_2:         return Key::Keypad2;
        case AKEYCODE_NUMPAD_3:         return Key::Keypad3;
        case AKEYCODE_NUMPAD_4:         return Key::Keypad4;
        case AKEYCODE_NUMPAD_5:         return Key::Keypad5;
        case AKEYCODE_NUMPAD_6:         return Key::Keypad6;
        case AKEYCODE_NUMPAD_7:         return Key::Keypad7;
        case AKEYCODE_NUMPAD_8:         return Key::Keypad8;
        case AKEYCODE_NUMPAD_9:         return Key::Keypad9;

        case AKEYCODE_NUMPAD_MULTIPLY:  return Key::KeypadMultiply;
        case AKEYCODE_NUMPAD_ADD:       return Key::KeypadPlus;
        case AKEYCODE_NUMPAD_ENTER:     return Key::KeypadSeparator; // TODO: rename Key::KeypadSeparator
        case AKEYCODE_NUMPAD_SUBTRACT:  return Key::KeypadMinus;
        case AKEYCODE_NUMPAD_COMMA:     return Key::KeypadDecimal; // Both map to Key::KeypadDecimal
        case AKEYCODE_NUMPAD_DOT:       return Key::KeypadDecimal; // Both map to Key::KeypadDecimal
        case AKEYCODE_NUMPAD_DIVIDE:    return Key::KeypadDivide;

        case AKEYCODE_F1:               return Key::F1;
        case AKEYCODE_F2:               return Key::F2;
        case AKEYCODE_F3:               return Key::F3;
        case AKEYCODE_F4:               return Key::F4;
        case AKEYCODE_F5:               return Key::F5;
        case AKEYCODE_F6:               return Key::F6;
        case AKEYCODE_F7:               return Key::F7;
        case AKEYCODE_F8:               return Key::F8;
        case AKEYCODE_F9:               return Key::F9;
        case AKEYCODE_F10:              return Key::F10;
        case AKEYCODE_F11:              return Key::F11;
        case AKEYCODE_F12:              return Key::F12;
        //case :                        return Key::F13;
        //case :                        return Key::F14;
        //case :                        return Key::F15;
        //case :                        return Key::F16;
        //case :                        return Key::F17;
        //case :                        return Key::F18;
        //case :                        return Key::F19;
        //case :                        return Key::F20;
        //case :                        return Key::F21;
        //case :                        return Key::F22;
        //case :                        return Key::F23;
        //case :                        return Key::F24;

        case AKEYCODE_NUM_LOCK:         return Key::NumLock;
        //case :                        return Key::ScrollLock;

        case AKEYCODE_SHIFT_LEFT:       return Key::LShift;
        case AKEYCODE_SHIFT_RIGHT:      return Key::RShift;
        case AKEYCODE_CTRL_LEFT:        return Key::LControl;
        case AKEYCODE_CTRL_RIGHT:       return Key::RControl;
        case AKEYCODE_ALT_LEFT:         return Key::LMenu;
        case AKEYCODE_ALT_RIGHT:        return Key::RMenu;

        case AKEYCODE_BACK:             return Key::BrowserBack;
        case AKEYCODE_FORWARD:          return Key::BrowserForward;
        case AKEYCODE_REFRESH:          return Key::BrowserRefresh;
        //case :                        return Key::BrowserStop;
        case AKEYCODE_SEARCH:           return Key::BrowserSearch;
        //case :                        return Key::BrowserFavorits;
        case AKEYCODE_HOME:             return Key::BrowserHome;

        case AKEYCODE_VOLUME_MUTE:      return Key::VolumeMute;
        case AKEYCODE_VOLUME_DOWN:      return Key::VolumeDown;
        case AKEYCODE_VOLUME_UP:        return Key::VolumeUp;

        case AKEYCODE_MEDIA_NEXT:       return Key::MediaNextTrack;
        case AKEYCODE_MEDIA_PREVIOUS:   return Key::MediaPrevTrack;
        case AKEYCODE_MEDIA_STOP:       return Key::MediaStop;
        case AKEYCODE_MEDIA_PLAY_PAUSE: return Key::MediaPlayPause;

        //case :                        return Key::LaunchMail;
        //case :                        return Key::LaunchMediaSelect;
        //case :                        return Key::LaunchApp1;
        //case :                        return Key::LaunchApp2;

        case AKEYCODE_PLUS:             return Key::Plus;
        case AKEYCODE_COMMA:            return Key::Comma;
        case AKEYCODE_MINUS:            return Key::Minus;
        case AKEYCODE_PERIOD:           return Key::Period;

        //case :                        return Key::Exponent;

        //case :                        return Key::Attn;
        //case :                        return Key::CrSel;
        //case :                        return Key::ExSel;
        //case :                        return Key::ErEOF;
        //case :                        return Key::Play;
        //case :                        return Key::Zoom;
        //case :                        return Key::NoName;
        //case :                        return Key::PA1;
        //case :                        return Key::OEMClear;

        default:                        return Key::Any;
    }
}



} // /namespace LLGL



// ================================================================================
