/*
 * MapKey.cpp (Linux)
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MapKey.h"
#include <map>
#include <X11/Xutil.h>


namespace LLGL
{


#define KEYPAIR(KEYSYM, KEY) { KEYSYM, Key::KEY }

static std::map<KeySym, Key> GenerateLinuxKeyCodeMap()
{
    return
    {
        KEYPAIR( XK_BackSpace   , Back            ),
        KEYPAIR( XK_Tab         , Tab             ),
        KEYPAIR( XK_Clear       , Clear           ),
        KEYPAIR( XK_Return      , Return          ),
        KEYPAIR( XK_Menu        , Menu            ),
        KEYPAIR( XK_Pause       , Pause           ),
        KEYPAIR( XK_Caps_Lock   , Capital         ),
    
        KEYPAIR( XK_Escape      , Escape          ),
        KEYPAIR( XK_space       , Space           ),
        KEYPAIR( XK_Page_Up     , PageUp          ),
        KEYPAIR( XK_Page_Down   , PageDown        ),
        KEYPAIR( XK_End         , End             ),
        KEYPAIR( XK_Home        , Home            ),
        KEYPAIR( XK_Left        , Left            ),
        KEYPAIR( XK_Up          , Up              ),
        KEYPAIR( XK_Right       , Right           ),
        KEYPAIR( XK_Down        , Down            ),
        KEYPAIR( XK_Select      , Select          ),
        KEYPAIR( XK_Execute     , Exe             ),
        KEYPAIR( XK_Print       , Snapshot        ),
        KEYPAIR( XK_Insert      , Insert          ),
        KEYPAIR( XK_Delete      , Delete          ),
        KEYPAIR( XK_Help        , Help            ),
    
        KEYPAIR( XK_0           , D0              ),
        KEYPAIR( XK_1           , D1              ),
        KEYPAIR( XK_2           , D2              ),
        KEYPAIR( XK_3           , D3              ),
        KEYPAIR( XK_4           , D4              ),
        KEYPAIR( XK_5           , D5              ),
        KEYPAIR( XK_6           , D6              ),
        KEYPAIR( XK_7           , D7              ),
        KEYPAIR( XK_8           , D8              ),
        KEYPAIR( XK_9           , D9              ),
    
        KEYPAIR( XK_a           , A               ),
        KEYPAIR( XK_b           , B               ),
        KEYPAIR( XK_c           , C               ),
        KEYPAIR( XK_d           , D               ),
        KEYPAIR( XK_e           , E               ),
        KEYPAIR( XK_f           , F               ),
        KEYPAIR( XK_g           , G               ),
        KEYPAIR( XK_h           , H               ),
        KEYPAIR( XK_i           , I               ),
        KEYPAIR( XK_j           , J               ),
        KEYPAIR( XK_k           , K               ),
        KEYPAIR( XK_l           , L               ),
        KEYPAIR( XK_m           , M               ),
        KEYPAIR( XK_n           , N               ),
        KEYPAIR( XK_o           , O               ),
        KEYPAIR( XK_p           , P               ),
        KEYPAIR( XK_q           , Q               ),
        KEYPAIR( XK_r           , R               ),
        KEYPAIR( XK_s           , S               ),
        KEYPAIR( XK_t           , T               ),
        KEYPAIR( XK_u           , U               ),
        KEYPAIR( XK_v           , V               ),
        KEYPAIR( XK_w           , W               ),
        KEYPAIR( XK_x           , X               ),
        KEYPAIR( XK_y           , Y               ),
        KEYPAIR( XK_z           , Z               ),
    
        KEYPAIR( XK_Meta_L      , LWin            ),
        KEYPAIR( XK_Meta_R      , RWin            ),
    
        KEYPAIR( 65438          , Keypad0         ),
        KEYPAIR( 65436          , Keypad1         ),
        KEYPAIR( 65433          , Keypad2         ),
        KEYPAIR( 65435          , Keypad3         ),
        KEYPAIR( 65430          , Keypad4         ),
        KEYPAIR( 65437          , Keypad5         ),
        KEYPAIR( 65432          , Keypad6         ),
        KEYPAIR( 65429          , Keypad7         ),
        KEYPAIR( 65431          , Keypad8         ),
        KEYPAIR( 65434          , Keypad9         ),
    
        KEYPAIR( XK_KP_Multiply , KeypadMultiply  ),
        KEYPAIR( XK_KP_Add      , KeypadPlus      ),
        KEYPAIR( XK_KP_Separator, KeypadSeparator ),
        KEYPAIR( XK_KP_Subtract , KeypadMinus     ),
        KEYPAIR( XK_KP_Decimal  , KeypadDecimal   ),
        KEYPAIR( XK_KP_Divide   , KeypadDivide    ),
    
        KEYPAIR( XK_F1          , F1              ),
        KEYPAIR( XK_F2          , F2              ),
        KEYPAIR( XK_F3          , F3              ),
        KEYPAIR( XK_F4          , F4              ),
        KEYPAIR( XK_F5          , F5              ),
        KEYPAIR( XK_F6          , F6              ),
        KEYPAIR( XK_F7          , F7              ),
        KEYPAIR( XK_F8          , F8              ),
        KEYPAIR( XK_F9          , F9              ),
        KEYPAIR( XK_F10         , F10             ),
        KEYPAIR( XK_F11         , F11             ),
        KEYPAIR( XK_F12         , F12             ),
        KEYPAIR( XK_F13         , F13             ),
        KEYPAIR( XK_F14         , F14             ),
        KEYPAIR( XK_F15         , F15             ),
        KEYPAIR( XK_F16         , F16             ),
        KEYPAIR( XK_F17         , F17             ),
        KEYPAIR( XK_F18         , F18             ),
        KEYPAIR( XK_F19         , F19             ),
        KEYPAIR( XK_F20         , F20             ),
        KEYPAIR( XK_F21         , F21             ),
        KEYPAIR( XK_F22         , F22             ),
        KEYPAIR( XK_F23         , F23             ),
        KEYPAIR( XK_F24         , F24             ),
    
        KEYPAIR( XK_Scroll_Lock , ScrollLock      ),
    
        KEYPAIR( XK_Shift_L     , LShift          ),
        KEYPAIR( XK_Shift_R     , RShift          ),
        KEYPAIR( XK_Control_L   , LControl        ),
        KEYPAIR( XK_Control_R   , RControl        ),
    
        KEYPAIR( XK_plus        , Plus            ),
        KEYPAIR( XK_comma       , Comma           ),
        KEYPAIR( XK_minus       , Minus           ),
        KEYPAIR( XK_period      , Period          ),
    
        KEYPAIR( 94             , Exponent        ),
    };
};

static std::map<KeySym, Key> g_linuxKeyCodeMap = GenerateLinuxKeyCodeMap();

#undef KEYPAIR


Key MapKey(XKeyEvent& keyEvent)
{
    auto keyCode = XLookupKeysym(&keyEvent, 0);
    auto it = g_linuxKeyCodeMap.find(keyCode);
    return (it != g_linuxKeyCodeMap.end() ? it->second : Key::Pause);
}



} // /namespace LLGL



// ================================================================================
