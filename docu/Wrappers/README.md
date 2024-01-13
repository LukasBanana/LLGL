# LLGL Wrappers / Language Bindings


## C99

The C99 wrapper of LLGL is the primary wrapper and is required for all other language bindings as it acts as a straightforward interface of C functions and plain-old-data structures (i.e. no constructors, no implementation defined functions, no name mangling etc.).

All typenames in the C99 wrapper correspond to the following naming convention:
- **Enumeration** entries start with upper case `LLGL` followed by their typename and then their entry name, e.g. `LLGL::TextureType::Texture2D` translates to `LLGLTextureTypeTexture2D`.
- **Structures** start with upper case `LLGL` followed by their typename, e.g. `LLGL::TextureDescriptor` translates to `LLGLTextureDescriptor`.
- **Interfaces** start with upper case `LLGL` followed by their typename and are declared as structures with opaque pointers, e.g. `LLGL::Buffer*` translates to `LLGLBuffer`.
- **Functions** start with lower case `llgl` followed by an *individual* name that varies from interface to interface, e.g. `LLGL::Texture::GetDesc` translates to `llglGetTextureDesc`.

As mentioned above, not all function names are 1:1 mappings between their C++ counterpart.
This applies especially for the `RenderSystem` and `CommandBuffer` interfaces as they encompass the most functions and their C99 versions have a reduced name length,
i.e. instead of `llglRenderSystemCreateBuffer` the C99 wrapper defines `llglCreateBuffer` and instead of `llglCommandBufferUpdateBuffer` the C99 wrapper defines `llglUpdateBuffer`.
This is not only a convenience to reduce name lengths but also due to the fact that these functions don't take a parameter of their interface, which is a major difference between the wrapper and the core library,
i.e. `commandBuffer->UpdateBuffer(data)` (C++) vs `llglUpdateBuffer(data)` (C99 without the `commandBuffer` instance).
This does not imply, however, that the C99 wrapper cannot use multiple instances of a render system as well as recording multiple command buffers in parallel!
To switch between render systems, the C99 wrapper defines `llglMakeRenderSystemCurrent`, similar to the OpenGL terminology of switching between contexts.
To encode render and compute commands into a command buffer, the C99 wrapper defines `llglBegin` which puts the specified command buffer for the *current thread* into recording mode.
That means this function can be called in a multi-threaded environment, but unlike the C++ core library, multiple command buffers cannot be encoded intertwined within the same thread,
which is not a common thing to do to begin with.
In other words, before a new command buffer can be encoded in the current thread, `llglEnd` must be called to finalize the command recording.

Other differences in naming are due to the fact that C does neither support function overloading nor default parameter values.
Some functions with several optional parameters have been split into a secondary function ending with the suffix `Ext` (for "Extended").
One of such functions is `llglCreateResourceHeapExt` for instance:
```c
LLGLResourceHeap llglCreateResourceHeap(const LLGLResourceHeapDescriptor* resourceHeapDesc);
LLGLResourceHeap llglCreateResourceHeapExt(const LLGLResourceHeapDescriptor* resourceHeapDesc, size_t numInitialResourceViews, const LLGLResourceViewDescriptor* initialResourceViews);
```
This way it is clear that either both the second and third parameter should be initialized together or not at all to indicate default initialization of the resource.

The macro `LLGL_ANNOTATE` is used to annotate certain function parameters in the C99 wrapper to help the wrapper generator determine how to marshal those parameters for other language bindings.
These annotations can also provide hints to the programmer what values are accepted, e.g. `LLGL_ANNOTATE(NULL)` indicates that such a parameter is nullable and
`LLGL_ANNOTATE([numBuffers])` indicates that such a parameter is a pointer that must provide at least `numBuffers` elements.


## C#

The C# wrapper of LLGL requires C# 6.0 and [.NET framework](https://dotnet.microsoft.com/en-us/download/dotnet-framework) 4.0.
Just like the C99 wrapper, the C# wrapper is mostly auto-generated via the [WrapperGen](../../scripts/WrapperGen) Python 3 scripts.

