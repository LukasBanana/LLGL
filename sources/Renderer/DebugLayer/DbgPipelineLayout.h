/*
 * DbgPipelineLayout.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_PIPELINE_LAYOUT_H
#define LLGL_DBG_PIPELINE_LAYOUT_H


#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>


namespace LLGL
{


class DbgPipelineLayout : public PipelineLayout
{

    public:

        DbgPipelineLayout(PipelineLayout& instance, const PipelineLayoutDescriptor& desc) :
            instance { instance },
            desc     { desc     }
        {
        }

        PipelineLayout&             instance;
        PipelineLayoutDescriptor    desc;

};


} // /namespace LLGL


#endif



// ================================================================================
