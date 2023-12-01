# Root directory to include LLGL

This should be your directory to add to your include paths for C and C++.
All LLGL headers include their dependencies via `<LLGL/ ... .h>` and so should your application.
For instance, add `#include <LLGL/LLGL.h>` at the top of your source files for C++ applications and `#include <LLGL-C/LLGL.h>` for C applications.

