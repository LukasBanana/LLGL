/*
 * Query.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_QUERY_H__
#define __LLGL_QUERY_H__


#include "Export.h"


namespace LLGL
{


//! Query type enumeration.
enum class QueryType
{
    SamplesPassed,                      //!< Number of samples that passed the depth test.
    AnySamplesPassed,                   //!< Non-zero if any samples passed the depth test.
    AnySamplesPassedConservative,       //!< Non-zero if any samples passed the depth test within a conservative rasterization.
    PrimitivesGenerated,                //!< Number of generated vertices (either emitted from the geometry or vertex shader).
    TransformFeedbackPrimitivesWritten, //!< Number of vertices that have been written into a transform feedback buffer.
    TimeElapsed,                        //!< Time that has elapsed between the begin- and end query command.
};


//! Query interface.
class LLGL_EXPORT Query
{

    public:

        virtual ~Query()
        {
        }

        //! Returns the type of this query.
        inline QueryType GetType() const
        {
            return type_;
        }

    protected:

        Query(const QueryType type) :
            type_( type )
        {
        }

    private:

        QueryType type_;

};


} // /namespace LLGL


#endif



// ================================================================================
