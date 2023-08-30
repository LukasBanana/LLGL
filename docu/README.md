# LLGL Build System


## CMake

The primary build tool for LLGL is [CMake](https://cmake.org/) and while LLGL is compatible with older versions down to version 3.7, it is recommended to use a more up to date version, i.e. 3.18 and later.


## Windows

LLGL is compatible with the MSVC toolchain of [Visual Studio](https://visualstudio.microsoft.com/) 2015 and later.
To build D3D11 and D3D12 backends, the Windows SDK 10 (10.0.10240.0 or later) is also required.


## macOS

LLGL for macOS is compatible with [Xcode](https://developer.apple.com/xcode/) 9 and later.


## iOS

It is recommended to use a higher version of [Xcode](https://developer.apple.com/xcode/) for LLGL on iOS.
At least version 12 on *macOS 10.15 Catalina* is required to test the Metal backend on the iOS simulator.


## GNU/Linux

The `BuildLinux.sh` script recognizes several major Linux distributions.
Use `$ ./BuildLinux.sh -h` in a command prompt for more details.
See also `scripts/ListMissingPackages.sh` for details about supported OS distributions.

The following libraries are the minimum requirement to build LLGL on Linux:
- **X11**: `libx11-dev` (Debian), `libx11` (Arch), `libx11-devel` (RedHat)
- **Xrandr**: `libxrandr-dev` (Debian), `libxrandr` (Arch), `libXrandr-devel` (RedHat)


## MSYS2

To build LLGL with the GNU ABI for Windows using MinGW, there is a build script for [MSYS2](https://www.msys2.org/).
The compiler toolchain, cmake, and the OpenGL development libraries for mingw64-w64 can be installed as follows:
```
$ pacman -S mingw-w64-x86_64-gcc
$ pacman -S mingw-w64-x86_64-cmake
$ pacman -S mingw-w64-x86_64-glew
$ pacman -S mingw-w64-x86_64-freeglut
```
The build script `BuildMsys2.sh` can be used to automate the build process.
Use `$ ./BuildMsys2.sh -h` in a command prompt for more details.

