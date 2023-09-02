/*
 * ShaderFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SHADER_FLAGS_H
#define LLGL_SHADER_FLAGS_H


#include <LLGL/Export.h>
#include <LLGL/Types.h>
#include <LLGL/VertexAttribute.h>
#include <LLGL/FragmentAttribute.h>
#include <cstddef>
#include <vector>


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Shader type enumeration.
\see ShaderDescriptor::type
*/
enum class ShaderType
{
    Undefined,      //!< Undefined shader type.
    Vertex,         //!< Vertex shader type.
    TessControl,    //!< Tessellation control shader type (also "Hull Shader").
    TessEvaluation, //!< Tessellation evaluation shader type (also "Domain Shader").
    Geometry,       //!< Geometry shader type.
    Fragment,       //!< Fragment shader type (also "Pixel Shader").
    Compute,        //!< Compute shader type.
};

/**
\brief Shader source type enumeration.
\see ShaderDescriptor::sourceType
\see ShaderDescriptor::sourceSize
*/
enum class ShaderSourceType
{
    CodeString,     //!< Refers to <code>sourceSize+1</code> bytes, describing shader high-level code (including null terminator).
    CodeFile,       //!< Refers to <code>sourceSize+1</code> bytes, describing the filename of the shader high-level code (including null terminator).
    BinaryBuffer,   //!< Refers to <code>sourceSize</code> bytes, describing shader binary code.
    BinaryFile,     //!< Refers to <code>sourceSize+1</code> bytes, describing the filename of the shader binary code (including null terminator).
};


/* ----- Flags ----- */

/**
\brief Shader compilation flags enumeration.
\see ShaderDescriptor::flags
\todo Rename to CompileFlags.
*/
struct ShaderCompileFlags
{
    enum
    {
        /**
        \brief Generate debug information.
        \remarks This compile option is equivalent to the command line arguments <tt>fxc /Od</tt>, <tt>dxc -Od</tt>, <tt>metal -O0</tt>.
        \note Only supported with: HLSL, Metal.
        */
        Debug                   = (1 << 0),

        /**
        \brief Disable optimizations.
        \remarks This compile option is equivalent to command line arguments <tt>fxc /Od</tt>, <tt>dxc -Od</tt>, <tt>metal -O0</tt>.
        \note Only supported with: HLSL, Metal, GLSL (adds <tt>#pragma optimize(off)</tt> after the <tt>#version</tt>-directive).
        */
        NoOptimization          = (1 << 1),

        /**
        \brief Optimization level 1.
        \remarks This compile option is equivalent to command line arguments <tt>fxc /O1</tt>, <tt>dxc -O1</tt>, <tt>metal -O1</tt>.
        \note Only supported with: HLSL, Metal.
        */
        OptimizationLevel1      = (1 << 2),

        /**
        \brief Optimization level 2.
        \remarks This compile option is equivalent to command line arguments <tt>fxc /O2</tt>, <tt>dxc -O2</tt>, <tt>metal -O2</tt>.
        \note Only supported with: HLSL, Metal.
        */
        OptimizationLevel2      = (1 << 3),

        /**
        \brief Optimization level 3.
        \remarks This compile option is equivalent to command line arguments <tt>fxc /O3</tt>, <tt>dxc -O3</tt>, <tt>metal -O3</tt>.
        \note Only supported with: HLSL, Metal.
        */
        OptimizationLevel3      = (1 << 4),

        /**
        \brief Warnings are treated as errors.
        \remarks This compile option is equivalent to command line arguments <tt>fxc /WX</tt>, <tt>dxc -WX</tt>, <tt>metal -Werror</tt>.
        \note Only supported with: HLSL, Metal.
        */
        WarningsAreErrors       = (1 << 5),

