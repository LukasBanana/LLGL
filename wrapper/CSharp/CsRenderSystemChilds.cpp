/*
 * CsRenderSystemChilds.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsRenderSystemChilds.h"


namespace LHermanns
{

namespace LLGL
{


Resource::Resource(::LLGL::Resource* instance) :
    instance_ { instance }
{
}

LHermanns::LLGL::ResourceType Resource::ResourceType::get()
{
    return static_cast<LHermanns::LLGL::ResourceType>(instance_->QueryResourceType());
}

void* Resource::Native::get()
{
    return instance_;
}



} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
