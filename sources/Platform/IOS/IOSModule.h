/*
 * IOSModule.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
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

        IOSModule(const std::string& moduleFilename);
        ~IOSModule();

        IOSModule(const IOSModule&) = delete;
        IOSModule& operator = (const IOSModule&) = delete;

        void* LoadProcedure(const std::string& procedureName) override;

    private:

        void* handle_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
