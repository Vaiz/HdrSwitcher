#include <Windows.h>

#include <exception>
#include <format>
#include <iostream>
#include <source_location>
#include <string>
#include <vector>

#include <CLI/CLI.hpp>

// https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-querydisplayconfig#examples

[[noreturn]] void ThrowWinErr(
    std::string_view message,
    HRESULT errCode,
    std::source_location src = std::source_location::current()) {
  throw std::runtime_error(std::format("{}. Error code: {}, Line: {}", message,
                                       errCode, src.line()));
}

[[noreturn]] inline void unreachable() {
  // Uses compiler specific extensions if possible.
  // Even if no extension is used, undefined behavior is still raised by
  // an empty function body and the noreturn attribute.
  __assume(false);
}

std::wstring FormatLUID(LUID luid) {
  return std::format(L"{:016x}-{:08x}", luid.HighPart, luid.LowPart);
}

std::vector<DISPLAYCONFIG_PATH_INFO> QueryDisplayConfigImpl() {
  std::vector<DISPLAYCONFIG_PATH_INFO> paths;
  std::vector<DISPLAYCONFIG_MODE_INFO> modes;
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

void PrintDisplayInfo(LUID adapterId, UINT32 id) {
  DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = GetDisplayName(adapterId, id);

  // Find the adapter device name
  DISPLAYCONFIG_ADAPTER_NAME adapterName = GetAdapterName(adapterId);

  std::wcout << L"Target ID   : " << id << L"\n";
  std::wcout << L"Target name : "
             << (targetName.flags.friendlyNameFromEdid
                     ? targetName.monitorFriendlyDeviceName
                     : L"Unknown")
             << L"\n";

  std::wcout << L"Adapter ID  : " << FormatLUID(adapterId) << L"\n";
  std::wcout << L"Adapter path: " << adapterName.adapterDevicePath << L"\n";
}

void ListDisplays() {
  const auto paths = QueryDisplayConfigImpl();

  // For each active path
  for (auto& path : paths) {
    PrintDisplayInfo(path.targetInfo.adapterId, path.targetInfo.id);
  }
}

void PrintDisplayMode() {
  auto pathArray = QueryDisplayConfigImpl();

  // Get the first display's adapter ID and source ID
  LUID adapterId = pathArray.at(0).targetInfo.adapterId;
  UINT32 targetId = pathArray.at(0).targetInfo.id;

  // Get the current advanced color info of the first display
  auto getColorInfo = GetAdvancedColorInfo(adapterId, targetId);

  std::wcout << (getColorInfo.advancedColorEnabled ? "HDR" : "SDR");
}

enum class Operation {
  enable,
  disable,
  toggle,
};

void ChangeHDR(Operation oper) {
  auto pathArray = QueryDisplayConfigImpl();

  // Get the first display's adapter ID and source ID
  LUID adapterId = pathArray.at(0).targetInfo.adapterId;
  UINT32 targetId = pathArray.at(0).targetInfo.id;
  PrintDisplayInfo(adapterId, targetId);

  // Get the current advanced color info of the first display
  auto getColorInfo = GetAdvancedColorInfo(adapterId, targetId);

  // Check if the display supports HDR
  if (!getColorInfo.advancedColorSupported) {
    throw std::runtime_error("The display does not support HDR");
  }

  bool enableAdvancedColor = [&] {
    switch (oper) {
      case Operation::enable:
        return true;
      case Operation::disable:
        return false;
      case Operation::toggle:
        return getColorInfo.advancedColorEnabled == FALSE;
    }
    unreachable();
  }();

  // Set the advanced color state to enable/disable HDR
  SetAdvancedColorInfo(adapterId, targetId, enableAdvancedColor);

  if (enableAdvancedColor) {
    std::wcout << L"HDR is enabled for the first display" << std::endl;
  } else {
    std::wcout << L"HDR is disabled for the first display" << std::endl;
  }
}

int main(int argc, char** argv) {
  try {
    CLI::App app{"HDRSwitcher"};
    app.require_subcommand(1);

    app.add_subcommand("list", "List available displays")
        ->callback(ListDisplays);
    app.add_subcommand("status", "Print current display's mode")
        ->callback(PrintDisplayMode);
    app.add_subcommand("enable", "Enable HDR")->callback([] {
      ChangeHDR(Operation::enable);
    });
    app.add_subcommand("disable", "Disable HDR")->callback([] {
      ChangeHDR(Operation::disable);
    });
    app.add_subcommand("toggle", "Toggle HDR settings")->callback([] {
      ChangeHDR(Operation::toggle);
    });

    CLI11_PARSE(app, argc, argv);
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
}
