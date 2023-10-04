# ChangeLog v0.04

Version 0.04 introduced deprecated attributes (via `LLGL_DEPRECATED`) to keep compatibility with the previous version as far as possible.

## Table of Contents

- [Image descriptors](#image-descriptors)


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


