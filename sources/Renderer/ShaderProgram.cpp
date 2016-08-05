/*
 * ShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/ShaderProgram.h>


namespace LLGL
{


ShaderProgram::~ShaderProgram()
{
}

void ShaderProgram::BindAllConstantBuffers()
{
    auto bufferDescs = QueryConstantBuffers();
    for (const auto& desc : bufferDescs)
        BindConstantBuffer(desc.name, desc.index);
}


} // /namespace LLGL



// ================================================================================
