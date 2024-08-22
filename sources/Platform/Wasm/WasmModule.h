/*
 * WasmModule.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WASM_MODULE_H
#define LLGL_WASM_MODULE_H


#include "../Module.h"


namespace LLGL
{


class WasmModule : public Module
{

    public:

        WasmModule(const char* moduleFilename, Report* report = nullptr);
        ~WasmModule();

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
