/*
 * IncludeHandler.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_INCLUDE_HANDLER_H
#define LLGL_INCLUDE_HANDLER_H


#include <LLGL/Export.h>
#include <LLGL/Report.h>
#include <LLGL/Container/UTF8String.h>
#include <LLGL/Blob.h>
#include <LLGL/Interface.h>


namespace LLGL
{


/**
\brief File include handler interface.
\see ShaderDescriptor::includeHandler
*/
class LLGL_EXPORT IncludeHandler : public Interface
{

        LLGL_DECLARE_INTERFACE( InterfaceID::IncludeHandler );

    public:

        /**
        \brief Includes a file from an input filename.
        \param[in] inFilename Specifies the input filename.
        \param[out] outFileContent Specifies the output content blob. This can contain binary or text data.
        \param[out] outReport Specifies the report handler for potential error or miscellaneous reports.
        \return True on success. Otherwise, including failed and \c outFileContent is unchanged.
        \see Blob::CreateFromFile
        */
        virtual bool Include(const UTF8String& inFilename, Blob& outFileContent, Report& outReport) = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
