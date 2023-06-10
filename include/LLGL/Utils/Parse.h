/*
 * Parse.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_PARSE_H
#define LLGL_PARSE_H


#include <LLGL/Export.h>
#include <LLGL/Container/SmallVector.h>
#include <LLGL/Container/StringView.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/SamplerFlags.h>


namespace LLGL
{


/**
\defgroup group_util Global utility functions to parse resource descriptors from strings.
\addtogroup group_util
@{
*/

/**
\brief Context class for descriptor parsing
\remarks Numeric values are parsed as follows:
\code
DIGIT       := '0'-'9'
HEX_DIGIT   := '0'-'9' | 'a'-'f' | 'A'-'F'
BOOL_TRUE   := 'true' | 'yes' | 'on' | '1'
BOOL_FALSE  := 'false' | 'no' | 'off' | '0'
uint        := DIGIT+ | '0x' HEX_DIGIT+
float       := uint | uint '.' uint
boolean     := BOOL_TRUE | BOOL_FALSE
\endcode
\see Parse
*/
class LLGL_EXPORT ParseContext
{

    public:

        using StringType        = SmallVector<char, 0>;
        using TokenArrayType    = SmallVector<StringView, 0>;

    public:

        ParseContext() = default;

        ParseContext(const ParseContext&) = default;
        ParseContext& operator = (const ParseContext&) = default;

        ParseContext(ParseContext&&) = default;
        ParseContext& operator = (ParseContext&&) = default;

        explicit ParseContext(const StringView& source);

    public:

        /**
        \brief Generates a pipeline layout descriptor from this parse context.
        \remarks The syntax for this conversion is as follows:
        - All binding points wrapped inside <code>"heap{"</code>...<code>"}"</code> will be put into PipelineLayoutDescriptor::heapBindings. Otherwise, they are put into PipelineLayoutDescriptor::bindings.
        - Each pair of binding point type (i.e. BindingDescriptor::type) and binding flags (i.e. BindingDescriptor::bindFlags) is specified by one of the following identifiers:
            - \c cbuffer for constant buffers (i.e. ResourceType::Buffer and BindFlags::ConstantBuffer).
            - \c buffer for sampled buffers (i.e. ResourceType::Buffer and BindFlags::Sampled).
            - \c rwbuffer for read/write storage buffers (i.e. ResourceType::Buffer and BindFlags::Storage).
            - \c texture for textures (i.e. ResourceType::Texture and BindFlags::Sampled).
            - \c rwtexture for read/write textures (i.e. ResourceType::Texture and BindFlags::Storage).
            - \c sampler for sampler states (i.e. ResourceType::Sampler).
        - Optionally, the resource <b>name</b> is specified as an arbitrary identifier followed by the at-sign (e.g. <code>"texture(myColorMap@1)"</code>).
        - The <b>slot</b> of each binding point (i.e. BindingDescriptor::slot) is specified as an integral number within brackets (e.g. <code>"texture(1)"</code>).
        - The <b>array size</b> of each binding point (i.e. BindingDescriptor::arraySize) can be optionally specified right after the slot within squared brackets (e.g. <code>"texture(1[2])"</code>).
        - Optionally, multiple slots can be specified within the brackets if separated by commas (e.g. <code>"texture(1[2],3)"</code>).
        - Each binding point is separated by a comma, the last comma being optional (e.g. <code>"texture(1),sampler(2),"</code> or <code>"texture(1),sampler(2)"</code>).
        - The stage flags (i.e. BindingDescriptor::stageFlags) can be specified after each binding point with a preceding colon using the following identifiers:
            - \c vert for the vertex shader stage (i.e. StageFlags::VertexStage).
            - \c tesc for the tessellation-control shader stage (i.e. StageFlags::TessControlStage).
            - \c tese for the tessellation-evaluation shader stage (i.e. StageFlags::TessEvaluationStage).
            - \c geom for the geometry shader stage (i.e. StageFlags::GeometryStage).
            - \c frag for the fragment shader stage (i.e. StageFlags::FragmentStage).
            - \c comp for the compute shader stage (i.e. StageFlags::ComputeStage).
        - If no stage flag is specified, all shader stages will be used.
        - There is a secondary syntax for uniform descriptors (see LLGL::UniformType for accepted type names):
            \code
            arraySize   := '[' INT ']'
            uniform     := NAME | NAME arraySize
            uniformList := uniform | uniform ',' uniformList
            uniformDesc := TYPE '(' uniformList ')'
            \endcode
        - Whitespaces are ignored (e.g. blanks <code>' '</code>, tabulators <code>'\\t'</code>, new-line characters <code>'\\n'</code> and <code>'\\r'</code> etc.), see C++ STL function <code>std::isspace</code>.
        \remarks Here is a usage example:
        \code
        // Standard way of declaring a pipeline layout:
        LLGL::PipelineLayoutDescriptor myLayoutDescStd;

        myLayoutDescStd.heapBindings = {
            LLGL::BindingDescriptor{ "Scene",    LLGL::ResourceType::Buffer,  LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::FragmentStage | LLGL::StageFlags::VertexStage, 0u,     },
            LLGL::BindingDescriptor{             LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled,        LLGL::StageFlags::FragmentStage,                                 1u      },
            LLGL::BindingDescriptor{ "TexArray", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled,        LLGL::StageFlags::FragmentStage,                                 2u, 4u, },
        };
        myLayoutDescStd.bindings = {
            LLGL::BindingDescriptor{             LLGL::ResourceType::Sampler, 0,                               LLGL::StageFlags::FragmentStage,                                 3u      },
        };
        myLayoutDescStd.uniforms = {
            LLGL::UniformDescriptor{ "WorldMatrix", LLGL::UniformType::Float4x4 }
        };

        auto myLayout = myRenderer->CreatePipelineLayout(myLayoutDescStd);
        \endcode
        The same pipeline layout can be created with the following usage of this utility function:
        \code
        // Abbreviated way of declaring a pipeline layout using the utility function:
        LLGL::PipelineLayoutDescriptor myLayoutDescUtil = LLGL::Parse("heap{ cbuffer(Scene@0):frag:vert },"
                                                                      "heap{ texture(1, TexArray@2[4]):frag },"
                                                                      "sampler(3):frag,"
                                                                      "float4x4(WorldMatrix),");
        auto myLayout = myRenderer->CreatePipelineLayout(myLayoutDescUtil);
        \endcode
        */
        PipelineLayoutDescriptor AsPipelineLayoutDesc() const;

