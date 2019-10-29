/*
 * MacOSModule.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MACOS_MODULE_H
#define LLGL_MACOS_MODULE_H


#include "../Module.h"


namespace LLGL
{


class MacOSModule : public Module
{

    public:

        MacOSModule(const char* moduleFilename);
        ~MacOSModule();

        void* LoadProcedure(const char* procedureName) override;

    private:

        void* handle_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
