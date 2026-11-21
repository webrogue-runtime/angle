# WebRogue WebGPU backend changes

This document lists every change in the `webrogue-webgpu` branch that is not in
upstream `origin/main`. The branch makes ANGLE build for WebAssembly (WASI) and
run on WebRogue, where the default rendering backend is WebGPU (implemented by
the WebRogue host, not by a vendored Dawn) instead of Vulkan.

Each entry has a stable identifier (`WR-NN`). Comments in the source reference
these identifiers, e.g. `// [WR-28] Front-face/y-flip handling.`, so a change can
be traced from the code back to this document.

The identifiers are grouped as follows:

| Range | Topic |
|-------|-------|
| WR-01 … WR-13 | WASI platform and build plumbing |
| WR-14 … WR-17 | Optional Vulkan WebRogue backend |
| WR-18 … WR-25 | WebGPU backend build, process table and window surface |
| WR-26 … WR-34 | WebGPU correctness fixes required by real applications |

The `WGPU_STRLEN` problems fixed in WR-22 are an upstream ANGLE bug (the code
passes the `WGPU_STRLEN` sentinel to `std::string` as a length); they are not
caused by the WebRogue host.

---

## WASI platform and build plumbing

### [WR-01] `gni/angle.gni`: WASI build configuration

* `wasm32` is added to the list of 32-bit CPUs. Without it the
  `angle_64bit_current_cpu` argument has no branch and the `assert` fires.
* SwiftShader is disabled on WASI (`!is_wasi`). SwiftShader needs a native
  Vulkan/CPU stack that does not exist in a WASI sandbox.
* Perfetto tracing is disabled on WASI (`&& !is_wasi`), since the Perfetto
  client is not built for that target.

### [WR-02] `include/EGL/eglplatform.h`: native EGL types for WASI

`__wasi__` gets `void *` for `EGLNativeDisplayType`, `EGLNativePixmapType` and
`EGLNativeWindowType`. The WebRogue host hands opaque window handles to the
guest, so a pointer is the natural representation.

### [WR-03] `src/common/platform.h`: identify WebRogue

`__wasi__` now defines both `ANGLE_PLATFORM_POSIX` (WASI libc provides enough
POSIX surface for ANGLE's common code) and `ANGLE_PLATFORM_WEBROGUE` (used to
select the WebRogue-specific backends and defaults). `ANGLE_PLATFORM_WEBROGUE`
is the switch that makes WebGPU the default display type in WR-16.

### [WR-04] `src/common/system_utils_linux.cpp`: WASI clock and thread name

* `CLOCK_MONOTONIC_RAW` is not available in WASI libc; use `CLOCK_MONOTONIC`
  instead in `GetCurrentSystemTime` and `GetCurrentSystemTimeNs`.
* `pthread_setname_np` is not available, so `SetCurrentThreadName` is a no-op on
  WASI.

### [WR-05] `src/common/system_utils_posix.cpp`: WASI stubs

Several POSIX APIs used by ANGLE do not exist in a WASI sandbox: dynamic
loading (`dlopen`/`dlsym`/`dlclose`), working-directory access
(`getcwd`/`chdir`), process CPU accounting, temporary files, memory protection
and page-fault handlers. The branch guards all of them with `#ifdef __wasi__`
and either returns a fixed value (`GetPageSize()` returns 16 KiB) or marks the
function unreachable. Headers that are unavailable on WASI (`grp.h`, `pwd.h`,
`sys/mman.h`, `sys/wait.h`, `sys/resource.h`) are guarded too.

This is a single logical change; the file-level comment at the top of the file
covers every `__wasi__` guard in it.

### [WR-06] `src/libGLESv2.gni`: compile POSIX system utilities on WASI

`system_utils_linux.cpp` and `system_utils_posix.cpp` are added to
`libangle_common_sources` for `is_wasi`. Together with WR-04/WR-05 they provide
the common utility implementation the rest of ANGLE links against.

### [WR-07] `src/common/vulkan/BUILD.gn`: no custom Vulkan loader on WASI

`angle_use_custom_libvulkan` and the custom-loader data dependencies are
disabled on WASI. WebRogue provides Vulkan entry points directly (see WR-10), so
there is no loader shared library to build or load.

### [WR-08] `src/libANGLE/renderer/vulkan/BUILD.gn`: libwr4c include path

