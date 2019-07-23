/*
 * Shader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SHADER_H
#define LLGL_SHADER_H


#include "RenderSystemChild.h"
#include "ShaderFlags.h"


namespace LLGL
{


/**
\brief Shader interface.
\see RenderSystem::CreateShader
*/
class LLGL_EXPORT Shader : public RenderSystemChild
{

        LLGL_DECLARE_INTERFACE( InterfaceID::Shader );

    public:

        /**
        \brief Returns true if this shader has any errors. Otherwise, the compilation was successful.
        \remarks If the compilation failed, this shader can not be used for a graphics or compute pipeline.
        However, the details about the failure can be queried by the QueryInfoLog function.
        \see QueryInfoLog
        */
        virtual bool HasErrors() const = 0;

        /**
        \brief Disassembles the previously compiled shader byte code.
        \param[in] flags Specifies optional disassemble flags. This can be a bitwise OR combination of the 'ShaderDisassembleFlags' enumeration entries. By default 0.
        \return Disassembled assembler code or an empty string if disassembling was not possible.
        \note Only supported with: Direct3D 11, Direct3D 12.
        \todo Change return value to std::unique_ptr<LLGL::Blob>
        */
        virtual std::string Disassemble(int flags = 0) = 0;

        /**
        \brief Returns the information log after the shader compilation.
        \todo Change return value to std::unique_ptr<LLGL::Blob>
        \todo Rename to QueryReport
        */
        virtual std::string QueryInfoLog() = 0;

        /**
        \brief Returns the shader stage bitmask for this shader object.
        \see StageFlags
        */
        long GetStageFlags() const;

        //! Returns the type of this shader.
        inline ShaderType GetType() const
        {
            return type_;
        }

    protected:

        Shader(const ShaderType type);

    private:

        ShaderType type_;

};


} // /namespace LLGL


#endif



// ================================================================================
