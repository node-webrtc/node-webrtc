# Building from source on Windows

Unfortunately, building node-webrtc from source on Windows is a bit trickier than on Linux or MacOS, because there is not a reproducible build environment. Also because libwebrtc itself seems a bit broken on this platform.

## Things you should do before building

These are things I had to do to get `npm run build` working on Windows. If you miss a step, the build might fail.

### CMake setup

+ Add the Visual Studio Clang directory to your `%PATH%`. The directory is `C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\Llvm\x64\bin\`.
+ Add the Windows SDK installation directory to your `%PATH%`. An example would be `C:\Program Files (x86)\Windows Kits\10\bin\10.0.22621.0\x64`.
+ Modify `C:\Program Files\CMake\share\cmake-3.26\Modules\Windows-Clang.cmake` to not have `-fuse-ld=lld-link`.
+ Modify `node_modules/cmake-js/lib/toolset.js`, line 184, to have the flag `-DELAYLOAD:NODE.EXE` instead of `/DELAYLOAD:NODE.EXE`.
+ Set the environment variables `CC=clang.exe` and `CXX=clang++.exe`.

### Source code modifications

+ In `build-win32-x64/external/libwebrtc/download/src`:
  + In `third_party/abseil-cpp/absl/meta/type_traits.h`, lines 482 and 493, comment out the assertions that trigger for the "`std::pair` is not trivially constructible" error.
  + In `rtc_base/third_party/sigslot/sigslot.h`, line 295, make `pmethod` always be an array size 24.
+ In `build-win32-x64/external/catch2/src`:
  + In `single_include/catch2/catch.hpp`, line 8722, modify this to always be 32768.
 
## Things to do for a debug build
You'll need to set the environment variable `DEBUG=1` if you want symbols. On PowerShell, this can be done with `$env:DEBUG=1`. Note that CMake will build these into the _same directory_, which is a bit annoying. TODO come up with a better way to switch between debug/release builds.
