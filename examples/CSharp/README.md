
# C# Examples

## Build Notes

Exchange the imported DLL for LLGL. Default path is `..\..\..\..\build_msvc14\build\Debug\LHermanns.LLGL.dll`. You'll find the respective DLL when you build the main project with the CMake option `LLGL_BUILD_WRAPPER_CSHARP` being enabled.

## Prerequisite

Copy all debug DLLs into the `bin\Debug\` directory of the examples, i.e. `LLGLD.dll`, `LLGL_OpenGLD.dll` etc. for debug mode, and copy all release DLLs into the `bin\Release\` directory of the examples, i.e. `LLGL.dll`, `LLGL_OpenGL.dll` etc. for release mode.

Here is an example of the directory structure after the copying:
```
LLGL
`-examples
  `-CSharp
    `-HelloTriangle
      `-bin
        |-Debug
        | |-LLGLD.dll
        | |-LLGL_OpenGLD.dll
        | `-LLGL_Direct3D11D.dll
        `-Release
          |-LLGL.dll
          |-LLGL_OpenGL.dll
          `-LLGL_Direct3D11.dll
```

