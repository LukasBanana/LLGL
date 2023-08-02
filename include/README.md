# Root directory to include LLGL

This should be your directory to add to your include paths for C and C++.
All LLGL headers include their dependencies via `<LLGL/ ... .h>` and so should your application.
For instance, add `#include <LLGL/LLGL.h>` at the top of your source files for C++ applications and `#include <LLGL-C/LLGL.h>` for C applications.


## Wrappers

## C99 Wrapper

All typenames in the C99 wrapper follow the same naming convention:
- **Enumeration** entries start with upper case `LLGL` followed by their typename and then their entry name, e.g. `LLGL::TextureType::Texture2D` translates to `LLGLTextureTypeTexture2D`.
- **Structures** start with upper case `LLGL` followed by their typename, e.g. `LLGL::TextureDescriptor` translates to `LLGLTextureDescriptor`.
- **Interface** instances start with upper case `LLGL` followed by their typename and are declared as structures with opaque pointers, e.g. `LLGL::Buffer*` translates to `LLGLBuffer`.
- **Function** start with lower case `llgl` followed by an individual name that varies from interface to interface, e.g. `LLGL::Texture::GetDesc` translates to `llglGetTextureDesc`.

