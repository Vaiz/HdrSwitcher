#pragma once
#include "display.h"
#include "winapi.h"

struct IFilter {
  virtual ~IFilter() = default;
  virtual vec<Display> Apply(vec<Display> displays) const = 0;
};

struct AllFilter final : IFilter {
  vec<Display> Apply(vec<Display> displays) const { return displays; }
};

struct IndexFilter final : IFilter {
  size_t index;

  IndexFilter(size_t index) : index(index) {}

  vec<Display> Apply(vec<Display> displays) const {
    if (displays.size() <= index) {
      throw std::logic_error(format(
          "Cannot find display with index {}. There is/are only {} display(s)",
          index, displays.size()));
    }

    return {displays.at(index)};
  }
};

struct TargetIdFilter final : IFilter {
  uint32_t targetId;

  TargetIdFilter(uint32_t targetId) : targetId(targetId) {}

  vec<Display> Apply(vec<Display> displays) const {
    auto iter = std::ranges::find_if(displays, [this](const Display& display) {
      return display.getTargetId() == targetId;
    });

    if (displays.end() == iter) {
      throw std::logic_error(
          format("Cannot find display with target id {}", targetId));
    }

    return {*iter};
  }
};

struct DisplayNameFilter final : IFilter {
  std::string displayName;

  DisplayNameFilter(std::string displayName)
      : displayName(to_upper(displayName)) {}

  vec<Display> Apply(vec<Display> displays) const {
    auto view = displays | std::views::filter([this](Display& display) {
                  auto name = display.getName();
                  return to_upper(to_utf8(name)) == displayName;
                });

    if (view.empty()) {
      throw std::logic_error(
          format("Cannot find display with name [{}]", displayName));
    }

    return view | std::ranges::to<std::vector>();
  }
};