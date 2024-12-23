#pragma once
#include "dbg.h"
#include "winapi.h"

enum class HdrStatus {
  NotSupported,
  Enabled,
  Disabled,
  Unknown,
};

class Display {
  LUID adapterId;
  std::uint32_t targetId;
  HdrStatus hdrStatus;
  std::wstring name;

 public:
  Display(const DISPLAYCONFIG_PATH_INFO& displayInfo)
      : adapterId(displayInfo.sourceInfo.adapterId),
        targetId(displayInfo.targetInfo.id) {
    updateHdrStatus();

    auto targetName = GetDisplayName(adapterId, targetId);
    name = (targetName.flags.friendlyNameFromEdid
                ? targetName.monitorFriendlyDeviceName
                : L"Unknown");
  }

  const std::wstring& getName() const { return name; }
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
    static bool UseGetAdvancedColorInfo2 = true;

    if (UseGetAdvancedColorInfo2) {
      auto status = tryGetAdvancedColorInfo2();
      if (status) {
        hdrStatus = *status;
        return;
      }
    }

    UseGetAdvancedColorInfo2 = false;

    dbg("Calling GetAdvancedColorInfo(_, {})", targetId);

    auto getColorInfo = GetAdvancedColorInfo(adapterId, targetId);

    dbg("  getColorInfo.advancedColorSupported: {}",
        getColorInfo.advancedColorSupported == 1);
    dbg("  getColorInfo.advancedColorEnabled: {}",
        (int)getColorInfo.advancedColorEnabled);

    if (!getColorInfo.advancedColorSupported) {
      hdrStatus = HdrStatus::NotSupported;
    } else if (getColorInfo.advancedColorEnabled) {
      hdrStatus = HdrStatus::Enabled;
    } else {
      hdrStatus = HdrStatus::Disabled;
    }
  }

  std::optional<HdrStatus> tryGetAdvancedColorInfo2() {
    dbg("Calling GetAdvancedColorInfo2(_, {})", targetId);
    try {
      auto getColorInfo = GetAdvancedColorInfo2(adapterId, targetId);
      dbg("  getColorInfo.highDynamicRangeSupported: {}",
          getColorInfo.highDynamicRangeSupported == 1);
      dbg("  getColorInfo.activeColorMode: {}",
          (int)getColorInfo.activeColorMode);
      if (!getColorInfo.highDynamicRangeSupported) {
        return HdrStatus::NotSupported;
      } else {
        switch (getColorInfo.activeColorMode) {
          case DISPLAYCONFIG_ADVANCED_COLOR_MODE_SDR:
          case DISPLAYCONFIG_ADVANCED_COLOR_MODE_WCG:
            return HdrStatus::Disabled;
          case DISPLAYCONFIG_ADVANCED_COLOR_MODE_HDR:
            return HdrStatus::Enabled;
          default:
            return HdrStatus::Unknown;
        }
      }
    } catch (const std::exception& e) {
      dbg("GetAdvancedColorInfo2 returned an error. {}", e.what());
    }
    return {};
  }
};