#pragma once
#include "util.h"

// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-querydisplayconfig#examples

vec<DISPLAYCONFIG_PATH_INFO> QueryDisplayConfigImpl() {
  vec<DISPLAYCONFIG_PATH_INFO> paths;
  vec<DISPLAYCONFIG_MODE_INFO> modes;
  constexpr UINT32 flags = QDC_ONLY_ACTIVE_PATHS | QDC_VIRTUAL_MODE_AWARE;
  LONG result = ERROR_SUCCESS;

  while (true) {
    // Determine how many path and mode structures to allocate
    UINT32 pathCount, modeCount;
    result = ::GetDisplayConfigBufferSizes(flags, &pathCount, &modeCount);

    if (result != ERROR_SUCCESS) {
      ThrowWinErr("GetDisplayConfigBufferSizes returned an error",
                  HRESULT_FROM_WIN32(result));
    }

    // Allocate the path and mode arrays
    paths.resize(pathCount);
    modes.resize(modeCount);

    // Get all active paths and their modes
    result = ::QueryDisplayConfig(flags, &pathCount, paths.data(), &modeCount,
                                  modes.data(), nullptr);

    // The function may have returned fewer paths/modes than estimated
    paths.resize(pathCount);
    modes.resize(modeCount);

    switch (result) {
      case ERROR_SUCCESS:
        return paths;

      // It's possible that between the call to GetDisplayConfigBufferSizes and
      // QueryDisplayConfig that the display state changed, so loop on the case
      // of ERROR_INSUFFICIENT_BUFFER.
      case ERROR_INSUFFICIENT_BUFFER:
        continue;

      default:
        ThrowWinErr("QueryDisplayConfig returned an error",
                    HRESULT_FROM_WIN32(result));
    }
  }

  unreachable();
}

DISPLAYCONFIG_TARGET_DEVICE_NAME GetDisplayName(LUID adapterId, UINT32 id) {
  DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {};
  targetName.header.adapterId = adapterId;
  targetName.header.id = id;
  targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
  targetName.header.size = sizeof(targetName);
  auto result = ::DisplayConfigGetDeviceInfo(&targetName.header);

  if (result != ERROR_SUCCESS) {
    ThrowWinErr("DisplayConfigGetDeviceInfo returned an error",
                HRESULT_FROM_WIN32(result));
  }

  return targetName;
}

DISPLAYCONFIG_ADAPTER_NAME GetAdapterName(LUID adapterId) {
  DISPLAYCONFIG_ADAPTER_NAME adapterName = {};
  adapterName.header.adapterId = adapterId;
  adapterName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADAPTER_NAME;
  adapterName.header.size = sizeof(adapterName);

  auto result = ::DisplayConfigGetDeviceInfo(&adapterName.header);

  if (result != ERROR_SUCCESS) {
    ThrowWinErr("DisplayConfigGetDeviceInfo returned an error",
                HRESULT_FROM_WIN32(result));
  }

  return adapterName;
}

DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO GetAdvancedColorInfo(LUID adapterId,
                                                           UINT32 id) {
  DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO colorInfo = {};
  colorInfo.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
  colorInfo.header.size = sizeof(colorInfo);
  colorInfo.header.adapterId = adapterId;
  colorInfo.header.id = id;
  auto err = ::DisplayConfigGetDeviceInfo(
      (DISPLAYCONFIG_DEVICE_INFO_HEADER*)&colorInfo);
  if (err != ERROR_SUCCESS) {
    ThrowWinErr("DisplayConfigGetDeviceInfo failed with error. ",
                HRESULT_FROM_WIN32(err));
  }

  return colorInfo;
}

DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO_2 GetAdvancedColorInfo2(LUID adapterId,
                                                              UINT32 id) {
  DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO_2 colorInfo = {};
  colorInfo.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO_2;
  colorInfo.header.size = sizeof(colorInfo);
  colorInfo.header.adapterId = adapterId;
  colorInfo.header.id = id;
  auto err = ::DisplayConfigGetDeviceInfo(
      (DISPLAYCONFIG_DEVICE_INFO_HEADER*)&colorInfo);
  if (err != ERROR_SUCCESS) {
    ThrowWinErr("DisplayConfigGetDeviceInfo failed with error. ",
                HRESULT_FROM_WIN32(err));
  }

  return colorInfo;
}

void SetAdvancedColorInfo(LUID adapterId,
                          UINT32 id,
                          bool advancedColorEnabled) {
  DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE setColorState{};
  setColorState.header.type =
      DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE;
  setColorState.header.size = sizeof(DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE);
  setColorState.header.adapterId = adapterId;
  setColorState.header.id = id;
  setColorState.enableAdvancedColor = advancedColorEnabled ? TRUE : FALSE;

  auto err = ::DisplayConfigSetDeviceInfo(
      (DISPLAYCONFIG_DEVICE_INFO_HEADER*)&setColorState);
  if (err != ERROR_SUCCESS) {
    ThrowWinErr("DisplayConfigSetDeviceInfo failed with error", err);
  }
}