/*
 * AndroidModule.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_ANDROID_MODULE_H
#define LLGL_ANDROID_MODULE_H


#include "../Module.h"


namespace LLGL
{


class AndroidModule : public Module
{

    public:

        AndroidModule(const char* moduleFilename);
        ~AndroidModule();

        void* LoadProcedure(const char* procedureName) override;

    private:

        void* handle_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================