`include_dirs` is initialised to an empty list so the WebRogue include path can
be appended, and `//third_party/libwr4c/include` is added for `is_wasi`. The
Vulkan backend includes `<libwr4c/wr4c.h>` in the WebRogue display/surface.

### [WR-09] `src/libANGLE/renderer/vulkan/vulkan_backend.gni`: WebRogue sources

The four new Vulkan WebRogue files (WR-14, WR-15) are added to
`vulkan_backend_sources` when `is_wasi`.

### [WR-10] `src/libANGLE/renderer/vulkan/vk_renderer.cpp`: static ICD entry point

There is no `libvulkan` to `dlopen` on WASI. Instead the branch declares the
ICD's `vk_icdGetInstanceProcAddr` (provided by the WebRogue host) and uses it
directly as the loader's `vkGetInstanceProcAddr`.

### [WR-11] `src/libANGLE/renderer/vulkan/MemoryObjectVk.cpp`: unsupported external memory

Zircon VMO import/duplication and external-image creation are not available on
WASI. The affected functions are compiled out (or return `Stop`/`UNREACHABLE`)
on `__wasi__`; they are not used by the WebRogue Vulkan path.

### [WR-12] `src/libANGLE/renderer/vulkan/SyncVk.cpp`: unsupported native fences

`SyncHelperNativeFence::serverWait` and `dupNativeFenceFD` depend on native
fence file descriptors, which do not exist on WASI. They are compiled out and
return `Stop` on `__wasi__`.

### [WR-13] Explicit `<iostream>` includes

`RewritePipelineVariables.cpp`, `ProgramWgpu.cpp` and `wgpu_wgsl_util.cpp`
already use `std::cout`, but relied on transitive includes that WASI libc++ does
not provide. The branch includes `<iostream>` explicitly.

---

## Optional Vulkan WebRogue backend

The Vulkan WebRogue backend is kept in the branch but is no longer the default
(WR-16 selects WebGPU). It exists so the WebRogue host's `VK_WEBROGUE_surface`
path can still be built and tested.

### [WR-14] New `DisplayVkWebrogue` (`renderer/vulkan/webrogue/`)

