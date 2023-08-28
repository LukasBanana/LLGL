/*
 * MacOSModule.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MACOS_MODULE_H
#define LLGL_MACOS_MODULE_H


#include "../Module.h"


namespace LLGL
{


class MacOSModule : public Module
{

    public:

        MacOSModule(const char* moduleFilename, Report* report = nullptr);
        ~MacOSModule();

        void* LoadProcedure(const char* procedureName) override;

    public:

        inline bool IsValid() const
        {
            return (handle_ != nullptr);
        }

    private:

        void* handle_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
