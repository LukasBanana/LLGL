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
#include "../../../Core/BasicReport.h"


namespace LLGL
{


class MTPipelineState : public PipelineState
{

    public:

        MTPipelineState(bool isGraphicsPSO);

        const Report* GetReport() const override final;

        // Returns true if this is a graphics PSO.
        inline bool IsGraphicsPSO() const
        {
            return isGraphicsPSO_;
        }

    protected:

        // Writes the report with the specified message and error bit.
        void ResetReport(std::string&& text, bool hasErrors = false);
        
    private:

        const bool  isGraphicsPSO_ = false;
        BasicReport report_;

};


} // /namespace LLGL


#endif



// ================================================================================
