/*
 * MacOSModule.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
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

        MacOSModule(const std::string& moduleFilename);
        ~MacOSModule();

        MacOSModule(const MacOSModule&) = delete;
        MacOSModule& operator = (const MacOSModule&) = delete;

        void* LoadProcedure(const std::string& procedureName) override;

    private:

        void* handle_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
