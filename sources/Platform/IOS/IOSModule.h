/*
 * IOSModule.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
