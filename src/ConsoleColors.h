#pragma once

#include <cstdlib>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

// Change these palette entries to any of the colors below.
namespace ConsoleColors {
inline constexpr const char *reset = "\033[0m";
inline constexpr const char *red = "\033[31m";
inline constexpr const char *green = "\033[32m";
inline constexpr const char *yellow = "\033[33m";
inline constexpr const char *blue = "\033[34m";
inline constexpr const char *magenta = "\033[35m";
inline constexpr const char *cyan = "\033[36m";
inline constexpr const char *white = "\033[37m";

inline constexpr const char *error = red;
inline constexpr const char *status = green;
inline constexpr const char *tick = cyan;
inline constexpr const char *statistics = yellow;
inline constexpr const char *city = magenta;

// Do not send escape sequences to redirected output or when NO_COLOR is set.
inline bool Enable(bool standardError = false)
{
    if (std::getenv("NO_COLOR") != nullptr) return false;
#ifdef _WIN32
    const int descriptor = standardError ? 2 : 1;
    if (!_isatty(descriptor)) return false;
    const HANDLE handle = GetStdHandle(standardError ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
    DWORD mode;
    return handle != INVALID_HANDLE_VALUE && handle != nullptr && GetConsoleMode(handle, &mode)
        && SetConsoleMode(handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#else
    return isatty(standardError ? STDERR_FILENO : STDOUT_FILENO) != 0;
#endif
}

inline const char *Use(bool enabled, const char *color) { return enabled ? color : ""; }
}
