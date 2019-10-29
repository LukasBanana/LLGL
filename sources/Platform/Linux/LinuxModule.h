/*
 * LinuxModule.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_LINUX_MODULE_H
#define LLGL_LINUX_MODULE_H


#include "../Module.h"


namespace LLGL
{


class LinuxModule : public Module
{

    public:

        LinuxModule(const char* moduleFilename);
        ~LinuxModule();

        void* LoadProcedure(const char* procedureName) override;

    private:

        void* handle_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
