/*
 * DeclTests.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */


/* --- Renderer independent (RI) tests --- */

#define DECL_RITEST(NAME) \
    static TestResult Test##NAME()

DECL_RITEST( ContainerDynamicArray );
DECL_RITEST( ContainerSmallVector );
DECL_RITEST( ContainerUTF8String );
DECL_RITEST( ParseUtil );

#undef DECL_RITEST

/* --- Main tests --- */

#define DECL_TEST(NAME) \
    TestResult Test##NAME(unsigned frame)

// Command buffer tests
DECL_TEST( CommandBufferSubmit );
DECL_TEST( CommandBufferSecondary );
DECL_TEST( CommandBufferMultiThreading );

// Resource tests
DECL_TEST( BufferWriteAndRead );
DECL_TEST( BufferMap );
DECL_TEST( BufferFill );
DECL_TEST( BufferUpdate );
DECL_TEST( BufferCopy );
DECL_TEST( BufferToTextureCopy );
DECL_TEST( TextureCopy );
DECL_TEST( TextureToBufferCopy );
DECL_TEST( TextureWriteAndRead );
DECL_TEST( TextureTypes );
DECL_TEST( RenderTargetNoAttachments );
DECL_TEST( RenderTarget1Attachment );
DECL_TEST( RenderTargetNAttachments );
DECL_TEST( MipMaps );
DECL_TEST( PipelineCaching );

// Rendering tests
DECL_TEST( DepthBuffer );
DECL_TEST( StencilBuffer );
DECL_TEST( SceneUpdate );
DECL_TEST( BlendStates );
DECL_TEST( DualSourceBlending );
DECL_TEST( TriangleStripCutOff );
DECL_TEST( TextureViews );

#undef DECL_TEST



// ================================================================================