A `DisplayVk` subclass that:
* creates `WindowSurfaceVkWebrogue` window surfaces (WR-15),
* advertises `VK_WEBROGUE_SURFACE_EXTENSION_NAME` as its WSI extension,
* generates configs with `GL_BGRA8_EXT` on desktop-like hosts and
  `GL_RGBA8`/`GL_RGB8` on Android (the surface's native formats),
* reports itself as available unconditionally.

### [WR-15] New `WindowSurfaceVkWebrogue` (`renderer/vulkan/webrogue/`)

A `WindowSurfaceVk` subclass that creates the Vulkan surface through
`vkCreateSurfaceWEBROGUE`, passing the window id from
`wr4c_get_vulkan_window_id()`, and queries the window size with
`wr4c_physical_size()`.

### [WR-16] `src/libANGLE/Display.cpp`: default backend and factory

* `GetDisplayTypeFromEnvironment()` returns
  `EGL_PLATFORM_ANGLE_TYPE_WEBGPU_ANGLE` when `ANGLE_PLATFORM_WEBROGUE` and
  `ANGLE_ENABLE_WGPU` are defined, *before* the Vulkan fallback. This is what
  makes WebGPU the default on WebRogue.
* `CreateDisplayFromAttribs()` knows how to construct `DisplayVkWebrogue` when
  the Vulkan display type is requested explicitly.

### [WR-17] `src/libANGLE/renderer/vulkan/DisplayVk_api.h`: declarations

Declares `IsVulkanWebrogueDisplayAvailable()` and
`CreateVulkanWebrogueDisplay()` for `ANGLE_PLATFORM_WEBROGUE`, mirroring the
existing Fuchsia/Apple/Android factories.

---

## WebGPU backend build, process table and window surface

### [WR-18] `src/libANGLE/renderer/wgpu/BUILD.gn`: build without a Dawn checkout

The upstream WGPU backend depends on `third_party/dawn` for headers, the
`DawnProcTable` implementation and the native backend. WebRogue does not vendor
Dawn. For `is_wasi` the branch:
* drops the `dawn_features.gni` import and the `angle_dawn_dir` assert,
* removes the Dawn deps,
* adds `//third_party/libwr4c/include` (WR-25).

Non-WASI builds keep the original Dawn dependencies.

### [WR-19] `src/libANGLE/renderer/wgpu/wgpu_sources.gni`: WebRogue surface sources

Adds `webrogue/WindowSurfaceWgpuWebrogue.{cpp,h}` (WR-24) to
`wgpu_backend_sources` when `is_wasi`.

### [WR-20] `src/libANGLE/renderer/wgpu/wgpu_proc_utils.cpp`: process table from libwr4c

`GetDefaultProcTable()` returns `*wr4c_get_dawn_proc_table()` instead of
`dawn::native::GetProcs()`. `libwr4c` builds a `DawnProcTable` whose function
pointers call the standard WebGPU C API implemented by the WebRogue host over
`wasi:webgpu`.

### [WR-21] `src/libANGLE/renderer/wgpu/wgpu_utils.h`: object list and helper declaration

* The `ANGLE_WGPU_OBJECTS_X` macro lists the Dawn-only shared-memory object
  types (`SharedBufferMemory`, `SharedFence`, `SharedTextureMemory`). The
  standard `webgpu.h` used on WebRogue does not define them, so a
  `ANGLE_PLATFORM_WEBROGUE` variant without those entries is provided.
* Declares `WgpuStringViewToString()` (WR-22).

### [WR-22] `src/libANGLE/renderer/wgpu/wgpu_utils.cpp`: string views and caps

* Adds `WgpuStringViewToString()`, which honours the `WGPU_STRLEN` sentinel:
  `{nullptr, WGPU_STRLEN}` is the null view and `{data, WGPU_STRLEN}` is a
  NUL-terminated string. Upstream ANGLE passed `WGPU_STRLEN` to `std::string`'s
  length and threw `std::length_error`; this is an upstream bug, not a host
  issue. `ErrorScope::PopScope()` uses the helper.
* `GenerateCaps()` changes are described in WR-29.

### [WR-23] `DisplayWgpu.cpp` and `ProgramWgpu.cpp`: decode string views

All adapter/device callback messages and shader compilation messages are decoded
with `WgpuStringViewToString()` (WR-22) instead of `std::string(data, length)`.

### [WR-24] New `WindowSurfaceWgpuWebrogue` (`renderer/wgpu/webrogue/`)

A `WindowSurfaceWgpu` subclass that:
* creates the WebGPU surface with `wr4c_create_webgpu_surface()` from the
  native window handle,
* reports the window size from `wr4c_physical_size()`.

It is the WebGPU counterpart of WR-15 and is what lets EGL window surfaces work
on WebRogue without Dawn's native windowing.

### [WR-25] Vendored `third_party/libwr4c/include`

ANGLE needs Dawn-compatible declarations at build time. Since Dawn is not
vendored, the branch adds the subset of headers `libwr4c` exposes:

* `webgpu/webgpu.h` — the standard WebGPU C API (upstream header).
* `dawn/dawn_proc.h`, `dawn/dawn_proc_table.h` — the `DawnProcTable` shape the
  WGPU backend already uses.
* `libwr4c/wr4c.h` — WebRogue host API (window id, physical size, OS family,
  Vulkan surface creation).
* `libwr4c/wr4c_webgpu.h` — `wr4c_get_dawn_proc_table()` and
  `wr4c_create_webgpu_surface()`.

These headers are vendored copies; they are not generated by the ANGLE build.

---

## WebGPU correctness fixes required by real applications

### [WR-26] Emulate A8/L8/L8A8 as RGBA8

WebGPU has no texture-view swizzle, so `R8_UNORM`/`R8G8_UNORM` cannot be sampled
as alpha/luminance the way GL expects. `wgpu_format_map.json` maps
`A8_UNORM`, `L8_UNORM` and `L8A8_UNORM` to `R8G8B8A8_UNORM`, and
`wgpu_format_table_autogen.cpp` is updated to match. The shader translator then
uses its LA8/L8/A8 load functions to expand the red channel as needed. This is
what fixes the garbled raylib font atlas.

`wgpu_format_map.json` cannot carry comments (strict JSON), so the entry is
documented here and in the header of the generated table.

### [WR-27] Depth writes enabled by default

`RenderPipelineDesc`'s constructor now sets
`mDepthStencilState.depthWriteEnabled = 1`, matching GL's initial state. With
the previous zero-initialised value, depth writes were disabled until the
application explicitly changed the depth mask.

### [WR-28] Front-face winding and the y-flip

WebGPU determines the front face in framebuffer coordinates, whose y axis points
down, while GL uses y-up. The WGSL translator flips `gl_Position.y` only for the
default framebuffer. Vulkan compensates by deriving the front face from
`isYFlipEnabledForDrawFBO()`; the WGPU backend did not, so geometry drawn into
an FBO was culled when it should not have been (and vice versa).

`RenderPipelineDesc::setFrontFace(frontFace, yFlip)` now swaps CCW and CW when
`yFlip` is false. `ContextWgpu::syncState()` recomputes the front face when the
draw framebuffer changes (`DIRTY_BIT_DRAW_FRAMEBUFFER_BINDING`) and on
`DIRTY_BIT_FRONT_FACE`. This is what fixed the empty/garbled raylib render
texture.

### [WR-29] Vertex-buffer slots and attribute limits

WebGPU limits the number of vertex buffers per pipeline (typically 8), but GL
attribute indices can go up to `maxVertexAttributes`. Upstream ANGLE gave every
attribute its own buffer slot, so high attribute indices exceeded the limit.

The branch:
* packs enabled attributes into consecutive WebGPU buffer slots, in attribute
  index order (`RenderPipelineDesc::getVertexBufferSlot()`);
* tracks the set of attributes present in the pipeline
  (`getEnabledAttributeMask()`), and uses it for invalidation and for binding
  vertex buffers (`ContextWgpu`);
* widens `PackedVertexAttribute::format` to 6 bits (and shrinks the padding
  accordingly) so all WebGPU vertex formats fit;
* sets `maxVertexAttributes` from `limitsWgpu.maxVertexAttributes` and
  `maxVertexAttribBindings` from `maxVertexAttributes` (ANGLE core requires
  `maxVertexAttribBindings >= maxVertexAttributes`). The number of *enabled*
  attributes is still limited by `maxVertexBuffers`.

### [WR-30] Emulate disabled (current-value) vertex attributes

WebGPU has no GL "current value" for a disabled attribute: every shader input
must have a vertex buffer. Vulkan emulates this with
`updateDefaultAttribs`; the WGPU backend previously asserted/unimplemented.

`VertexArrayWgpu::ensureDefaultAttrib()` uploads the current generic value into
a one-element buffer and marks the attribute as enabled in the pipeline. It is
called from `syncDirtyAttrib` when the shader reads a disabled attribute, and
from `ContextWgpu::handleDirtyRenderPipelineDesc()` /
`handleDirtyVertexBuffers()` when the program's active attribute set changes
without a vertex-array dirty bit.

### [WR-31] Remap buffers when a write falls outside the mapped range

`BufferWgpu::setData`/`setSubData` used to map the buffer only if it was not
mapped at all. A second write to a different range therefore wrote through a
mapping that did not cover it. `BufferHelper::coversMapRange()` checks the
aligned mapped range, and the setters unmap and re-map when the requested range
is not covered.

### [WR-32] Release surface textures before destroying/reconfiguring the swapchain

`wgpu-hal` panics if a swapchain is destroyed while a `SurfaceTexture` holding
one of its acquire semaphores is alive. `WindowSurfaceWgpu::destroy()` and
`configureSurface()` now reset the color/depth attachments before dropping or
reconfiguring the surface. This was required for clean shutdown and window
resize.

### [WR-33] Report `EGL_BUFFER_DESTROYED`

`WindowSurfaceWgpu::getSwapBehavior()` returns `EGL_BUFFER_DESTROYED` instead of
calling `UNIMPLEMENTED()`. WebGPU surface contents are not preserved across
presentation, which matches the destroyed semantics.

### [WR-34] `wgpu_command_buffer.cpp`: include `common/debug.h`

The command-recording logging macros use `ANGLE_LOG`, which is declared in
`common/debug.h`. The include is added explicitly; it was previously available
transitively.

---

## Non-code differences

These are differences against `origin/main` that are not source changes and are
therefore not annotated in code:

* `build` is a symlink to `../chromium-build` in this checkout. Upstream tracks
  `build` as a directory, so `git diff origin/main` reports it as deleted. The
  symlink is required by the WebRogue build and must be restored if a merge
  recreates `build` as an empty directory (otherwise `gn gen` fails with
  `Unable to load "//build/dotfile_settings.gni"`).
* Submodule pointers (`buildtools`, `testing`, `third_party/*`, `tools/*`) are
  rolled to revisions used by the WebRogue build. They are not ANGLE changes.
* The leftover untracked `angle/angle/` directory in the checkout is not part of
  the branch and is ignored.
