# ChangeLog v0.03

## Table of Contents

- [`BufferDescriptor` interface](#bufferdescriptor-interface)
- [`TextureDescriptor` interface](#texturedescriptor-interface)
- [Index buffer format](#index-buffer-format)


## `BufferDescriptor` interface

A buffer no longer has a unique type. Instead the binding flags specify for which purposes the buffer will be used. This enables the buffer to be used for multiple purposes, e.g. as read/write storage buffer in a compute shader and later as an indirect argument buffer for drawing commands. Overall the `flags` member has been replaced by three new flags: `bindFlags`, `cpuAccessFlags`, and `miscFlags`. The new flags enumerations are shared between `BufferDescriptor` and `TextureDescriptor`: `BindFlags`, `CPUAccessFlags`, and `MiscFlags`.

Before:
```cpp
// Interface:
BufferType BufferDescriptor::type;
long       BufferDescriptor::flags;
uint64_t   BufferDescriptor::size;
/* ... */

// Usage:
LLGL::BufferDescriptor myBufferDesc;
myBufferDesc.type  = LLGL::ConstantBuffer;
myBufferDesc.flags = LLGL::BufferFlags::DynamicUsage | LLGL::BufferFlags::MapWriteAccess;
myBufferDesc.size  = /* ... */;
```

After:
```cpp
// Interface:
uint64_t BufferDescriptor::size;
long     BufferDescriptor::bindFlags;
long     BufferDescriptor::cpuAccessFlags;
long     BufferDescriptor::miscFlags;
/* ... */

// Usage:
LLGL::BufferDescriptor myBufferDesc;
myBufferDesc.size           = /* ... */;
myBufferDesc.bindFlags      = LLGL::BindFlags::ConstantBuffer;
myBufferDesc.cpuAccessFlags = LLGL::CPUAccessFlags::Write;
myBufferDesc.miscFlags      = LLGL::MiscFlags::DynamicUsage;
```


## `TextureDescriptor` interface

Textures still have a unique type, but also use the new flags enumerations `BindFlags`, `CPUAccessFlags`, and `MiscFlags`.

Before:
```cpp
// Interface:
TextureType TextureDescriptor::type;
long        TextureDescriptor::flags;
Format      TextureDescriptor::format;
/* ... */

// Usage:
LLGL::TextureDescriptor myTexDesc;
myTexDesc.type  = LLGL::TextureType::Texture2DMS;
myTexDesc.flags = LLGL::TextureFlags::ColorAttachmentUsage |
                  LLGL::TextureFlags::SampleUsage          |
                  LLGL::TextureFlags::StorageUsage         |
                  LLGL::TextureFlags::FixedSamples;
/* ... */
```

After:
```cpp
// Interface:
TextureType TextureDescriptor::type;
long        TextureDescriptor::bindFlags;
long        TextureDescriptor::cpuAccessFlags;
long        TextureDescriptor::miscFlags;
Format      TextureDescriptor::format;
/* ... */

// Usage:
LLGL::TextureDescriptor myTexDesc;
myTexDesc.type              = LLGL::TextureType::Texture2DMS;
myTexDesc.bindFlags         = LLGL::BindFlags::ColorAttachment |
                              LLGL::BindFlags::SampleBuffer    |
                              LLGL::BindFlags::RWStorageBuffer;
myTexDesc.cpuAccessFlags    = 0;
myTexDesc.miscFlags         = LLGL::MiscFlags::FixedSamples;
```


## Index buffer format

The class `IndexFormat` has been removed and replaced by the `Format` enum. The only valid index formats are `Format::R16UInt` (16-bit) and `Format::R32UInt` (32-bit). An 8-bit index format is no longer supported (OpenGL was the only renderer anyways that supported it).

Before:
```cpp
// Usage:
LLGL::BufferDescriptor myBufferDesc;
myBufferDesc.type               = LLGL::BufferType::Index;
myBufferDesc.indexBuffer.format = LLGL::IndexFormat(LLGL::DataType::UInt16);
/* ... */
```

After:
```cpp
// Usage:
LLGL::BufferDescriptor myBufferDesc;
myBufferDesc.bindFlags          = LLGL::BindFlags::IndexBuffer;
myBufferDesc.indexBuffer.format = LLGL::Format::R16UInt;
/* ... */
```



