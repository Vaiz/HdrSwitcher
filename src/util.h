#pragma once
#include <Windows.h>

#include <algorithm>
#include <codecvt>
#include <exception>
#include <format>
#include <iostream>
#include <print>
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

std::string to_utf8(std::wstring_view wstr) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.to_bytes(wstr.data());
}

std::string to_upper(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(),
                 [](char c) -> char { return std::toupper(c); });
  return str;
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

std::string FormatLUID(LUID luid) {
  return format("{:016x}-{:08x}", luid.HighPart, luid.LowPart);
}
