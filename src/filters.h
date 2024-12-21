#pragma once
#include "winapi.h"

struct IFilter {
  virtual ~IFilter() = default;
  virtual vec<DISPLAYCONFIG_PATH_INFO> Apply(
      vec<DISPLAYCONFIG_PATH_INFO> displays) const = 0;
};

struct AllFilter final : IFilter {
  vec<DISPLAYCONFIG_PATH_INFO> Apply(
      vec<DISPLAYCONFIG_PATH_INFO> displays) const {
    return displays;
  }
};

struct IndexFilter final : IFilter {
  size_t index;

  IndexFilter(size_t index) : index(index) {}

  vec<DISPLAYCONFIG_PATH_INFO> Apply(
      vec<DISPLAYCONFIG_PATH_INFO> displays) const {
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

  vec<DISPLAYCONFIG_PATH_INFO> Apply(
      vec<DISPLAYCONFIG_PATH_INFO> displays) const {
    auto iter = std::ranges::find_if(
        displays, [this](const DISPLAYCONFIG_PATH_INFO& display) {
          return display.targetInfo.id == targetId;
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

  DisplayNameFilter(std::string displayName) : displayName(to_upper(displayName)) {}

  vec<DISPLAYCONFIG_PATH_INFO> Apply(
      vec<DISPLAYCONFIG_PATH_INFO> displays) const {
    auto view =
        displays |
        std::views::filter([this](const DISPLAYCONFIG_PATH_INFO& display) {
          auto name = GetDisplayName(display.targetInfo.adapterId,
                                     display.targetInfo.id);

          return to_upper(to_utf8(name.monitorFriendlyDeviceName)) == displayName;
        });

    if (view.empty()) {
      throw std::logic_error(
          format("Cannot find display with name [{}]", displayName));
    }

    return vec<DISPLAYCONFIG_PATH_INFO>(view.begin(), view.end());
  }
};