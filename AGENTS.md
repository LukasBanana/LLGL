# AGENTS.md — LLGL

## Project context

LLGL (Low Level Graphics Library) is a thin C++11 abstraction layer over modern and legacy
rendering APIs (Direct3D 11/12, Vulkan, OpenGL, OpenGLES 3, WebGL, Metal) across desktop and
mobile platforms. C99, C#, and Go wrappers are generated on top of the C++ API.

## Primary goals when editing

When making changes, prefer solutions that are:
1. Correct across supported backends and platforms
2. Consistent with existing LLGL API and naming conventions
3. Easy to validate via examples and tests
4. Minimal and localized unless a broader refactor is clearly needed

## General coding rules

- Preserve the existing public API unless the task explicitly requires an API change.
- Avoid unnecessary stylistic churn in unrelated files.
- Keep platform-specific code isolated where possible.
- Prefer explicit, readable code over clever or condensed code.
- Maintain compatibility with the project’s existing C++ style and header organization.
- Do not introduce dependencies unless they are clearly needed and justified.

## C++ language standard policy

- The C++ codebase should be treated as **strictly C++11 compliant by default**.
- You may use compiler extensions or newer C++ features (C++14, C++17, and later)
  **only when they are explicitly guarded by preprocessor conditions** so they are excluded
  on compilers that do not support them.
- Do not introduce unguarded syntax or library usage that would break C++11 compilation.
- If a feature depends on a newer language version or compiler-specific extension,
  isolate it clearly and document the requirement near the use site.
- Prefer C++11-compatible alternatives unless the newer feature is clearly necessary and
  safely conditional.

## Build and configuration model

CMake is the source of truth; the platform scripts are convenience wrappers around it. The
top-level configuration uses C++11 for normal desktop/mobile builds, but C++17 for UWP and
Emscripten. `CMAKE_CXX_STANDARD_REQUIRED` is enabled, so do not infer the language standard from
the compiler selected by the IDE.

Most optional components are OFF unless the platform or a convenience script enables them:

- `LLGL_BUILD_TESTS` and `LLGL_BUILD_EXAMPLES` are OFF in a bare CMake configure.
- `LLGL_BUILD_STATIC_LIB` is OFF. The default is separate shared libraries: the frontend
  `LLGL` library plus one module per enabled renderer. Runtime loading uses names such as
  `LLGL_Vulkan`; static builds link the enabled modules into the application and have different
  link-order requirements (see `docu/README.md`).
- Renderer defaults are platform-dependent: Null is ON, desktop Windows enables D3D11/D3D12 and
  OpenGL, desktop Apple enables Metal and OpenGL, Linux enables OpenGL, mobile enables OpenGLES3,
  and Emscripten enables WebGL. Vulkan and OpenXR are opt-in.
- `LLGL_BUILD_WRAPPER_CSHARP` and `LLGL_BUILD_WRAPPER_GO` require
  `LLGL_BUILD_WRAPPER_C99`. `LLGL_BUILD_XR_OPENXR` currently requires a compatible Vulkan,
  D3D11, or D3D12 renderer and an OpenXR loader/header setup.

For a clean local configure, initialize submodules first, then use an out-of-source build:

```text
git submodule update --init --recursive
cmake -S . -B build_local -A x64 -DLLGL_BUILD_EXAMPLES=ON -DLLGL_BUILD_TESTS=ON
cmake --build build_local --config RelWithDebInfo
```

Use a separate build directory for materially different configurations (static/shared,
Debug/Release, renderer options, or architecture). CMake caches options, and rebuilding only an
executable does not reconfigure or rebuild a renderer whose compile-time option changed.

The checked-in platform entry points encode extra assumptions: `BuildWin64.bat` enables examples,
tests, C99, OpenGL, D3D11, and D3D12 and may obtain `GaussianLib`; Android requires
`ANDROID_NDK_ROOT` and API level 21 or later; Linux needs X11 and Xrandr development libraries;
Wasm uses Emscripten; Apple builds use Xcode. Check the corresponding `Build*.sh/.bat/.command`
script before inventing a new invocation.

## Dependencies and generated files

