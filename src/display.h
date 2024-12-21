#pragma once
#include "winapi.h"

enum HdrStatus {
  NotSupported,
  Enabled,
  Disabled,
};

class Display {
  LUID adapterId;
  std::uint32_t targetId;
  HdrStatus hdrStatus;
  std::optional<std::wstring> name;

 public:
  Display(const DISPLAYCONFIG_PATH_INFO& displayInfo)
      : adapterId(displayInfo.sourceInfo.adapterId),
        targetId(displayInfo.targetInfo.id) {
    updateHdrStatus();
  }

  const std::wstring& getName() {
    if (!name) {
      auto targetName = GetDisplayName(adapterId, targetId);
      name = (targetName.flags.friendlyNameFromEdid
                  ? targetName.monitorFriendlyDeviceName
                  : L"Unknown");
    }
    return *name;
  }
  const std::uint32_t getTargetId() const { return targetId; }
  const LUID getAdapterId() const { return adapterId; }

  void enableHdr() {
    SetAdvancedColorInfo(adapterId, targetId, true);
    updateHdrStatus();
  }
  void disableHdr() {
    SetAdvancedColorInfo(adapterId, targetId, false);
    updateHdrStatus();
  }
  void toggleHdr() {
    if (!isHdrSupported()) {
      throw std::runtime_error("The display does not support HDR");
    }
    SetAdvancedColorInfo(adapterId, targetId, !isHdrEnabled());
    updateHdrStatus();
  }
  HdrStatus getHdrStatus() const { return hdrStatus; }
  bool isHdrEnabled() const { return hdrStatus == HdrStatus::Enabled; }
  bool isHdrSupported() const { return hdrStatus != HdrStatus::NotSupported; }

  static vec<Display> QueryAllDisplays() {
    return QueryDisplayConfigImpl() |
           std::views::transform(
               [](const auto& info) { return Display(info); }) |
           std::ranges::to<std::vector>();
  }

 private:
  void updateHdrStatus() {
    auto getColorInfo = GetAdvancedColorInfo(adapterId, targetId);
    if (!getColorInfo.advancedColorSupported) {
      hdrStatus = HdrStatus::NotSupported;
    } else if (getColorInfo.advancedColorEnabled) {
      hdrStatus = HdrStatus::Enabled;
    } else {
      hdrStatus = HdrStatus::Disabled;
    }
  }
};