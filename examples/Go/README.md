# Go examples

<p align="center"><img src="../../docu/Icons/Go-Logo_Blue.svg" height="200" alt="Go-Logo_Blue.svg"/></p>

## Build and run examples with Go wrapper

Use the build script from the root directory of this repository to build the library that is compatible with **cgo**.
* On Windows, run `BuildMsys2.sh --golang` to build the Go wrapper with the [MSYS2/CLANG64 environment](https://www.msys2.org/docs/environments/).
* On Linux, run `BuildLinus.sh --golang`.

The `--golang` argument generates the intermediate configuration file *LLGL.pc* for the `pgk-config` command to find the binary dependencies.