        /**
        \brief Generates a sampler descriptor from this parse context.
        \remarks The syntax for this conversion is as follows:
        - Each sampler attribute must be assigned with a value using the \c '=' assignment operator (e.g. <code>filter=linear</code>).
        - Each sampler attribute is separated by a comma, the last comma being optional (e.g. <code>"filter=linear,address=repeat,"</code> or <code>"filter=linear,address=repeat"</code>).
        - The accepted sampler attributes are as follows:
            - \c address maps to SamplerAddressMode and the accepted values are:
                - \c repeat (SamplerAddressMode::Repeat).
                - \c mirror (SamplerAddressMode::Mirror).
                - \c clamp (SamplerAddressMode::Clamp).
                - \c border (SamplerAddressMode::Border).
                - \c mirrorOnce (SamplerAddressMode::MirrorOnce).
            - \c filter maps to minification, magnification, and MIP-map filters and the accepted values are:
                - \c none (Only for \c filter.mip to disable \c mipMapEnabled).
                - \c nearest (SamplerFilter::Nearest).
                - \c linear (SamplerFilter::Linear).
            - \c lod.bias maps to SamplerDescriptor::mipMapLODBias and the value must be an integral or fractional number.
            - \c lod.min maps to SamplerDescriptor::minLOD and the value must be an integral or fractional number.
            - \c lod.max maps to SamplerDescriptor::maxLOD and the value must be an integral or fractional number.
            - \c anisotropy maps to SamplerDescriptor::maxAnisotropy and the value must be an integral number.
            - \c compare maps to SamplerDescriptor::compareOp (also enables SamplerDescriptor::compareEnabled) and the accepted values are:
                - \c never (CompareOp::NeverPass).
                - \c ls (CompareOp::Less).
                - \c eq (CompareOp::Equal).
                - \c le (CompareOp::LessEqual).
                - \c gr (CompareOp::Greater).
                - \c ne (CompareOp::NotEqual).
                - \c ge (CompareOp::GreaterEqual).
                - \c always (CompareOp::AlwaysPass).
            - \c border maps to SamplerDescriptor::borderColor and the accepted values are:
                - \c transparent (Border color of <code>{ 0, 0, 0, 0 }</code>).
                - \c black (Border color of <code>{ 0, 0, 0, 1 }</code>).
                - \c white (Border color of <code>{ 1, 1, 1, 1 }</code>).
        - The sampler attribute \c address also accepts optional subscripts which can be one or more of the following subscripts:
            - \c u or \c x maps to SamplerDescriptor::addressModeU.
            - \c v or \c y maps to SamplerDescriptor::addressModeV.
            - \c w or \c z maps to SamplerDescriptor::addressModeW.
        - The sampler attribute \c filter also accepts optional subscripts which can be either one of the following identifiers:
            - \c min maps to SamplerDescriptor::minFilter.
            - \c mag maps to SamplerDescriptor::magFilter.
            - \c mip maps to SamplerDescriptor::mipMapFilter.
        \remarks Here is a usage example:
        \code
        // Standard way of declaring a sampler descriptor:
        LLGL::SamplerDescriptor mySamplerDescStd;

        mySamplerDescStd.addressModeU = LLGL::SamplerDescriptor::Clamp;
        mySamplerDescStd.addressModeV = LLGL::SamplerDescriptor::Clamp;
        mySamplerDescStd.minFilter = LLGL::SamplerFilter::Nearest;
        mySamplerDescStd.magFilter = LLGL::SamplerFilter::Nearest;
        mySamplerDescStd.mipMapLODBias = 2.5f;

        auto mySampler = myRenderer->CreateSampler(mySamplerDescStd);
        \endcode
        The same sampler can be created with the following usage of this utility function:
        \code
        LLGL::SamplerDescriptor mySamplerDescUtil = LLGL::Parse(
            "address.uv=clamp,filter.min=nearest,filter.mag=nearest,lod.bias=2.5"
        );
        auto mySampler = myRenderer->CreateSampler(mySamplerDescUtil);
        \endcode
        */
        SamplerDescriptor AsSamplerDesc() const;

