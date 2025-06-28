/*
 * D3D9Buffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_BUFFER_H
#define LLGL_D3D9_BUFFER_H


#include <LLGL/Buffer.h>


namespace LLGL
{


class D3D9Buffer : public Buffer
{

    public:

        void SetDebugName(const char* name) override;

    protected:

        D3D9Buffer(long bindFlags);

};


} // /namespace LLGL


#endif



// ================================================================================
