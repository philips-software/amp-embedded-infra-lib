# This toolchain file enables cross-compilation from non-Windows hosts to Windows hosts.
# It assumes the Windows SDK and CRT are installed at a path specified by WINDOWS_SDK_ROOT.
#
# The recommended way of installing the SDK and CRT is by using xwin (https://github.com/Jake-Shadle/xwin):
# $ xwin --accept-license splat --preserve-ms-arch-notation
#
# xwin patches the SDK and CRT for case-sensitive filesystems; the --preserve-ms-arch-notation is
# necessary to make the /winsdkdir and /vctoolsdir options of clang work.

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_TRY_COMPILE_CONFIGURATION Release)

set(WINDOWS_SDK_ROOT "/winsdk" CACHE PATH "Path to a Windows SDK and CRT installation")
set(CMAKE_RC_STANDARD_INCLUDE_DIRECTORIES ${WINDOWS_SDK_ROOT}/sdk/include/um ${WINDOWS_SDK_ROOT}/sdk/include/ucrt ${WINDOWS_SDK_ROOT}/sdk/include/shared)

find_program(CMAKE_C_COMPILER NAMES clang-cl REQUIRED)
find_program(CMAKE_CXX_COMPILER NAMES clang-cl REQUIRED)
find_program(CMAKE_AR NAMES llvm-lib REQUIRED)

add_compile_options(--target=x86_64-pc-windows-msvc -Wno-error -fuse-ld=lld /winsdkdir ${WINDOWS_SDK_ROOT}/sdk /vctoolsdir ${WINDOWS_SDK_ROOT}/crt)
add_link_options(/manifest:no -libpath:${WINDOWS_SDK_ROOT}/sdk/lib/um/x64 -libpath:${WINDOWS_SDK_ROOT}/sdk/lib/ucrt/x64 -libpath:${WINDOWS_SDK_ROOT}/crt/lib/x64)
