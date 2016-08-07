/*
 * ShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/ShaderProgram.h>
#include <stdexcept>


namespace LLGL
{


ShaderProgram::~ShaderProgram()
{
}


/*
 * ======= Protected: =======
 */

void ShaderProgram::ValidateShaderAttachment(Shader& shader)
{
    /* Check if another shader with the same type has already been attached */
    for (auto other : attachedShaders_)
    {
        if (other->GetType() == shader.GetType())
            throw std::invalid_argument("can not attach multiple shaders of the same type to shader program");
    }

    /* Store new attached shader */
    attachedShaders_.push_back(&shader);
}


} // /namespace LLGL



// ================================================================================
