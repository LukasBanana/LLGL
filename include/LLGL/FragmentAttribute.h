/*
 * FragmentAttribute.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_FRAGMENT_ATTRIBUTE_H
#define LLGL_FRAGMENT_ATTRIBUTE_H


#include <LLGL/Format.h>
#include <LLGL/SystemValue.h>
#include <LLGL/Container/StringView.h>
#include <string>
#include <cstdint>


namespace LLGL
{


/* ----- Structures ----- */

/**
\brief Fragment output attribute structure.
\remarks This is primarily used for shader reflection and to bind fragment output attributes for OpenGL 2.0.
\see FragmentShaderAttributes::outputAttribs
\see VertexAttribute
*/
struct FragmentAttribute
{
    FragmentAttribute() = default;
    FragmentAttribute(const FragmentAttribute&) = default;
    FragmentAttribute& operator = (const FragmentAttribute&) = default;

    //! Constructor for minimal fragment attribute information.
    inline FragmentAttribute(const StringView& name, std::uint32_t location = 0) :
        name     { name.begin(), name.end() },
        location { location                 }
    {
    }

    //! Constructor to initialize all members.
    inline FragmentAttribute(
        const StringView&   name,
        const Format        format,
        std::uint32_t       location    = 0,
        const SystemValue   systemValue = SystemValue::Undefined)
    :
        name        { name.begin(), name.end() },
        format      { format                   },
        location    { location                 },
        systemValue { systemValue              }
    {
    }

    /**
    \brief Fragment attribute name (for GLSL) or semantic name (for HLSL).
    \remarks Semantic names in HLSL may contain an index as suffix.
    However, this name must not contain an index suffix, because it will be added automatically.
    */
    std::string     name;

    //! Fragment attribute format. By default Format::RGBA32Float.
    Format          format      = Format::RGBA32Float;

    /**
    \brief Fragment attribute location.
    \remarks Here is an example of two fragment output attributes in GLSL with location 0 and 1:
    \code
    layout(location = 0) out vec4 myBaseColor;
    layout(location = 1) out vec4 mySecondaryColor;
    \endcode
    \remarks Here is an example of two fragment output attributes in HLSL with location 0 and 1:
    \code
    struct PixelShaderOutput
    {
        float4 myBaseColor      : SV_Target0;
        float4 mySecondaryColor : SV_Target1;
    };
    \endcode
    \remarks Here is an example of two fragment output attributes in Metal with location 0 and 1:
    \code
    struct FragmentFunctionOutput
    {
        float4 myBaseColor      [[color(0)]];
        float4 mySecondaryColor [[color(1)]];
    };
    \endcode
    */
    std::uint32_t   location    = 0;

    /**
    \brief Specifies the system value type for this fragment attribute or SystemValue::Undefined if this attribute is not a system value. By default SystemValue::Undefined.
    \remarks This can only be one the following values:
    - SystemValue::Undefined
    - SystemValue::Color
    - SystemValue::Depth
    - SystemValue::DepthGreater
    - SystemValue::DepthLess
    - SystemValue::Stencil
    */
    SystemValue     systemValue = SystemValue::Undefined;
};


} // /namespace LLGL


#endif



// ================================================================================
