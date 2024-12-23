#pragma once

static bool IsDebugOutputEnabled = false;

void EnableDebugOutput() {
  IsDebugOutputEnabled = true;
}

template <typename... Args>
void dbg(std::format_string<Args...> fmt, Args&&... args) {
  if (IsDebugOutputEnabled) {
    std::print("[DBG] ");
    std::println(fmt, std::forward<Args>(args)...);
  }
}