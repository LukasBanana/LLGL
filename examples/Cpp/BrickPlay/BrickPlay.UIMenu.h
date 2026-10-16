/*
 * BrickPlay.UIMenu.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_EXAMPLE_BRICKPLAY_UI_MENU_H
#define LLGL_EXAMPLE_BRICKPLAY_UI_MENU_H


#include "BrickPlay.UIRenderer.h"
#include <LLGL/Utils/ColorRGBA.h>
#include <LLGL/Platform/Platform.h>


// All main menu items for BrickPlay.
enum UIMenuItem
{
    UIMenuItem_Save = 0,        // Download current scene to local machine
    UIMenuItem_Load,            // Upload current scene to public database
    UIMenuItem_Undo,            // Undo previous operation
    UIMenuItem_Redo,            // Redo previously reverted operation
    UIMenuItem_TurnCW,          // Turns the currently selected brick clock-wise (CW)
    UIMenuItem_TurnCCW,         // Turns the currently selected brick counter-clock-wise (CCW)

    #if LLGL_OS_WASM
    UIMenuItem_Share,           // Allow user to share their creation with the world
    #endif

    UIMenuItem_Num,
};

struct UIMenuInfo
{
    int     layer;              // Texture array layer
    int     flags;              // UIRenderFlags
    bool    endsWithSeparator;  // If true, add `uiState.layout.mainMenuSeparatorSize` to the next position
};

inline bool IsPointInsideRect(LLGL::Offset2D point, UIRect rect)
{
    return (point.x >= rect.x && point.x <= rect.x + rect.w && point.y >= rect.y && point.y <= rect.y + rect.h);
}

enum class UIAlign
{
    Left,
    Right,
    Top,
    Bottom,
};

inline bool IsUIAlignVertical(UIAlign align)
{
    return (align == UIAlign::Left || align == UIAlign::Right);
}

inline bool IsUIAlignHorizontal(UIAlign align)
{
    return (align == UIAlign::Top || align == UIAlign::Bottom);
}

struct UILayout
{
    static constexpr int    defaultElementSize  = 32; // 32 for standard displays, 64 for high-resolution displays

    static constexpr int    buttonOffset[2]     = { 15, 15 };
    static constexpr int    buttonFrameSize     = 3;
    static constexpr int    buttonMargin        = 5;

    const LLGL::ColorRGBAub buttonFrameColor    = { 0, 0, 0, 255 };

    const LLGL::ColorRGBAub mainMenuColor[2]    = { { 80, 80, 80 },     // Top color
                                                    { 20, 20, 20 } };   // Bottom color

    const UIMenuInfo menuInfos[UIMenuItem_Num] =
    {
        UIMenuInfo{ 0, 0, false },                      // UIMenuItem_Save
        UIMenuInfo{ 1, 0, true },                       // UIMenuItem_Load
        UIMenuInfo{ 2, 0, false },                      // UIMenuItem_Undo
        UIMenuInfo{ 2, UIRenderFlags_FlipX, true },     // UIMenuItem_Redo
        UIMenuInfo{ 3, 0, false },                      // UIMenuItem_TurnCW

        #if LLGL_OS_WASM
        UIMenuInfo{ 3, UIRenderFlags_FlipX, true },     // UIMenuItem_TurnCCW
        UIMenuInfo{ 4, 0, false },                      // UIMenuItem_Share
        #else
        UIMenuInfo{ 3, UIRenderFlags_FlipX, false },    // UIMenuItem_TurnCCW
        #endif
    };

    int     mainMenuItemSize        = 0;
    int     mainMenuSeparatorSize   = 0;
    int     mainMenuSize            = 0;
    int     mainMenuBaseAdvance     = 0;
    int     colorPickerSize         = 0;
    int     brickThumbnailSize      = 0;
    int     scrollIndicatorSize     = 0;
    float   menuBarTriggerDist      = 0.0f;

    int     thumbstickSize          = 0;
    int     thumbstickBgSize        = 0;
    int     thumbstickOffset        = 0;
    float   thumbstickMaxDrift      = 0.0f;

    int     trashIconSize           = 0;
    int     trashIconOffset         = 0;

    UILayout()
    {
        InitForElementSize(defaultElementSize, 0, 0, 0);
    }

    void InitForElementSize(int uiElementSize, int inThumbstickSize, int inThumbstickBgSize, int inTrashIconSize)
    {
        mainMenuItemSize        = uiElementSize;
        mainMenuSeparatorSize   = uiElementSize/2;
        mainMenuSize            = uiElementSize*3/2;
        mainMenuBaseAdvance     = mainMenuItemSize + UILayout::buttonMargin + UILayout::buttonFrameSize*2;
        colorPickerSize         = (uiElementSize + 1)*8/10; // Size (in pixels) of each color picker button
        brickThumbnailSize      = uiElementSize*3/2;
        menuBarTriggerDist      = static_cast<float>(uiElementSize) * Cfg::uiMenuTriggerDistCoeff;
        scrollIndicatorSize     = uiElementSize/2;

        thumbstickSize          = inThumbstickSize;
        thumbstickBgSize        = inThumbstickBgSize;
        thumbstickOffset        = inThumbstickBgSize*2/3;
        thumbstickMaxDrift      = static_cast<float>(inThumbstickBgSize/2);

        trashIconSize           = inTrashIconSize;
        trashIconOffset         = inTrashIconSize*2/3;
    }
};

// State tracker for UI rendering.
// This is handled primarily in an intermediate mode, but batched draw commands are generated separately from vertex buffer updates to keep them outside render-passes.
struct UIState
{
    static constexpr float scrollBarMass = 0.1f;

    struct MenuBar
    {
        MenuBar()
        {
            scroll.SetMass(scrollBarMass);
        }

        UIAlign             align           = UIAlign::Left; // Alignment of the menu bar
        int                 width           = 0;
        int                 height          = 0;
        float               scrollTarget    = -1.0f; // < 0 means inactive target
        RigidBody<float>    scroll;

        void UpdateScrolling(float dt, int scrW, int scrH)
        {
            const int scrollMax = (IsUIAlignHorizontal(align) ? width - scrW : height - scrH);
            if (scrollMax > 0)
            {
                if (scrollTarget >= 0.0f)
                {
                    // Apply target position
                    scroll.position = Gs::Lerp(scroll.position, scrollTarget, 0.2f * (dt * 60.0f));
                }
                else
                {
                    // Apply forces for smooth scrolling and clamp to screen height
                    scroll.position += scroll.IntegrateForces(dt);
                }

                if (scroll.position < 0.0f)
                {
                    scroll.position = 0.0f;
                    scroll.velocity = 0.0f;
                }
                else if (scroll.position > static_cast<float>(scrollMax))
                {
                    scroll.position = static_cast<float>(scrollMax);
                    scroll.velocity = 0.0f;
                }
            }
            else
            {
                scroll.position = 0.0f;
                scroll.velocity = 0.0f;
            }
        }
    };

    struct Thumbstick
    {
        LLGL::Offset2D  center;
        Gs::Vector2f    drift;
    };

    struct Trash
    {
        LLGL::Offset2D  center;
        unsigned        activePointerId  = 0;
    };

    MenuBar     menuBarColors;
    MenuBar     menuBarBricks;
    MenuBar*    currentMenuBar  = nullptr;
    Trash       trash;

    Thumbstick  thumbstickLeft;
    Thumbstick  thumbstickRight;

    int         scrW            = 0;
    int         scrH            = 0;
    int         x               = 0;
    int         y               = 0;
    int         advance         = 0;
    int         columns         = 0;
    int         mainMenuWidth   = 0;
    bool        isScrollEnabled = false;

    void UpdateScreenSize(LLGL::Extent2D resolution)
    {
        scrW = static_cast<int>(resolution.width);
        scrH = static_cast<int>(resolution.height);

        if (scrW > scrH)
        {
            menuBarColors.align = UIAlign::Left;
            menuBarBricks.align = UIAlign::Right;
        }
        else
        {
            menuBarColors.align = UIAlign::Top;
            menuBarBricks.align = UIAlign::Bottom;
        }
    }

    void ResetToMenuBar(const UILayout& layout, MenuBar& menuBar, int uiElementSize, bool enableScrolling = false)
    {
        isScrollEnabled = enableScrolling;
        currentMenuBar = &menuBar;

        advance = uiElementSize + UILayout::buttonFrameSize*2 + UILayout::buttonMargin;

        const int marginRightOrBottom = UILayout::buttonOffset[0] + uiElementSize + UILayout::buttonFrameSize*2;
        if (IsUIAlignVertical(currentMenuBar->align))
        {
            x = currentMenuBar->align == UIAlign::Right ? scrW - marginRightOrBottom : UILayout::buttonOffset[0];
            y = layout.mainMenuSize + (isScrollEnabled ? UILayout::buttonOffset[1] - static_cast<int>(menuBar.scroll.position) : UILayout::buttonOffset[1]);
        }
        else
        {
            x = (isScrollEnabled ? UILayout::buttonOffset[1] - static_cast<int>(menuBar.scroll.position) : UILayout::buttonOffset[1]);
            y = currentMenuBar->align == UIAlign::Bottom ? scrH - marginRightOrBottom : layout.mainMenuSize + UILayout::buttonOffset[1];
        }

        columns = 1;
    }

    void FinalizeMenuBar(const UILayout& layout, int numElements, int maxColumns)
    {
        LLGL_VERIFY(currentMenuBar != nullptr);

        auto SizeForCells = [this](int axis, int count) -> int
        {
            return UILayout::buttonOffset[axis]*2 - UILayout::buttonMargin + count * advance;
        };

        if (columns > maxColumns)
        {
            const int maxNumElementsPerPage = (numElements / maxColumns + (numElements % maxColumns > 0 ? 1 : 0));

            if (IsUIAlignVertical(currentMenuBar->align))
            {
                currentMenuBar->width   = SizeForCells(0, maxColumns);
                currentMenuBar->height  = advance * maxNumElementsPerPage + layout.mainMenuSize + UILayout::buttonOffset[1]*2 - UILayout::buttonMargin;
            }
            else
            {
                currentMenuBar->width   = advance * maxNumElementsPerPage + UILayout::buttonOffset[1]*2 - UILayout::buttonMargin;
                currentMenuBar->height  = SizeForCells(1, maxColumns);
            }
        }
        else
        {
            if (IsUIAlignVertical(currentMenuBar->align))
            {
                currentMenuBar->width   = SizeForCells(0, columns);
                currentMenuBar->height  = scrH;
            }
            else
            {
                currentMenuBar->width   = scrW;
                currentMenuBar->height  = SizeForCells(1, columns);
            }
        }
    }

    void AdvanceColumn(const UILayout& layout)
    {
        LLGL_VERIFY(currentMenuBar != nullptr);

        const int scroll = static_cast<int>(currentMenuBar->scroll.position);
        if (IsUIAlignVertical(currentMenuBar->align))
        {
            if (y + scroll + advance > (isScrollEnabled ? currentMenuBar->height : scrH))
            {
                // Move to next color picker column
                x += (currentMenuBar->align == UIAlign::Right ? -advance : advance);
                y = layout.mainMenuSize + (isScrollEnabled ? UILayout::buttonOffset[1] - scroll : UILayout::buttonOffset[1]);
                ++columns;
            }
        }
        else
        {
            if (x + scroll + advance > (isScrollEnabled ? currentMenuBar->width : scrW))
            {
                // Move to next color picker column
                x = (isScrollEnabled ? UILayout::buttonOffset[1] - scroll : UILayout::buttonOffset[1]);
                y += (currentMenuBar->align == UIAlign::Bottom ? -advance : advance);
                ++columns;
            }
        }
    }

    void AdvanceRow()
    {
        LLGL_VERIFY(currentMenuBar != nullptr);

        // Move to next UI element position
        if (IsUIAlignVertical(currentMenuBar->align))
            y += advance;
        else
            x += advance;
    }

    bool IsVisible() const
    {
        LLGL_VERIFY(currentMenuBar != nullptr);
        if (IsUIAlignVertical(currentMenuBar->align))
            return y + advance > 0 && y < scrH;
        else
            return x + advance > 0 && x < scrW;
    }
};


#endif // /LLGL_EXAMPLE_BRICKPLAY_UI_RENDERER_H