        /**
        \brief Patches the GLSL shader source to accommodate a flipped coordinate system from lower-left to upper-left and vice-versa,
        effectively injecting <tt>gl_Position.y = -gl_Position.y;</tt> statements into a vertex shader.
        \remarks This can be used to maintain the same vertex shader logic between GLSL and other shading languages when the screen origin is lower-left (see ScreenOrigin::LowerLeft).
        This flag should also only be used for shaders that render into an OpenGL texture as their coordinate system is reversed compared to the other rendering APIs.
        What shader stage should this flag be used with depends on what shader stage is the last to modify vertex positions before they are passed to the clipping stage,
        i.e. either vertex, tessellation-evaluation, or geometry shaders.
        \note Since there is no preprocessing performed prior to scanning the shader source, control-flow modifying macros are not recognized.
        For example, using macros that change the control flow in the main entry point or
        even obfuscate the declaration of the entry point will not be scanned correctly by this feature.
        If in doubt, write your own adjustment in the shader source like this:
        \code
        void main() {
          // Vertex shader body ...
          #if FLIP_POSITION_Y
          gl_Position.y = -gl_Position.y;
          #endif
        }
        \endcode
        Then define the macro \c FLIP_POSITION_Y on the API side like this and pass it to all vertex
        (or tessellation-evaluation or geometry) shaders that will be used for render targets,
        i.e. those shaders that render into a texture instead of a Window or Canvas:
        \code
        const LLGL::ShaderMacro myDefinesForRenderTargetShaders[] = {
          { "FLIP_POSITION_Y", myRenderer->GetRenderingCaps().screenOrigin == LLGL::ScreenOrigin::LowerLeft ? "1" : "0" },
          { nullptr, nullptr } // Null terminating entry
        };
        \endcode
        \note Only supported with: GLSL.
        \see RenderingCapabilities::screenOrigin
        */
        PatchClippingOrigin     = (1 << 6),

        /**
        \brief Specifies whether to create separable or legacy shaders.
        \remarks This is only used for the OpenGL backend. Separate and non-separate shaders (i.e. legacy shaders) must not be mixed and matched when a graphics PSO is created!
        If specified, the GLSL vertex shader must contain a \c gl_PerVertex block and the GLSL fragment shader \e may contain a \c gl_PerFragment block.
        \see https://registry.khronos.org/OpenGL/extensions/ARB/ARB_separate_shader_objects.txt
        \note Only supported with: GLSL.
        */
        SeparateShader          = (1 << 7),

        /**
        \brief Specifies whether to load the shader from the \c default.metallib file.
        \remarks This is only used for Metal and primarily for iOS (but also available on macOS).
        The default Xcode configuration will compile all Metal shaders into a single library named \c default.metallib.
        All shader entry points must have unique names or linker errors will occur.
        \note Only supported with: Metal.
        */
        DefaultLibrary          = (1 << 8),
    };
};

/**
\brief Shader stage flags enumeration.
\remarks Specifies which shader stages are affected by a state change, e.g. to which shader stages a constant buffer is bound.
\see BindingDescriptor::stageFlags
\todo Remove "Stage" suffix from enum entries and maybe unify this enumeration with ShaderType somehow.
*/
struct StageFlags
{
    enum
    {
        //! Specifies the vertex shader stage.
        VertexStage         = (1 << 0),

        //! Specifies the tessellation-control shader stage (also referred to as "Hull Shader").
        TessControlStage    = (1 << 1),

        //! Specifies the tessellation-evaluation shader stage (also referred to as "Domain Shader").
        TessEvaluationStage = (1 << 2),

        //! Specifies the geometry shader stage.
        GeometryStage       = (1 << 3),

        //! Specifies the fragment shader stage (also referred to as "Pixel Shader").
        FragmentStage       = (1 << 4),

        //! Specifies the compute shader stage.
        ComputeStage        = (1 << 5),

        //! Specifies all tessellation stages, i.e. tessellation-control-, tessellation-evaluation shader stages.
        AllTessStages       = (TessControlStage | TessEvaluationStage),

        //! Specifies all graphics pipeline shader stages, i.e. vertex-, tessellation-, geometry-, and fragment shader stages.
        AllGraphicsStages   = (VertexStage | AllTessStages | GeometryStage | FragmentStage),

        //! Specifies all shader stages.
        AllStages           = (AllGraphicsStages | ComputeStage),
    };
};


/* ----- Structures ----- */

