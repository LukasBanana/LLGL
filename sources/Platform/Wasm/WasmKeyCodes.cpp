/*
 * WasmKeyCodes.cpp
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WasmKeyCodes.h"
#include <map>
#include <emscripten.h>
#include <emscripten/key_codes.h>


namespace LLGL
{


#define KEYPAIR(KEYSYM, KEY) { KEYSYM, Key::KEY }

static std::map<int, Key> GenerateEmscriptenKeyCodeMap()
{
    return
    {
        KEYPAIR(DOM_PK_ESCAPE               , Escape), /* "Escape"             */

        KEYPAIR(DOM_PK_0                    , D0), /* "Digit0"             */
        KEYPAIR(DOM_PK_1                    , D1), /* "Digit1"             */
        KEYPAIR(DOM_PK_2                    , D2), /* "Digit2"             */
        KEYPAIR(DOM_PK_3                    , D3), /* "Digit3"             */
        KEYPAIR(DOM_PK_4                    , D4), /* "Digit4"             */
        KEYPAIR(DOM_PK_5                    , D5), /* "Digit5"             */
        KEYPAIR(DOM_PK_6                    , D6), /* "Digit6"             */
        KEYPAIR(DOM_PK_7                    , D7), /* "Digit7"             */
        KEYPAIR(DOM_PK_8                    , D8), /* "Digit8"             */
        KEYPAIR(DOM_PK_9                    , D9), /* "Digit9"             */

        KEYPAIR(DOM_PK_MINUS                , Minus), /* "Minus"              */
        KEYPAIR(DOM_PK_EQUAL                , Any), /* "Equal"              */
        KEYPAIR(DOM_PK_BACKSPACE            , Back), /* "Backspace"          */
        KEYPAIR(DOM_PK_TAB                  , Tab), /* "Tab"                */

        KEYPAIR(DOM_PK_Q                    , Q), /* "KeyQ"               */
        KEYPAIR(DOM_PK_W                    , W), /* "KeyW"               */
        KEYPAIR(DOM_PK_E                    , E), /* "KeyE"               */
        KEYPAIR(DOM_PK_R                    , R), /* "KeyR"               */
        KEYPAIR(DOM_PK_T                    , T), /* "KeyT"               */
        KEYPAIR(DOM_PK_Y                    , Y), /* "KeyY"               */
        KEYPAIR(DOM_PK_U                    , U), /* "KeyU"               */
        KEYPAIR(DOM_PK_I                    , I), /* "KeyI"               */
        KEYPAIR(DOM_PK_O                    , O), /* "KeyO"               */
        KEYPAIR(DOM_PK_P                    , P), /* "KeyP"               */
        //KEYPAIR(DOM_PK_BRACKET_LEFT         , Any), /* "BracketLeft"        */
        //KEYPAIR(DOM_PK_BRACKET_RIGHT        , Any), /* "BracketRight"       */
        KEYPAIR(DOM_PK_ENTER                , Return), /* "Enter"              */
        KEYPAIR(DOM_PK_CONTROL_LEFT         , LControl), /* "ControlLeft"        */
        KEYPAIR(DOM_PK_A                    , A), /* "KeyA"               */
        KEYPAIR(DOM_PK_S                    , S), /* "KeyS"               */
        KEYPAIR(DOM_PK_D                    , D), /* "KeyD"               */
        KEYPAIR(DOM_PK_F                    , F), /* "KeyF"               */
        KEYPAIR(DOM_PK_G                    , G), /* "KeyG"               */
        KEYPAIR(DOM_PK_H                    , H), /* "KeyH"               */
        KEYPAIR(DOM_PK_J                    , J), /* "KeyJ"               */
        KEYPAIR(DOM_PK_K                    , K), /* "KeyK"               */
        KEYPAIR(DOM_PK_L                    , L), /* "KeyL"               */
        //KEYPAIR(DOM_PK_SEMICOLON            , Any), /* "Semicolon"          */
        //KEYPAIR(DOM_PK_QUOTE                , Any), /* "Quote"              */
        //KEYPAIR(DOM_PK_BACKQUOTE            , Any), /* "Backquote"          */
        KEYPAIR(DOM_PK_SHIFT_LEFT           , LShift), /* "ShiftLeft"          */
        //KEYPAIR(DOM_PK_BACKSLASH            , Any), /* "Backslash"          */
        KEYPAIR(DOM_PK_Z                    , Z), /* "KeyZ"               */
        KEYPAIR(DOM_PK_X                    , X), /* "KeyX"               */
        KEYPAIR(DOM_PK_C                    , C), /* "KeyC"               */
        KEYPAIR(DOM_PK_V                    , V), /* "KeyV"               */
        KEYPAIR(DOM_PK_B                    , B), /* "KeyB"               */
        KEYPAIR(DOM_PK_N                    , N), /* "KeyN"               */
        KEYPAIR(DOM_PK_M                    , M), /* "KeyM"               */
        KEYPAIR(DOM_PK_COMMA                , Comma), /* "Comma"              */
        KEYPAIR(DOM_PK_PERIOD               , Period), /* "Period"             */
        //KEYPAIR(DOM_PK_SLASH                , Any), /* "Slash"              */
        KEYPAIR(DOM_PK_SHIFT_RIGHT          , RShift), /* "ShiftRight"         */
        KEYPAIR(DOM_PK_NUMPAD_MULTIPLY      , KeypadMultiply), /* "NumpadMultiply"     */
        KEYPAIR(DOM_PK_ALT_LEFT             , LMenu), /* "AltLeft"            */
        KEYPAIR(DOM_PK_SPACE                , Space), /* "Space"              */
        KEYPAIR(DOM_PK_CAPS_LOCK            , Capital), /* "CapsLock"           */
        KEYPAIR(DOM_PK_F1                   , F1), /* "F1"                 */
        KEYPAIR(DOM_PK_F2                   , F2), /* "F2"                 */
        KEYPAIR(DOM_PK_F3                   , F3), /* "F3"                 */
        KEYPAIR(DOM_PK_F4                   , F4), /* "F4"                 */
        KEYPAIR(DOM_PK_F5                   , F5), /* "F5"                 */
        KEYPAIR(DOM_PK_F6                   , F6), /* "F6"                 */
        KEYPAIR(DOM_PK_F7                   , F7), /* "F7"                 */
        KEYPAIR(DOM_PK_F8                   , F8), /* "F8"                 */
        KEYPAIR(DOM_PK_F9                   , F9), /* "F9"                 */
        KEYPAIR(DOM_PK_F10                  , F10), /* "F10"                */

        KEYPAIR(DOM_PK_PAUSE                , Pause), /* "Pause"              */
        KEYPAIR(DOM_PK_SCROLL_LOCK          , ScrollLock), /* "ScrollLock"         */
        KEYPAIR(DOM_PK_NUMPAD_7             , Keypad7), /* "Numpad7"            */
        KEYPAIR(DOM_PK_NUMPAD_8             , Keypad8), /* "Numpad8"            */
        KEYPAIR(DOM_PK_NUMPAD_9             , Keypad9), /* "Numpad9"            */
        KEYPAIR(DOM_PK_NUMPAD_SUBTRACT      , KeypadMinus), /* "NumpadSubtract"     */
        KEYPAIR(DOM_PK_NUMPAD_4             , Keypad4), /* "Numpad4"            */
        KEYPAIR(DOM_PK_NUMPAD_5             , Keypad5), /* "Numpad5"            */
        KEYPAIR(DOM_PK_NUMPAD_6             , Keypad6), /* "Numpad6"            */
        KEYPAIR(DOM_PK_NUMPAD_ADD           , KeypadPlus), /* "NumpadAdd"          */
        KEYPAIR(DOM_PK_NUMPAD_1             , Keypad1), /* "Numpad1"            */
        KEYPAIR(DOM_PK_NUMPAD_2             , Keypad2), /* "Numpad2"            */
        KEYPAIR(DOM_PK_NUMPAD_3             , Keypad3), /* "Numpad3"            */
        KEYPAIR(DOM_PK_NUMPAD_0             , Keypad0), /* "Numpad0"            */
        KEYPAIR(DOM_PK_NUMPAD_DECIMAL       , KeypadDecimal), /* "NumpadDecimal"      */
        KEYPAIR(DOM_PK_PRINT_SCREEN         , Print), /* "PrintScreen"        */
        //KEYPAIR(DOM_PK_INTL_BACKSLASH       , Any), /* "IntlBackslash"      */
        KEYPAIR(DOM_PK_F11                  , F11), /* "F11"                */
        KEYPAIR(DOM_PK_F12                  , F12), /* "F12"                */
        //KEYPAIR(DOM_PK_NUMPAD_EQUAL         , Any), /* "NumpadEqual"        */
        KEYPAIR(DOM_PK_F13                  , F13), /* "F13"                */
        KEYPAIR(DOM_PK_F14                  , F14), /* "F14"                */
        KEYPAIR(DOM_PK_F15                  , F15), /* "F15"                */
        KEYPAIR(DOM_PK_F16                  , F16), /* "F16"                */
        KEYPAIR(DOM_PK_F17                  , F17), /* "F17"                */
        KEYPAIR(DOM_PK_F18                  , F18), /* "F18"                */
        KEYPAIR(DOM_PK_F19                  , F19), /* "F19"                */
        KEYPAIR(DOM_PK_F20                  , F20), /* "F20"                */
        KEYPAIR(DOM_PK_F21                  , F21), /* "F21"                */
        KEYPAIR(DOM_PK_F22                  , F22), /* "F22"                */
        KEYPAIR(DOM_PK_F23                  , F23), /* "F23"                */
        //KEYPAIR(DOM_PK_KANA_MODE            , Any), /* "KanaMode"           */
        //KEYPAIR(DOM_PK_LANG_2               , Any), /* "Lang2"              */
        //KEYPAIR(DOM_PK_LANG_1               , Any), /* "Lang1"              */
        //KEYPAIR(DOM_PK_INTL_RO              , Any), /* "IntlRo"             */
        KEYPAIR(DOM_PK_F24                  , F24), /* "F24"                */
        //KEYPAIR(DOM_PK_CONVERT              , Any), /* "Convert"            */
        //KEYPAIR(DOM_PK_NON_CONVERT          , Any), /* "NonConvert"         */
        //KEYPAIR(DOM_PK_INTL_YEN             , Any), /* "IntlYen"            */
        KEYPAIR(DOM_PK_NUMPAD_COMMA         , KeypadDecimal), /* "NumpadComma"        */
        //KEYPAIR(DOM_PK_PASTE                , Any), /* "Paste"              */
        KEYPAIR(DOM_PK_MEDIA_TRACK_PREVIOUS , MediaPrevTrack), /* "MediaTrackPrevious" */
        //KEYPAIR(DOM_PK_CUT                  , Any), /* "Cut"                */
        //KEYPAIR(DOM_PK_COPY                 , Any), /* "Copy"               */
        KEYPAIR(DOM_PK_MEDIA_TRACK_NEXT     , MediaNextTrack), /* "MediaTrackNext"     */

        KEYPAIR(DOM_PK_NUMPAD_ENTER         , Return), /* "NumpadEnter"        */
        KEYPAIR(DOM_PK_CONTROL_RIGHT        , RControl), /* "ControlRight"       */
        KEYPAIR(DOM_PK_AUDIO_VOLUME_MUTE    , VolumeMute), /* "VolumeMute"         */
        KEYPAIR(DOM_PK_LAUNCH_APP_2         , LaunchApp2), /* "LaunchApp2"         */
        KEYPAIR(DOM_PK_MEDIA_PLAY_PAUSE     , MediaPlayPause), /* "MediaPlayPause"     */
        KEYPAIR(DOM_PK_MEDIA_STOP           , MediaStop), /* "MediaStop"          */
        //KEYPAIR(DOM_PK_EJECT                , Any), /* "Eject"              */
        KEYPAIR(DOM_PK_AUDIO_VOLUME_DOWN    , VolumeDown), /* "VolumeDown"         */
        KEYPAIR(DOM_PK_AUDIO_VOLUME_UP      , VolumeUp), /* "VolumeUp"           */
        KEYPAIR(DOM_PK_BROWSER_HOME         , BrowserHome), /* "BrowserHome"        */
        KEYPAIR(DOM_PK_NUMPAD_DIVIDE        , KeypadDivide), /* "NumpadDivide"       */
        KEYPAIR(DOM_PK_ALT_RIGHT            , RMenu), /* "AltRight"           */
        KEYPAIR(DOM_PK_HELP                 , Help), /* "Help"               */
        KEYPAIR(DOM_PK_NUM_LOCK             , NumLock), /* "NumLock"            */
        KEYPAIR(DOM_PK_HOME                 , Home), /* "Home"               */
        KEYPAIR(DOM_PK_ARROW_UP             , Up), /* "ArrowUp"            */
        KEYPAIR(DOM_PK_PAGE_UP              , PageUp), /* "PageUp"             */
        KEYPAIR(DOM_PK_ARROW_LEFT           , Left), /* "ArrowLeft"          */
        KEYPAIR(DOM_PK_ARROW_RIGHT          , Right), /* "ArrowRight"         */
        KEYPAIR(DOM_PK_END                  , End), /* "End"                */
        KEYPAIR(DOM_PK_ARROW_DOWN           , Down), /* "ArrowDown"          */
        KEYPAIR(DOM_PK_PAGE_DOWN            , PageDown), /* "PageDown"           */
        KEYPAIR(DOM_PK_INSERT               , Insert), /* "Insert"             */
        KEYPAIR(DOM_PK_DELETE               , Delete), /* "Delete"             */
        KEYPAIR(DOM_PK_META_LEFT            , LWin), /* "MetaLeft"           */
        //KEYPAIR(DOM_PK_OS_LEFT              , Any), /* "OSLeft"             */
        KEYPAIR(DOM_PK_META_RIGHT           , RWin), /* "MetaRight"          */
        //KEYPAIR(DOM_PK_OS_RIGHT             , Any), /* "OSRight"            */
        KEYPAIR(DOM_PK_CONTEXT_MENU         , Apps), /* "ContextMenu"        */
        //KEYPAIR(DOM_PK_POWER                , Any), /* "Power"              */
        KEYPAIR(DOM_PK_BROWSER_SEARCH       , BrowserSearch), /* "BrowserSearch"      */
        KEYPAIR(DOM_PK_BROWSER_FAVORITES    , BrowserFavorits), /* "BrowserFavorites"   */
        KEYPAIR(DOM_PK_BROWSER_REFRESH      , BrowserRefresh), /* "BrowserRefresh"     */
        KEYPAIR(DOM_PK_BROWSER_STOP         , BrowserStop), /* "BrowserStop"        */
        KEYPAIR(DOM_PK_BROWSER_FORWARD      , BrowserForward), /* "BrowserForward"     */
        KEYPAIR(DOM_PK_BROWSER_BACK         , BrowserBack), /* "BrowserBack"        */
        KEYPAIR(DOM_PK_LAUNCH_APP_1         , LaunchApp1), /* "LaunchApp1"         */
        KEYPAIR(DOM_PK_LAUNCH_MAIL          , LaunchMail), /* "LaunchMail"         */
        //KEYPAIR(DOM_PK_LAUNCH_MEDIA_PLAYER  , Any), /* "LaunchMediaPlayer"  */
        KEYPAIR(DOM_PK_MEDIA_SELECT         , LaunchMediaSelect), /* "MediaSelect"        */        
    };
};

static std::map<int, Key> g_emscriptenWindowKeyCodeMap = GenerateEmscriptenKeyCodeMap();

#undef KEYPAIR


Key MapEmscriptenKeyCode(const char* keyEvent)
{
    int keyCode = emscripten_compute_dom_pk_code(keyEvent);
    auto it = g_emscriptenWindowKeyCodeMap.find(keyCode);
    return (it != g_emscriptenWindowKeyCodeMap.end() ? it->second : Key::Pause);
}



} // /namespace LLGL



// ================================================================================
