# Low Level Graphics Library (LLGL)

<p align="center">
    <a href="https://github.com/LukasBanana/LLGL/blob/master/LICENSE.txt"><img alt="License" src="https://img.shields.io/badge/license-BSD--3%20clause-blue.svg" /></a>
    <a href="https://app.codacy.com/gh/LukasBanana/LLGL/dashboard"><img src="https://api.codacy.com/project/badge/Grade/53e975cd13834e78bbb6120287a5f4d8"/></a>
    <a href="https://gitter.im/LLGL-Project/community"><img alt="Join the chat at https://gitter.im/LLGL-Project/community" src="https://badges.gitter.im/LLGL-Project/LLGL.svg" /></a>
</p>

<p align="center"><img src="docu/LLGL_Logo.png"/></p>


## Abstract

LLGL aims to be a thin abstraction layer for a wide variety of modern and legacy rendering APIs as well as a multitude of platforms targeting both desktop and mobile.
LLGL provides close coupling with the underlying APIs for a rich feature set while also simplifying architectural hurdles.
The library is written mostly in C++11 with the addition of a C99 wrapper.


## Documentation

- **Version**: 0.04 Beta (see [ChangeLog](docu/ChangeLog))
- [Getting Started with LLGL](docu/GettingStarted/Getting%20Started%20with%20LLGL.pdf) (PDF)
with Introduction, Hello Triangle Tutorial, and Extensibility Example with [GLFW](http://www.glfw.org/)
- [LLGL Reference Manual](docu/refman.pdf) (PDF)
- [LLGL Coding Conventions](docu/CodingConventions/Coding%20Conventions%20for%20LLGL.pdf) (PDF)
- [Examples and Tutorials for C++](examples/Cpp)
- [Examples for C99](examples/C99)
- [Examples for C#](examples/CSharp)


## Platform Support

| Platform | CI | D3D12 | D3D11 | Vulkan | OpenGL | OpenGLES 3 | Metal |
|----------|:--:|:-----:|:-----:|:------:|:------:|:----------:|:-----:|
| <img src="docu/Icons/windows.svg" height="20" /> Windows | <p>[![MSVC16+ Build Status](https://github.com/LukasBanana/LLGL/actions/workflows/ci_windows.yml/badge.svg)](https://github.com/LukasBanana/LLGL/actions/workflows/ci_windows.yml)</p> <p>[![MSVC14 Build Status](https://ci.appveyor.com/api/projects/status/j09x8n07u3byfky0?svg=true)](https://ci.appveyor.com/project/LukasBanana/llgl)</p> | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | N/A | N/A |
| <img src="docu/Icons/linux.svg" height="20" /> GNU/Linux | [![GNU/Linux Build Status](https://github.com/LukasBanana/LLGL/actions/workflows/ci_linux.yml/badge.svg)](https://github.com/LukasBanana/LLGL/actions/workflows/ci_linux.yml) | N/A | N/A | :heavy_check_mark: | :heavy_check_mark: | N/A | N/A |
| <img src="docu/Icons/macos.svg" height="20" /> macOS | [![macOS Build Status](https://github.com/LukasBanana/LLGL/actions/workflows/ci_macos.yml/badge.svg)](https://github.com/LukasBanana/LLGL/actions/workflows/ci_macos.yml) | N/A | N/A | N/A | :heavy_check_mark: | N/A | :heavy_check_mark: |
| <img src="docu/Icons/ios.svg" height="20" /> iOS | [![iOS Build Status](https://github.com/LukasBanana/LLGL/actions/workflows/ci_ios.yml/badge.svg)](https://github.com/LukasBanana/LLGL/actions/workflows/ci_ios.yml) | N/A | N/A | N/A | N/A | :heavy_check_mark: | :heavy_check_mark: |
| <img src="docu/Icons/android.svg" height="20" /> Android | | N/A | N/A | :heavy_multiplication_x: | N/A | :heavy_check_mark: | N/A |


## Build Notes

Build scripts are provided for [**CMake**]((https://cmake.org/)). See [LLGL Build System](https://github.com/LukasBanana/LLGL/tree/master/docu#llgl-build-system) for more details.

### Windows

[**Visual Studio 2015**](https://visualstudio.microsoft.com/) or later is required to build LLGL on Windows.
The Windows SDK is also required to build D3D11 and D3D12 backends.

### macOS, iOS

[**Xcode 9**](https://developer.apple.com/xcode/) or later is required to build LLGL on macOS and iOS.

### GNU/Linux

LLGL on GNU/Linux requires the development libraries for [X11](https://www.x.org/) and its [Xrandr](https://www.x.org/wiki/Projects/XRandR/) extension (see [docs](docu#gnulinux) for details).

### Android

The [Android NDK](https://developer.android.com/ndk) with at least API level 21 is required.
The build script to generate project files is currently only supported on **GNU/Linux**
and requires [CMake 3.10](https://cmake.org/) or later and the [Code::Blocks](http://www.codeblocks.org/) IDE.

*This platform support is currently in an experimental state.*

## Installing LuaBridge (vcpkg)

Alternatively, you can build and install LLGL using [vcpkg](https://github.com/Microsoft/vcpkg/) dependency manager:

```
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
    ./vcpkg integrate install
    ./vcpkg install llgl
```

The LLGL port in vcpkg is kept up to date by Microsoft team members and community contributors. If the version is out of date, please [create an issue or pull request](https://github.com/Microsoft/vcpkg) on the vcpkg repository.


## Showcase

<p align="center">
    <img src="examples/Cpp/PostProcessing/Example.png" alt="Screenshot missing: Post processing example" style="width:300px;height:auto;">
    <img src="examples/Cpp/ShadowMapping/Example.png" alt="Screenshot missing: Shadow mapping example" style="width:300px;height:auto;">
</p>

<p align="center">
    <img src="examples/Cpp/PBR/Example.png" alt="Screenshot missing: PBR example" style="width:300px;height:auto;">
    <img src="examples/Cpp/ClothPhysics/Example.gif" alt="Screenshot missing: Cloth physics example" style="width:300px;height:auto;">
</p>

<p align="center">
    <img src="examples/Cpp/Fonts/Example.iOS.png" alt="Screenshot missing: Fonts example (iOS)" style="height:400px;width:auto;">
    <img src="examples/Cpp/ClothPhysics/Example.iOS.png" alt="Screenshot missing: Cloth physics example (iOS)" style="height:400px;width:auto;">
</p>

