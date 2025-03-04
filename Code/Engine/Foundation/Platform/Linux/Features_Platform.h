#pragma once

#include <cstdio>
#include <malloc.h>
#include <pthread.h>
#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>

// unset common macros
#ifdef min
#  undef min
#endif
#ifdef max
#  undef max
#endif

#define EZ_PLATFORM_NAME "Linux"

#undef EZ_PLATFORM_LINUX
#define EZ_PLATFORM_LINUX EZ_ON

#undef EZ_PLATFORM_LITTLE_ENDIAN
#define EZ_PLATFORM_LITTLE_ENDIAN EZ_ON

#undef EZ_PLATFORM_PATH_SEPARATOR
#define EZ_PLATFORM_PATH_SEPARATOR '/'

/// If set to 1, the POSIX file implementation will be used. Otherwise a platform specific implementation must be available.
#undef EZ_USE_POSIX_FILE_API
#define EZ_USE_POSIX_FILE_API EZ_ON

/// If set to one linux posix extensions such as pipe2, dup3, etc are used.
#undef EZ_USE_LINUX_POSIX_EXTENSIONS
#define EZ_USE_LINUX_POSIX_EXTENSIONS EZ_ON

/// Iterating through the file system is not supported
#undef EZ_SUPPORTS_FILE_ITERATORS
#define EZ_SUPPORTS_FILE_ITERATORS EZ_ON

/// Getting the stats of a file (modification times etc.) is supported.
#undef EZ_SUPPORTS_FILE_STATS
#define EZ_SUPPORTS_FILE_STATS EZ_ON

/// Directory watcher is supported
#undef EZ_SUPPORTS_DIRECTORY_WATCHER
#define EZ_SUPPORTS_DIRECTORY_WATCHER EZ_ON

/// Memory mapping a file is supported.
#undef EZ_SUPPORTS_MEMORY_MAPPED_FILE
#define EZ_SUPPORTS_MEMORY_MAPPED_FILE EZ_ON

/// Shared memory IPC is supported.
#undef EZ_SUPPORTS_SHARED_MEMORY
#define EZ_SUPPORTS_SHARED_MEMORY EZ_ON

/// Whether dynamic plugins (through DLLs loaded/unloaded at runtime) are supported
#undef EZ_SUPPORTS_DYNAMIC_PLUGINS
#define EZ_SUPPORTS_DYNAMIC_PLUGINS EZ_ON

/// Whether applications can access any file (not sandboxed)
#undef EZ_SUPPORTS_UNRESTRICTED_FILE_ACCESS
#define EZ_SUPPORTS_UNRESTRICTED_FILE_ACCESS EZ_ON

/// Whether file accesses can be done through paths that do not match exact casing
#undef EZ_SUPPORTS_CASE_INSENSITIVE_PATHS
#define EZ_SUPPORTS_CASE_INSENSITIVE_PATHS EZ_OFF

/// Whether writing to files with very long paths is supported / implemented
#undef EZ_SUPPORTS_LONG_PATHS
#define EZ_SUPPORTS_LONG_PATHS EZ_ON

/// Whether starting other processes is supported.
#undef EZ_SUPPORTS_PROCESSES
#define EZ_SUPPORTS_PROCESSES EZ_ON

/// Whether inter-process communication via pipes is supported
#undef EZ_SUPPORTS_IPC
#define EZ_SUPPORTS_IPC EZ_ON

// SIMD support
#undef EZ_SIMD_IMPLEMENTATION

#if EZ_ENABLED(EZ_PLATFORM_ARCH_X86)
#  if __SSE4_1__ && __SSSE3__
#    define EZ_SIMD_IMPLEMENTATION EZ_SIMD_IMPLEMENTATION_SSE
#    define EZ_SSE_LEVEL EZ_SSE_41
#  else
#    define EZ_SIMD_IMPLEMENTATION EZ_SIMD_IMPLEMENTATION_FPU
#  endif
#elif EZ_ENABLED(EZ_PLATFORM_ARCH_ARM)
#  define EZ_SIMD_IMPLEMENTATION EZ_SIMD_IMPLEMENTATION_FPU
#else
#  error "Unknown architecture."
#endif