        /**
        \brief Generates a depth descriptor from this parse context.
        \remarks The syntax for this conversion is as follows:
        - \c compare maps to DepthDescriptor::compareOp; see ParseContext::AsSamplerDesc for accepted value.
        - \c test maps to DepthDescriptor::testEnabled and accepts a boolean value.
        - \c write maps to DepthDescriptor::writeEnabled and accepts a boolean value.
        \remarks Here is a usage example:
        \code
        LLGL::DepthDescriptor myDepthDesc = LLGL::Parse("compare=ls,test=on,write=on");
        \endcode
        */
        DepthDescriptor AsDepthDesc() const;

        /**
        \brief Generates a stencil-face descriptor from this parse context.
        \remarks The syntax for this conversion is as follows:
        - \c sfail maps to StencilFaceDescriptor::stencilFailOp and the accepted values are:
            - \c keep (StencilOp::Keep).
            - \c zero (StencilOp::Zero).
            - \c set (StencilOp::Replace).
            - \c inc (StencilOp::IncClamp).
            - \c dec (StencilOp::DecClamp).
            - \c inv (StencilOp::Invert).
            - \c incw (StencilOp::IncWrap).
            - \c decw (StencilOp::DecWrap).
        - \c dfail maps to StencilFaceDescriptor::depthFailOp and the accepted values are the same as for \c sfail.
        - \c dpass maps to StencilFaceDescriptor::depthPassOp and the accepted values are the same as for \c sfail.
        - \c compare maps to StencilFaceDescriptor::compareOp; see ParseContext::AsSamplerDesc for accepted value.
        - \c read maps to StencilFaceDescriptor::readMask and accepts an integer value.
        - \c write maps to StencilFaceDescriptor::writeMask and accepts an integer value.
        - \c ref maps to StencilFaceDescriptor::reference and accepts an integer value.
        */
        StencilFaceDescriptor AsStencilFaceDesc() const;

        /**
        \brief Generates a stencil descriptor from this parse context.
        */
        StencilDescriptor AsStencilDesc() const;

    public:

        /**
        \brief Implicit conversion to PipelineLayoutDescriptor.
        \see AsPipelineLayoutDesc
        */
        inline operator PipelineLayoutDescriptor () const
        {
            return AsPipelineLayoutDesc();
        }

        /**
        \brief Implicit conversion to SamplerDescriptor.
        \see AsSamplerDesc
        */
        inline operator SamplerDescriptor() const
        {
            return AsSamplerDesc();
        }

        /**
        \brief Implicit conversion to DepthDescriptor.
        \see AsDepthDesc
        */
        inline operator DepthDescriptor() const
        {
            return AsDepthDesc();
        }

        /**
        \brief Implicit conversion to StencilFaceDescriptor.
        \see AsStencilFaceDesc
        */
        inline operator StencilFaceDescriptor() const
        {
            return AsStencilFaceDesc();
        }

        /**
        \brief Implicit conversion to StencilDescriptor.
        \see AsStencilDesc
        */
        inline operator StencilDescriptor() const
        {
            return AsStencilDesc();
        }

    private:

        StringType      source_;
        TokenArrayType  tokens_;

};

/**
\brief Returns a parse context for the input source code.
\remarks This is only a convenience function for the ParseContext constructor.
\see ParseContext
*/
inline ParseContext Parse(const StringView& s)
{
    return ParseContext{ s };
}

/** @} */


} // /namespace LLGL


#endif



// ================================================================================
