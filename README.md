# Low Level Graphics Library (LLGL)

<p align="center">
    <a href="https://github.com/LukasBanana/LLGL/blob/master/LICENSE.txt"><img alt="License" src="https://img.shields.io/badge/license-BSD--3%20clause-blue.svg" /></a>
    <a href="https://discord.com/channels/1257440130021457941"><img alt="Discord" src="https://img.shields.io/discord/1257440130021457941?logo=discord&logoColor=white"></a?>
</p>

<p align="center"><img src="docu/LLGL_Logo.png"/></p>


## Abstract

LLGL aims to be a thin abstraction layer for a wide variety of modern and legacy rendering APIs as well as a multitude of platforms targeting both desktop and mobile.
LLGL provides close coupling with the underlying APIs for a rich feature set while also simplifying architectural hurdles.
The library is written mostly in C++11 with the addition of a C99, C# 6.0, and Go wrapper.


## Documentation

- **Version**: 0.04 Beta (see [ChangeLog](docu/ChangeLog))
- [Getting Started with LLGL](docu/GettingStarted/Getting%20Started%20with%20LLGL.pdf) (PDF)
with Introduction, Hello Triangle Tutorial, and Extensibility Example with [GLFW](http://www.glfw.org/)
- [LLGL Reference Manual](docu/refman.pdf) (PDF)
- [LLGL Coding Conventions](docu/CodingConventions/Coding%20Conventions%20for%20LLGL.pdf) (PDF)
- [C++ Examples and Tutorials](examples/Cpp)
- [C99 Examples](examples/C99)
- [C# Examples](examples/CSharp)
- [Go Examples](examples/Go)
- [WebGL Examples](https://lukasbanana.github.io/LLGL/docu/WebPage)


## Platform Support

| Platform | CI | D3D12 | D3D11 | Vulkan | OpenGL | Metal |
|----------|:--:|:-----:|:-----:|:------:|:------:|:-----:|
| <img src="docu/Icons/windows.svg" height="20" /> Windows | <p>[![MSVC16+ CI](https://github.com/LukasBanana/LLGL/actions/workflows/ci_windows.yml/badge.svg)](https://github.com/LukasBanana/LLGL/actions/workflows/ci_windows.yml)</p> <p>[![MSVC14 CI](https://ci.appveyor.com/api/projects/status/j09x8n07u3byfky0?svg=true)](https://ci.appveyor.com/project/LukasBanana/llgl)</p> | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: | N/A |
| <img src="docu/Icons/uwp.svg" height="20" /> UWP | [![UWP CI](https://github.com/LukasBanana/LLGL/actions/workflows/ci_uwp.yml/badge.svg)](https://github.com/LukasBanana/LLGL/actions/workflows/ci_uwp.yml) | :heavy_check_mark: | :heavy_check_mark: | N/A | N/A | N/A |
| <img src="docu/Icons/linux.svg" height="20" /> GNU/Linux | [![GNU/Linux CI](https://github.com/LukasBanana/LLGL/actions/workflows/ci_linux.yml/badge.svg)](https://github.com/LukasBanana/LLGL/actions/workflows/ci_linux.yml) | N/A | N/A | :heavy_check_mark: | :heavy_check_mark: | N/A |
| <img src="docu/Icons/macos.svg" height="20" /> macOS | [![macOS CI](https://github.com/LukasBanana/LLGL/actions/workflows/ci_macos.yml/badge.svg)](https://github.com/LukasBanana/LLGL/actions/workflows/ci_macos.yml) | N/A | N/A | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| <img src="docu/Icons/ios.svg" height="20" /> iOS | [![iOS CI](https://github.com/LukasBanana/LLGL/actions/workflows/ci_ios.yml/badge.svg)](https://github.com/LukasBanana/LLGL/actions/workflows/ci_ios.yml) | N/A | N/A | :heavy_check_mark: | :heavy_check_mark: | :heavy_check_mark: |
| <img src="docu/Icons/android.svg" height="20" /> Android | [![Android CI](https://github.com/LukasBanana/LLGL/actions/workflows/ci_android.yml/badge.svg)](https://github.com/LukasBanana/LLGL/actions/workflows/ci_android.yml) | N/A | N/A | :construction: | :heavy_check_mark: | N/A |
| <img src="docu/Icons/wasm.svg" height="20" /> Wasm | [![WebAssembly CI](https://github.com/LukasBanana/LLGL/actions/workflows/ci_wasm.yml/badge.svg)](https://github.com/LukasBanana/LLGL/actions/workflows/ci_wasm.yml) | N/A | N/A | N/A | :heavy_check_mark: | N/A |
| <img src="docu/Icons/cmake.svg" height="20" /> CMake/Unity | [![Unity Build CI](https://github.com/LukasBanana/LLGL/actions/workflows/ci_unity.yml/badge.svg)](https://github.com/LukasBanana/LLGL/actions/workflows/ci_unity.yml) | | | | | |


## Build Notes

Build scripts are provided for [**CMake**]((https://cmake.org/)). See [LLGL Build System](https://github.com/LukasBanana/LLGL/tree/master/docu#llgl-build-system) for more details.

### Windows

[**Visual Studio 2015**](https://visualstudio.microsoft.com/) or later is required to build LLGL on Windows.
The Windows SDK is also required to build D3D11 and D3D12 backends.

### macOS, iOS

[**Xcode 9**](https://developer.apple.com/xcode/) or later is required to build LLGL on macOS and iOS.
For older Macs, there is a legacy mode to build LLGL for Mac OS X 10.6 using [MacPorts](https://www.macports.org/) of Clang.

### GNU/Linux

LLGL on GNU/Linux requires the development libraries for [X11](https://www.x.org/) and its [Xrandr](https://www.x.org/wiki/Projects/XRandR/) extension (see [docs](docu#gnulinux) for details).

### Android

[Android NDK](https://developer.android.com/ndk) with at least API level 21 is required.
The build script supports generating project files for [Android Studio](https://developer.android.com/studio).

## Installing (vcpkg)

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

