/*
 * MTPipelineState.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_PIPELINE_STATE_H
#define LLGL_MT_PIPELINE_STATE_H


#import <Metal/Metal.h>

#include <LLGL/PipelineState.h>


namespace LLGL
{


class MTPipelineState : public PipelineState
{

    public:

        MTPipelineState(bool isGraphicsPSO);

        // Returns true if this is a graphics PSO.
        inline bool IsGraphicsPSO() const
        {
            return isGraphicsPSO_;
        }

    private:

        const bool isGraphicsPSO_ = false;

};


} // /namespace LLGL


#endif



// ================================================================================
