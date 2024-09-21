#pragma once
#include <Windows.h>

#include <codecvt>
#include <exception>
#include <format>
#include <iostream>
#include <ranges>
#include <source_location>
#include <string>
#include <vector>

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

template <typename T>
using vec = std::vector<T>;
using std::format;

std::string to_utf8(const wchar_t* wstr) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.to_bytes(wstr);
}

[[noreturn]] void ThrowWinErr(
    std::string_view message,
    HRESULT errCode,
    std::source_location src = std::source_location::current()) {
  throw std::runtime_error(
      format("{}. Error code: {}, Line: {}", message, errCode, src.line()));
}

[[noreturn]] inline void unreachable() {
  // Uses compiler specific extensions if possible.
  // Even if no extension is used, undefined behavior is still raised by
  // an empty function body and the noreturn attribute.
  __assume(false);
}

std::wstring FormatLUID(LUID luid) {
  return format(L"{:016x}-{:08x}", luid.HighPart, luid.LowPart);
}