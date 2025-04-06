/*
 * WasmDisplay.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WASM_DISPLAY_H
#define LLGL_WASM_DISPLAY_H


#include <LLGL/Display.h>


namespace LLGL
{


class WasmDisplay : public Display
{

    public:

        WasmDisplay() = default;

        bool IsPrimary() const override;

        UTF8String GetDeviceName() const override;

        Offset2D GetOffset() const override;
        float GetScale() const override;

        bool ResetDisplayMode() override;
        bool SetDisplayMode(const DisplayMode& displayMode) override;
        DisplayMode GetDisplayMode() const override;

        vector<DisplayMode> GetSupportedDisplayModes() const override;

};


} // /namespace LLGL


#endif



// ================================================================================
