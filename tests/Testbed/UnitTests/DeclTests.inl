/*
 * DeclTests.inl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */


#ifndef LLGL_STANDALONE_UNIT_TEST
#   define LLGL_STANDALONE_UNIT_TEST(NAME) // Dummy
#endif

#ifndef LLGL_UNIT_TEST
#   define LLGL_UNIT_TEST(NAME) // Dummy
#endif

#ifndef LLGL_CUSTOM_PRESENT_UNIT_TEST
#   define LLGL_CUSTOM_PRESENT_UNIT_TEST(NAME) // Dummy
#endif

/* --- Renderer independent (RI) tests --- */

LLGL_STANDALONE_UNIT_TEST( ContainerDynamicArray );
LLGL_STANDALONE_UNIT_TEST( ContainerSmallVector );
LLGL_STANDALONE_UNIT_TEST( ContainerUTF8String );
LLGL_STANDALONE_UNIT_TEST( ContainerStringLiteral );
LLGL_STANDALONE_UNIT_TEST( ContainerStringOperators );
LLGL_STANDALONE_UNIT_TEST( ParseUtil );
LLGL_STANDALONE_UNIT_TEST( ImageConversions );
LLGL_STANDALONE_UNIT_TEST( ImageStrides );
LLGL_STANDALONE_UNIT_TEST( FormatAttribs );

/* --- Main tests --- */

// Command buffer tests
LLGL_UNIT_TEST( CommandBufferSubmit );
LLGL_UNIT_TEST( CommandBufferEncode );

// Resource tests - these don't render to the screen, they either only test resource creation or render to an offscreen framebuffer.
LLGL_UNIT_TEST( NativeHandle );
LLGL_UNIT_TEST( BufferWriteAndRead );
LLGL_UNIT_TEST( BufferMap );
LLGL_UNIT_TEST( BufferFill );
LLGL_UNIT_TEST( BufferUpdate );
LLGL_UNIT_TEST( BufferCopy );
LLGL_UNIT_TEST( TextureTypes );
LLGL_UNIT_TEST( TextureWriteAndRead );
LLGL_UNIT_TEST( TextureCopy );
LLGL_UNIT_TEST( TextureToBufferCopy );
LLGL_UNIT_TEST( BufferToTextureCopy );
LLGL_UNIT_TEST( RenderTargetNoAttachments );
LLGL_UNIT_TEST( RenderTarget1Attachment );
LLGL_UNIT_TEST( RenderTargetNAttachments );
LLGL_UNIT_TEST( MipMaps );
LLGL_UNIT_TEST( PipelineCaching );
LLGL_UNIT_TEST( ShaderErrors );
LLGL_UNIT_TEST( SamplerBuffer );
LLGL_UNIT_TEST( ByteBuffer );
LLGL_UNIT_TEST( BarrierReadAfterWrite );
LLGL_UNIT_TEST( Multiview );
LLGL_UNIT_TEST( DepthStencilResolve );

// Rendering tests - these are meant to render to the Testbed output window.
LLGL_UNIT_TEST( DepthBuffer );
LLGL_UNIT_TEST( StencilBuffer );
LLGL_UNIT_TEST( SceneUpdate );
LLGL_UNIT_TEST( VertexBuffer );
LLGL_UNIT_TEST( BlendStates );
LLGL_UNIT_TEST( DualSourceBlending );
LLGL_UNIT_TEST( AlphaOnlyTexture );
//LLGL_UNIT_TEST( CommandBufferMultiThreading ); //TODO: this must be rewritten as CommandBuffer constraints are violated in this test
LLGL_UNIT_TEST( CommandBufferSecondary );
LLGL_UNIT_TEST( TriangleStripCutOff );
LLGL_UNIT_TEST( TextureViews );
LLGL_UNIT_TEST( TextureStrides );
LLGL_UNIT_TEST( Uniforms );
LLGL_UNIT_TEST( ShadowMapping );
LLGL_UNIT_TEST( ViewportAndScissor );
LLGL_UNIT_TEST( ResourceBinding );
LLGL_UNIT_TEST( ResourceArrays );
LLGL_UNIT_TEST( StreamOutput );
LLGL_UNIT_TEST( ResourceCopy );
LLGL_UNIT_TEST( CombinedTexSamplers );
LLGL_UNIT_TEST( MeshShaders );
LLGL_UNIT_TEST( BGRAVertexFormat );
LLGL_UNIT_TEST( DescriptorCache );
LLGL_UNIT_TEST( VariableRateShading );

// C99 tests - must be declared last, since these tests reset the main renderer!
LLGL_CUSTOM_PRESENT_UNIT_TEST( OffscreenC99 );


#undef LLGL_STANDALONE_UNIT_TEST
#undef LLGL_UNIT_TEST
#undef LLGL_CUSTOM_PRESENT_UNIT_TEST



// ================================================================================
