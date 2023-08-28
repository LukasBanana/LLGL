/*
 * AndroidModule.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_ANDROID_MODULE_H
#define LLGL_ANDROID_MODULE_H


#include "../Module.h"


namespace LLGL
{


class AndroidModule : public Module
{

    public:

        AndroidModule(const char* moduleFilename, Report* report = nullptr);
        ~AndroidModule();

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