/**
\brief Shader macro structure with name and optional body definition.
\see ShaderDescriptor::defines
*/
struct ShaderMacro
{
    ShaderMacro() = default;
    ShaderMacro(const ShaderMacro&) = default;
    ShaderMacro& operator = (const ShaderMacro&) = default;

    //! Constructor to initialize the shader macro with a name and an optional body definition.
    inline ShaderMacro(const char* name, const char* definition = nullptr) :
        name       { name       },
        definition { definition }
    {
    }

    //! Specifies the name of the macro. This must not be null.
    const char* name        = nullptr;

    //! Specifies the macro definition. If this is null, the macro has no body definition.
    const char* definition  = nullptr;
};

/**
\brief Vertex (or geometry) shader specific structure.
\see ShaderDescriptor::vertex
\see ShaderReflection::vertex
*/
struct VertexShaderAttributes
{
    /**
    \brief Vertex shader input attributes.
    \remarks All of these attributes must be contained in the \c vertexAttribs list of the vertex buffer that will be used in conjunction with the respective shader.
    In other words, a shader must not declare any vertex attributes that are not contained in the currently bound vertex buffer.
    \see BufferDescriptor::vertexAttribs
    */
    std::vector<VertexAttribute> inputAttribs;

    /**
    \brief Vertex (or geometry) shader stream-output attributes.
    \remarks Some rendering APIs need the output stream attributes for the vertex shader and other APIs need them for the geometry shader.
    To keep the code logic simple, it is valid to declare the output attributes for both the vertex and geometry shader (or even all that will be used in the same shader program).
    Output attributes are ignored where they cannot be used.
    \see RenderingFeatures::hasStreamOutputs
    \see CommandBuffer::BeginStreamOutput
    */
    std::vector<VertexAttribute> outputAttribs;
};

/**
\brief Fragment shader specific descriptor structure.
\see ShaderDescriptor::fragment
\see ShaderReflection::fragment
*/
struct FragmentShaderAttributes
{
    //! Fragment shader output attributes.
    std::vector<FragmentAttribute> outputAttribs;
};

/**
\brief Compute shader specific descriptor structure.
\see ShaderDescriptor::compute
\see ShaderReflection::compute
*/
struct ComputeShaderAttributes
{
    /**
    \brief Specifies the number of threads per threadgroup in X, Y, and Z direction. By default (1, 1, 1).
    \remarks Each component must be greater than zero.
    \remarks Only the Metal backend supports dispatch compute kernels with dynamic work group sizes.
    If not used for shader reflection, all other renderers need to specified the workgroup size within the shader code:
    - For GLSL: <code>layout(local_size_x = X, local_size_y = Y, local_size_z = Z)</code>
    - For HLSL: <code>[numthreads(X, Y, Z)]</code>
    */
    Extent3D workGroupSize = { 1, 1, 1 };
};

/**
\brief Shader source and binary code descriptor structure.
\see RenderSystem::CreateShader
*/
struct ShaderDescriptor
{
    ShaderDescriptor() = default;
    ShaderDescriptor(const ShaderDescriptor&) = default;
    ShaderDescriptor& operator = (const ShaderDescriptor&) = default;

    //! Constructor to initialize the shader descriptor with a source filename.
    inline ShaderDescriptor(const ShaderType type, const char* source) :
        type   { type   },
        source { source }
    {
    }

    //! Constructor to initialize the shader descriptor with a source filename, entry point, profile, and optional flags.
    inline ShaderDescriptor(
        const ShaderType    type,
        const char*         source,
        const char*         entryPoint,
        const char*         profile,
        long                flags = 0)
    :
        type       { type       },
        source     { source     },
        entryPoint { entryPoint },
        profile    { profile    },
        flags      { flags      }
    {
    }

    //! Specifies the type of the shader, i.e. if it is either a vertex or fragment shader or the like. By default ShaderType::Undefined.
    ShaderType                  type            = ShaderType::Undefined;

    /**
    \brief Pointer to the shader source. This is either a null terminated string or a raw byte buffer (depending on the \c sourceType member).
    \remarks This must not be null when passed to the RenderSystem::CreateShader function.
    If this is raw byte buffer rather than a null terminated string, the \c sourceSize member must not be zero!
    \see sourceSize
    \see sourceType
    */
    const char*                 source          = nullptr;

