/*
 * DXReport.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DX_REPORT_H
#define LLGL_DX_REPORT_H


#include <LLGL/Report.h>
#include <LLGL/Container/StringView.h>
#include <d3dcommon.h>
#include "ComPtr.h"
#include <string>


namespace LLGL
{


class DXReport final : public Report
{

    public:

        // Resets the report.
        void Reset(const StringView& text, bool hasErrors = false);
        void Reset(ID3DBlob* blob, bool hasErrors = false);

        const char* GetText() const override;
        bool HasErrors() const override;

    public:

        // Returns whether this is a valid report. Otherwise, there is no text and no errors.
        inline operator bool() const
        {
            return (!text_.empty() || hasErrors_);
        }

    private:

        std::string text_;
        bool        hasErrors_  = false;

};


} // /namespace LLGL


#endif



// ================================================================================
