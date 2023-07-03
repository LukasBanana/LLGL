/*
 * RenderSystem.Shader.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

/* ----- Shaders ----- */

virtual LLGL::Shader* CreateShader(
    const LLGL::ShaderDescriptor&   shaderDesc
) override final;

virtual void Release(
    LLGL::Shader&                   shader
) override final;



// ================================================================================