    /**
    \brief Specifies the size of the shader source (excluding the null terminator).
    \remarks If this is zero, the 'source' member is expected to point to a null terminated string and the size is automatically determined.
    For the binary buffer source type (i.e. ShaderSourceType::BinaryBuffer), this must not be zero!
    \see source
    */
    std::size_t                 sourceSize      = 0;

    /**
    \brief Specifies the type of the shader source. By default ShaderSourceType::CodeFile.
    \remarks With the filename source types (i.e. ShaderSourceType::CodeFile and ShaderSourceType::BinaryFile),
    the shader source or binary code will be loaded from file using the standard C++ file streams (i.e. std::ifstream).
    Only the binary buffer source type (i.e. ShaderSourceType::BinaryBuffer) does not require a null terminator for the \c source pointer.
    \see ShaderSourceType
    \see source
    */
    ShaderSourceType            sourceType      = ShaderSourceType::CodeFile;

    /**
    \brief Shader entry point (shader main function). If this is null, the empty string is used. By default null.
    \note Only supported with: HLSL, SPIR-V, Metal.
    */
    const char*                 entryPoint      = nullptr;

    /**
    \brief Shader target profile. If this is null, the empty string is used. By default null.
    \remarks This is renderer API dependent and is forwarded to the respective shader compiler.
    \remarks Here are a few examples:
    - For HLSL: \c "vs_5_0" specifies vertex shader model 5.0.
    - For Metal: \c "2.1" specifies shader version 2.1.
    - For GLSL: \c "320 es" specifies that the GLSL version must be patched to "#version 300 es".
    \see https://msdn.microsoft.com/en-us/library/windows/desktop/jj215820(v=vs.85).aspx
    */
    const char*                 profile         = nullptr;

    /**
    \brief Optional array of macro definitions. By default null.
    \remarks Shader macros can only be defined if \c sourceType refers to source code,
    i.e. ShaderSourceType::CodeString or ShaderSourceType::CodeFile. Otherwise, this field is ignored.
    \remarks This must either be null or a null-terminated array of ShaderMacro entries as shown here:
    \code
    const LLGL::ShaderMacro myDefines[] = {
        { "ENABLE_SHADER_PASS_FOO", "1" }, // first macro
        { "ENABLE_SHADER_PASS_BAR", "0" }, // second macro
        { nullptr, nullptr },              // terminate array
    };
    LLGL::ShaderDescriptor myShaderDesc;
    myShaderDesc.defines = myDefines;
    \endcode
    */
    const ShaderMacro*          defines         = nullptr;

    /**
    \brief Optional compilation flags. By default 0.
    \remarks This can be a bitwise OR combination of the ShaderCompileFlags enumeration entries.
    \see ShaderCompileFlags
    */
    long                        flags           = 0;

    //! Vertex (or geometry) shader specific attributes.
    VertexShaderAttributes      vertex;

    //! Fragment shader specific attributes.
    FragmentShaderAttributes    fragment;

    /**
    \brief Compute shader specific attributes.
    \remarks This member is only used to specify the number of threads per threadgroup for the Metal backend.
    \note Only supported with: Metal.
    */
    ComputeShaderAttributes     compute;
};


/* ----- Functions ----- */

/**
\brief Returns true if the specified shader source type is either ShaderSourceType::CodeString or ShaderSourceType::CodeFile.
\see ShaderSourceType
*/
LLGL_EXPORT bool IsShaderSourceCode(const ShaderSourceType type);

/**
\brief Returns true if the specified shader source type is either ShaderSourceType::BinaryBuffer or ShaderSourceType::BinaryFile.
\see ShaderSourceType
*/
LLGL_EXPORT bool IsShaderSourceBinary(const ShaderSourceType type);

/**
\brief Returns the StageFlags bitmask for the specified shader type.
\return Bitmask of the StageFlags enumeration entries for the specified input shader type, e.g. StageFlags::VertexStage for the input ShaderType::Vertex.
\see StageFlags
\see ShaderType
*/
LLGL_EXPORT long GetStageFlags(const ShaderType type);


} // /namespace LLGL


#endif



// ================================================================================