The repository has two git submodules: `external/SPIRV-Headers` and `external/GaussianLib`.
SPIRV-Headers is required when Vulkan SPIR-V reflection is enabled. GaussianLib is used by some
builds/examples and the Windows convenience script can fetch it into the build area. Keep these
requirements optional where the existing CMake option makes them optional; do not include generated
or downloaded dependency trees in a focused source change.

The wrapper files are generated from public headers and C99 declarations. Run
`scripts/GenerateWrappers.bat` from the `scripts/` directory (with Python available) after changing
an input declaration; it regenerates `include/LLGL-C/LLGLWrapper.h`, `wrapper/CSharp/LLGLWrapper.cs`,
and `wrapper/Go/LLGLWrapper.go`. Never hand-edit those generated outputs. Public struct layout is
part of the wrapper ABI: update the generator inputs and check the C99 assertions when adding,
removing, or reordering fields.

A high priority goal of LLGL is to keep external dependencies to an absolute minimum.

- Avoid adding new third-party dependencies unless there is a strong, explicit justification.
- Prefer standard library, existing in-repo utilities, or optional features already controlled
  by CMake configuration.
- LLGL should remain buildable with only the standard compiler toolchain when optional features
  are disabled in CMake.
- Features that require extra libraries, such as SPIR-V reflection or similar backend-specific
  tooling, must remain optional and clearly gated in the build system.
- Do not make a new dependency mandatory if the feature can be isolated behind a build option.
- If you add or use a dependency, document why it is required and what build option or platform
  feature enables it.

## Formatting and style

### Important: style over formatter dogma
This project should **not** be forced into a rigid formatter style if that harms readability or
conflicts with established LLGL conventions.
Use `clang-format` only as a reference, not as an absolute authority. Match nearby code first.

## Tests

The test setups are gated by `LLGL_BUILD_TESTS`:

