# ChangeLog v0.04

Version 0.04 introduced deprecated attributes (via `LLGL_DEPRECATED`) to keep compatibility with the previous version as far as possible.

## Table of Contents

- [Image descriptors](#image-descriptors)
- [Debug layer](#debug-layer)
- [Video adapter descriptors](#video-adapter-descriptors)
- [Rendering features](#rendering-features)
- [`FrameProfile` structure](#frameprofile-structure)
- [Binding model](#binding-model)
- [Resource barriers](#resource-barriers)
- [Renamed identifiers](#renamed-identifiers)


## Image descriptors

Image descriptors are now called image views to be consistent with what descriptors are intended for, describing a CPU-opaque object such as GPU resources that is.
`SrcImageDescriptor` has been renamed to `ImageView` and `DstImageDescriptor` has been renamed to `MutableImageView`.
This follows the same convention as with `ArrayView<T>` which is always for read-only data. A `MutableArrayView` template class might be added in the future if needed.

Before:
```cpp
// Interface:
struct LLGL::SrcImageDescriptor;
struct LLGL::DstImageDescriptor;
```

After:
```cpp
// Interface:
struct LLGL::ImageView;
struct LLGL::MutableImageView;
```


## Debug layer

The `RenderingProfiler` and `RenderingDebugger` interfaces have been merged into the latter one.
Also the single string functions `PostError` and `PostWarning` have been deprecated and replaced by `Errorf` and `Warningf` to follow the same pattern as the `Report` class.


## Video adapter descriptors

The structures `VideoAdapterDescriptor` and `VideoOutputDescriptor` have been deprecated. They can be substituted with custom structures if needed.
Instead of public information about video adpaters, the render system flags have been extended to specify a preferred video adpater for multi-GPU systems.

```cpp
// Usage:
LLGL::RenderSystemDescriptor myRenderSystemDesc;
myRenderSystemDesc.moduleName = "Direct3D12";

myRenderSystemDesc.flags = LLGL::RenderSystemFlags::PreferNVIDIA;
LLGL::RenderSystemPtr myRenderSystemNV = LLGL::RenderSystem::Load(myRenderSystemDesc);

myRenderSystemDesc.flags = LLGL::RenderSystemFlags::PreferAMD;
LLGL::RenderSystemPtr myRenderSystemAMD = LLGL::RenderSystem::Load(myRenderSystemDesc);
```


## Rendering features

The following entries from `RenderingFeatures` have been deprecated:
- `hasSamplers`: All backends must support sampler states either natively or emulated. This can be substituted with `true`.
- `hasUniforms`: All backends must support uniforms either natively or emulated. This can be substituted with `true`.


## `FrameProfile` structure

The entire union inside the `LLGL::FrameProfile` structure has been deprecated and replaced by `LLGL::ProfileCommandQueueRecord` and `LLGL::ProfileCommandBufferRecord`.


## Binding model

The binding model in LLGL is finally fully abstracted from the rendering API to the point where the actual binding slots are only defined in the `PipelineLayout`.
This only affects one function that has been deprecated, which is `ResetResourceSlots`. Simply remove the function call, no replacement is needed.
Said function was only needed for the D3D11 backend which is now managed to automatically unbind resources to avoid overlapping read-only and read-write resource views.

Rationale:
1. `ResetResourceSlots` used backend specific resource slots instead of the descriptor slots from a `PipelineLayout`. This is opposed to the `SetResource` and `SetResourceHeap` interfaces.
2. Changing `ResetResourceSlots` to use the same binding model as `SetResource(-Heap)` requires a depedency to the active PSO, which doesn't include stream-outputs and render-targets.
3. Modern rendering APIs (i.e. Vulkan, D3D12, Metal) either use a resource heap binding model or command encoder that don't save the binding state across multiple frames (or across multiple render-passes). Since LLGL aims to be aligned towards the newer APIs, the older backends should emulate that same functionality and not require to manually unbind resources.


## Resource barriers

Alongside a new binding model, the resource barriers are no longer specified per `ResourceHeap` but per `PipelineLayout` to make them accessible to individual bindings as well.
Therefore, `ResourceHeapDescriptor::barrierFlags` has been deprecated and superseded by `PipelineLayoutDescriptor::barrierFlags`.


## Renamed identifiers

The following identifiers have also been renamed and their old names have been deprecated as type aliases, i.e. they are still available but will be removed in the next version of LLGL:

##### DisplayModeDescriptor &rarr; DisplayMode
Descriptors in LLGL are intended for structures that describe an opaque object only, first and foremost GPU objects such as buffers, textures, PSOs etc.
`DisplayMode` is only a small structure to change the setting of a `Display` instance but not to create or query an object.
Therefore, `DisplayModeDescriptor` has been renamed to `DisplayMode`.

##### SetName &rarr; SetDebugName
To be consistent with the newly added field `debugName` in all `RenderSystemChild` descriptors, `RenderSystemChild::SetName` has been renamed to `RenderSystemChild::SetDebugName`.
This also separates it from the `name` identifier used for some descriptors such as `BindingDescriptor`, which refers to the shader variable name.
