/*
 * IOSModule.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_IOS_MODULE_H
#define LLGL_IOS_MODULE_H


#include "../Module.h"


namespace LLGL
{


class IOSModule : public Module
{

    public:

        IOSModule(const char* moduleFilename);
        ~IOSModule();

        void* LoadProcedure(const char* procedureName) override;

    private:

        void* handle_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