- **`tests/Testbed/`** — the automated regression suite. The `Testbed` executable runs unit tests
  against one or more backends, many by rendering to an offscreen target and image-diffing against
  `tests/Testbed/Reference/`. Run with backend aliases as args:
  - `Testbed` — all available backends.
  - `Testbed vk` / `gl` / `gl330` / `d3d11` / `d3d12` / `mt` — specific backend (optional GL version).
  - `-run=TestName1,TestName2` — run only the named tests.
  - `-f` fast mode, `-g` greedy (don't stop on first failure), `-d gpu+cpu` enable debug layers,
    `-p` pedantic (disable diff threshold), `-v` verbose. Single-char flags combine: `-cdf`.
  - New unit tests live in `tests/Testbed/UnitTests/Test*.cpp` and are registered in `DeclTests.inl`.
- **`tests/Test_*.cpp`** — standalone, often interactive manual tests for a single backend/feature.
  These are deprecated and no longer maintained. They served as temporary tests during development
  of new backends.

#### Testbed gotchas (each has cost real debugging time)

- **Vulkan needs `-DLLGL_VK_ENABLE_SPIRV_REFLECT=ON`.** It defaults to **OFF** (declared in
  `sources/Renderer/Vulkan/CMakeLists.txt`; needs the `external/SPIRV-Headers` submodule) and
  several tests require it, but the failures do not name the cause. `TextureViews` fails with a
  near-total image mismatch and renders a flat clear colour — it uses a pipeline layout mixing
  heap-based and individual bindings, so without reflection the descriptor set is invalid and
  every draw is dropped **silently**. The full suite then traps in
  `VKPipelineLayout::CreatePermutation` on `assertion failed: 'uniformDescs_.empty()'`.
  The option affects the **Vulkan renderer module**, so rebuild the `LLGL_Vulkan` target — not
  just `Testbed`, which changes nothing and looks like the fix failed. With it on the whole
  Vulkan suite passes; NativeHandle, ByteBuffer, CombinedTexSamplers and OffscreenC99 skip by
  design.
- **Reach for `-d gpu+cpu` early.** It is what surfaces the real message above. A near-total
  image mismatch was once mistaken for a regression and chased across three worktrees before
  debug layers named it.
- The Testbed prompts for a keypress at startup; feed it `</dev/null` in scripted runs.

## Architecture

LLGL splits into a public API, a backend-agnostic frontend, and pluggable backend modules.

### Public API — `include/LLGL/`
Pure-virtual interface classes (`RenderSystem`, `CommandBuffer`, `SwapChain`, `Buffer`, `Texture`,
`PipelineState`, etc.). `RenderSystem` is the top-level factory; `XRSystem` is a parallel
top-level entry point for OpenXR that produces an XR-compatible `RenderSystem` — its public
headers live in the `include/LLGL/XR/` subdirectory.
`include/LLGL/Backend/*.inl` are not normal headers — they are interface-method declaration
snippets that a backend `#include`s inside its class body to guarantee it implements every
frontend method (see `include/LLGL/Backend/README.md`).

### Backend modules — `sources/Renderer/`
Each backend (`Direct3D11/`, `Direct3D12/`, `Vulkan/`, `OpenGL/`, `Metal/`, `Null/`) is built as
a **separate shared library**. `RenderSystem::Load("Vulkan")` dynamically loads the corresponding
module (`LLGL_Vulkan.dll` etc.) at runtime; with `LLGL_BUILD_STATIC_LIB` they are linked
statically instead. Module loading/registry lives in `sources/Renderer/RenderSystem*.cpp`,
`RenderSystemModule.*`, and `RenderSystemRegistry.*`. Files directly under `sources/Renderer/`
(not in a backend subdir) are the shared frontend used by all backends. `sources/Renderer/SPIRV/`
holds the SPIR-V parser/reflection used by the Vulkan (and GL SPIR-V) backends.
`sources/Renderer/DXCommon` holds shared utilities for D3D11 and D3D12 backends.

### Resource barriers / synchronization
LLGL inserts the barriers needed for a render-target→sampled-texture transition implicitly, via
`CommandBuffer::SetResourceHeap()`: the Vulkan backend emits `vkCmdPipelineBarrier` through
`VKResourceHeap::SetBarrierSlots`, and the D3D12 backend issues state transitions through
`D3D12ResourceHeap::TransitionResources` / `InsertUAVBarriers`. D3D11 and Metal don't need
explicit barriers (driver/encoder boundaries handle it); OpenGL flushes a fixed set of
`glMemoryBarrier` bits from `SetResourceHeap`. The public `CommandBuffer::ResourceBarrier()` is
**not** for this case — it's for UAV/storage hazards between compute and graphics work, and is
documented as D3D12/D3D11/OpenGL-only (TODO for Vulkan/Metal). Apps therefore do not call
`ResourceBarrier()` between render passes (see `examples/Cpp/ShadowMapping/Example.cpp`).

### Platform layer — `sources/Platform/`
Per-OS windowing, displays, input, paths, and debug hooks (`Win32/`, `Linux/`, `MacOS/`, `IOS/`,
`Android/`, `UWP/`, `Wasm/`, plus shared `POSIX/`).

### Core — `sources/Core/`
Renderer- and platform-independent utilities: image conversion, string/UTF-8 handling, logging,
threading, custom containers, the `Trap`/`Report` error machinery.

### XR — `include/LLGL/XR/`, `sources/XR/`
The public XR API — `XRSystem`, `XRSession`, `XRSwapChain`, and `XRSystemFlags` — lives in
`include/LLGL/XR/`, a subdirectory kept separate from the rest of `include/LLGL/`. The frontend
implementation is in `sources/XR/`, with the runtime-specific code under `sources/XR/OpenXR/`.
The OpenXR graphics bindings live alongside each backend — `sources/Renderer/Vulkan/OpenXR/`,
`sources/Renderer/Direct3D11/OpenXR/`, and `sources/Renderer/Direct3D12/OpenXR/`; the XR system
prepares native device handles and feeds them into the chosen backend via
`RenderSystemDescriptor::nativeHandle`. The API is intentionally not OpenXR specific but
currently the only backend against the XR interface.

## Examples

`LLGL_BUILD_EXAMPLES` builds a static `ExampleBase` library (`examples/Cpp/ExampleBase/`) that
every C++ example links against. Most examples subclass `ExampleBase` — which owns the
window/canvas, swap-chain, and main loop — and use the `LLGL_IMPLEMENT_EXAMPLE(CLASS)` macro to
generate the per-platform entry point (`main`, `android_main`, or iOS `InstantiateExample`). XR
examples are the exception: they render into XR swap-chains rather than a window, so they stand
alone and define their own entry point. Each example's working directory is set (per
`CMakeLists.txt`) to its source folder so it can find its shaders/assets.

