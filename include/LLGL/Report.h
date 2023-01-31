/*
 * Report.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_REPORT_H
#define LLGL_REPORT_H


#include <LLGL/Interface.h>


namespace LLGL
{


/**
\brief Error and warning report interface.
\see PipelineState::GetReport
*/
class LLGL_EXPORT Report : public Interface
{

        LLGL_DECLARE_INTERFACE( InterfaceID::Report );

    public:

        //! Returns a NUL-terminated string of the report text. This must never be null.
        virtual const char* GetText() const = 0;

        //! Returns true if this report contains error messages.
        virtual bool HasErrors() const = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
