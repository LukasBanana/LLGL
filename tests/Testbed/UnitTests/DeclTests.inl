/*
 * DeclTests.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */


/* --- Renderer independent (RI) tests --- */

#ifndef DECL_RITEST
#   ifdef GATHER_KNOWN_TESTS
#       define DECL_RITEST(NAME) \
            knownTests.push_back(#NAME)
#   else
#       define DECL_RITEST(NAME) \
            static TestResult Test##NAME(const Options& opt)
#   endif
#endif

DECL_RITEST( ContainerDynamicArray );
DECL_RITEST( ContainerSmallVector );
DECL_RITEST( ContainerUTF8String );
DECL_RITEST( ContainerStringLiteral );
DECL_RITEST( ContainerStringOperators );
DECL_RITEST( ParseUtil );
DECL_RITEST( ImageConversions );
DECL_RITEST( ImageStrides );

#undef DECL_RITEST

/* --- Main tests --- */

#ifndef DECL_TEST
#   ifdef GATHER_KNOWN_TESTS
#       define DECL_TEST(NAME) \
            knownTests.push_back(#NAME)
#   else
    #   define DECL_TEST(NAME) \
            TestResult Test##NAME(unsigned frame)
#   endif
#endif

// Command buffer tests
DECL_TEST( CommandBufferSubmit );
DECL_TEST( CommandBufferEncode );
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
DECL_TEST( ShaderErrors );
DECL_TEST( SamplerBuffer );
DECL_TEST( NativeHandle );
DECL_TEST( BarrierReadAfterWrite );

// Rendering tests
DECL_TEST( DepthBuffer );
DECL_TEST( StencilBuffer );
DECL_TEST( SceneUpdate );
DECL_TEST( BlendStates );
DECL_TEST( DualSourceBlending );
DECL_TEST( TriangleStripCutOff );
DECL_TEST( TextureViews );
DECL_TEST( Uniforms );
DECL_TEST( ShadowMapping );
DECL_TEST( ViewportAndScissor );
DECL_TEST( ResourceBinding );
DECL_TEST( ResourceArrays );
DECL_TEST( StreamOutput );
DECL_TEST( ResourceCopy );
DECL_TEST( CombinedTexSamplers );

// C99 tests
DECL_TEST( OffscreenC99 );

#undef DECL_TEST



// ================================================================================
