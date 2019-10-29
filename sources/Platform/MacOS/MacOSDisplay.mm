/*
 * MacOSDisplay.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MacOSDisplay.h"
#include "../../Core/Helper.h"


namespace LLGL
{


/*
 * Internal functions
 */

// Converts a CGDisplayMode to a descriptor structure
static void Convert(DisplayModeDescriptor& dst, CGDisplayModeRef src)
{
    dst.resolution.width    = static_cast<std::uint32_t>(CGDisplayModeGetWidth(src));
    dst.resolution.height   = static_cast<std::uint32_t>(CGDisplayModeGetHeight(src));
    dst.refreshRate         = static_cast<std::uint32_t>(CGDisplayModeGetRefreshRate(src));
}

// Returns true if the specified descriptor matches the display mode
static bool MatchDisplayMode(const DisplayModeDescriptor& displayModeDesc, CGDisplayModeRef modeRef)
{
    return
    (
        static_cast<std::size_t>(displayModeDesc.resolution.width ) == CGDisplayModeGetWidth (modeRef) &&
        static_cast<std::size_t>(displayModeDesc.resolution.height) == CGDisplayModeGetHeight(modeRef)
    );
}


/*
 * Display class
 */

std::vector<std::unique_ptr<Display>> Display::InstantiateList()
{
    static const std::uint32_t maxNumDisplays = 16;

    CGDirectDisplayID displayIDArray[maxNumDisplays] = {};
    std::uint32_t numDisplays = 0;

    std::vector<std::unique_ptr<Display>> displayList;

    if (CGGetActiveDisplayList(maxNumDisplays, displayIDArray, &numDisplays) == kCGErrorSuccess)
    {
        for (std::uint32_t i = 0; i < numDisplays; ++i)
            displayList.emplace_back(MakeUnique<MacOSDisplay>(displayIDArray[i]));
    }

    return displayList;
}

std::unique_ptr<Display> Display::InstantiatePrimary()
{
    return MakeUnique<MacOSDisplay>(CGMainDisplayID());
}

static bool g_cursorVisible = true;

bool Display::ShowCursor(bool show)
{
    if (g_cursorVisible != show)
    {
        if (show)
            [NSCursor unhide];
        else
            [NSCursor hide];
        g_cursorVisible = show;
    }
    return true;
}

bool Display::IsCursorShown()
{
    return g_cursorVisible;
}


/*
 * MacOSDisplay class
 */

MacOSDisplay::MacOSDisplay(CGDirectDisplayID displayID) :
    displayID_             { displayID                            },
    defaultDisplayModeRef_ { CGDisplayCopyDisplayMode(displayID_) }
{
}

MacOSDisplay::~MacOSDisplay()
{
    CGDisplayModeRelease(defaultDisplayModeRef_);
}

bool MacOSDisplay::IsPrimary() const
{
    return (CGDisplayIsMain(displayID_) != 0);
}

std::wstring MacOSDisplay::GetDeviceName() const
{
    //TODO
    return L"";
}

Offset2D MacOSDisplay::GetOffset() const
{
    CGRect rc = CGDisplayBounds(displayID_);
    return Offset2D
    {
        static_cast<std::int32_t>(rc.origin.x),
        static_cast<std::int32_t>(rc.origin.y)
    };
}

bool MacOSDisplay::ResetDisplayMode()
{
    return (CGDisplaySetDisplayMode(displayID_, defaultDisplayModeRef_, nullptr) == kCGErrorSuccess);
}

bool MacOSDisplay::SetDisplayMode(const DisplayModeDescriptor& displayModeDesc)
{
    bool result = false;

    CFArrayRef modeArrayRef = CGDisplayCopyAllDisplayModes(displayID_, nullptr);

    for (CFIndex i = 0, n = CFArrayGetCount(modeArrayRef); i < n; ++i)
    {
        /* Check if current display mode matches the input descriptor */
        CGDisplayModeRef modeRef = (CGDisplayModeRef)CFArrayGetValueAtIndex(modeArrayRef, i);

        if (MatchDisplayMode(displayModeDesc, modeRef))
        {
            result = (CGDisplaySetDisplayMode(displayID_, modeRef, nullptr) == kCGErrorSuccess);
            break;
        }
    }

    CFRelease(modeArrayRef);

    return result;
}

DisplayModeDescriptor MacOSDisplay::GetDisplayMode() const
{
    DisplayModeDescriptor displayModeDesc;
    {
        CGDisplayModeRef modeRef = CGDisplayCopyDisplayMode(displayID_);
        Convert(displayModeDesc, modeRef);
        CGDisplayModeRelease(modeRef);
    }
    return displayModeDesc;
}

std::vector<DisplayModeDescriptor> MacOSDisplay::GetSupportedDisplayModes() const
{
    std::vector<DisplayModeDescriptor> displayModeDescs;

    CFArrayRef modeArrayRef = CGDisplayCopyAllDisplayModes(displayID_, nullptr);

    for (CFIndex i = 0, n = CFArrayGetCount(modeArrayRef); i < n; ++i)
    {
        DisplayModeDescriptor modeDesc;
        CGDisplayModeRef modeRef = (CGDisplayModeRef)CFArrayGetValueAtIndex(modeArrayRef, i);
        Convert(modeDesc, modeRef);
        displayModeDescs.push_back(modeDesc);
    }

    CFRelease(modeArrayRef);

    /* Sort final display mode list and remove duplciate entries */
    FinalizeDisplayModes(displayModeDescs);

    return displayModeDescs;
}


} // /namespace LLGL



// ================================================================================