- **Assets/shaders**: `ReadAsset()` (`ExampleBase/FileUtils.h`) is the cross-platform asset
  reader — on Android it reads from the APK bundle via the asset manager registered by
  `ExampleBase::SetAndroidApp`. Note `LLGL::ShaderDescFromFile` only records a *filename* for the
  renderer to open later, which cannot reach APK assets; for Android-compatible shader loading,
  read the bytes via `ReadAsset` and pass them as a `ShaderSourceType::BinaryBuffer`/`CodeString`.
- **Android app packaging**: `./BuildAndroid.sh --apps` cross-compiles the examples and generates
  Android Studio projects under `build_android/apps/Example_<Name>/`. `--apps` forces all four
  ABIs; append `--abi=<abi>` *after* `--apps` to build just one. The generated projects ship only
  `gradle-wrapper.properties` (no `gradlew`/`gradle-wrapper.jar`), so they must be built from
  Android Studio or a separately-installed Gradle, not a self-contained `./gradlew`. An
  `Android.openxr` marker file in an example directory makes `BuildAndroid.sh` substitute the XR
  `AndroidManifest` (immersive-HMD feature, OpenXR permissions and runtime-broker queries) and
  bundle `libopenxr_loader.so` into the APK.

## Traps in the existing code

Non-obvious behaviour that has caused real bugs here. Each is a property of LLGL as it stands,
not a style preference.

### `g_formatAttribs` is indexed positionally by the `Format` enum
`sources/Renderer/Format.cpp` holds `g_formatAttribs[]`, and `GetFormatAttribs(format)` indexes
it by `static_cast<uint32>(format)`. Every active member of `include/LLGL/Format.h` must have
exactly one row **in the same order**, and a member commented out in one file must be commented
out in the other. A missing row silently misaligns every format after it, returning another
format's block size and bits-per-pixel. Four missing BC6H/BC7 rows did exactly that to
everything from ASTC4x4 onward. To verify: extract the trailing `// Name` comments from the
active rows and diff against the enum member order — identical count, identical order.

### `GetMemoryFootprint(format, numTexels)` cannot size block-compressed images
It returns **0** when the texel count does not divide the block area, and otherwise assumes
exact tiling — so any extent that is not a whole number of blocks, and every sub-block tail mip,
sizes to zero. Zero-size Vulkan staging buffers then fail `VUID-VkBufferCreateInfo-size-00912`
at upload. The extent-based overload `GetMemoryFootprint(type, format, extent, subresource)`
(`TextureFlags.cpp`) does per-mip ceil-division block maths and is the one to use. The
texel-count overload is unfixable by signature — a count cannot be ceil-divided per dimension.

### `Surface::AdaptForVideoMode` returning false aborts the resize
`SwapChain::ResizeBuffers` only proceeds inside `if (GetSurface().AdaptForVideoMode(...))`. The
documented contract is "false means I modified your arguments", so a `Surface` that legitimately
clamps a request would never resize at all. A `Surface` implementation should be handed
`GetContentSize()` by its caller and return true on the common path.

### `VKFindMemoryType` traps; it does not return null
It ends in `LLGL_TRAP`, so a "try this memory type, fall back to that one" pattern written
against it never falls back — it hard-crashes. Probe with the non-trapping `VKHasMemoryType` /
`VKDeviceMemoryManager::SupportsMemoryType` instead.

### Adding a field to a public struct requires regenerating the wrappers
The C99/C#/Go wrappers are generated, and `C99TypeAssertions.cpp` asserts struct offsets — so an
unmirrored field breaks the build wherever `LLGL_BUILD_WRAPPER_C99` is on (the Android build has
it on; the default Windows build does not, which is how one slipped through). Regenerate with
`scripts/WrapperGen`; never hand-edit the generated files.

## If unsure

If you are uncertain about style or behavior:
1. follow the nearest established pattern in the same directory,
2. minimize the scope of the change,
3. preserve existing formatting unless there is a strong reason not to,
4. add a short note describing the tradeoff or limitation.

## Suggested review checklist

Before finalizing a change:
- [ ] Public API impact reviewed
- [ ] Examples still make sense
- [ ] Tests added or updated where appropriate
- [ ] Formatting matches local LLGL style
- [ ] Comments and documentation are accurate
- [ ] Platform/backend implications considered
