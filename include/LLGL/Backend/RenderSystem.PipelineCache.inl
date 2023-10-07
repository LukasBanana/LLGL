/*
 * RenderSystem.PipelineCache.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Pipeline Caches ----- */

virtual LLGL::PipelineCache* CreatePipelineCache(
    const LLGL::Blob&       initialBlob     = {}
) override final;

virtual void Release(
    LLGL::PipelineCache&    pipelineCache
) override final;



// ================================================================================
