#include "filters.h"
#include "util.h"
#include "winapi.h"

#include <argparse/argparse.hpp>

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

void PrintDisplayMode(std::unique_ptr<IFilter> filter) {
  auto pathArray = QueryDisplayConfigImpl();
  pathArray = filter->Apply(pathArray);

  for (const auto& display : pathArray) {
    LUID adapterId = display.targetInfo.adapterId;
    UINT32 targetId = display.targetInfo.id;

    // Get the current advanced color info of the display
    auto getColorInfo = GetAdvancedColorInfo(adapterId, targetId);

    std::wcout << (getColorInfo.advancedColorEnabled ? "HDR" : "SDR");
  }
}

enum class Operation {
  enable,
  disable,
  toggle,
};

void ChangeHDR(Operation oper, std::unique_ptr<IFilter> filter) {
  auto pathArray = QueryDisplayConfigImpl();
  pathArray = filter->Apply(pathArray);

  for (const auto& display : pathArray) {
    LUID adapterId = display.targetInfo.adapterId;
    UINT32 targetId = display.targetInfo.id;
    PrintDisplayInfo(adapterId, targetId);

    // Get the current advanced color info of the display
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
      std::wcout << L"HDR is enabled for display with id " << targetId
                 << std::endl;
    } else {
      std::wcout << L"HDR is disabled for display with id " << targetId
                 << std::endl;
    }
  }
}

void add_filters(argparse::ArgumentParser& app) {
  auto& group = app.add_mutually_exclusive_group();
  group.add_argument("--all")
      .help("Apply to all displays")
      .default_value(false)
      .implicit_value(true);
  group.add_argument("--index").scan<'i', size_t>().help(
      "Choose display by its index received from `list` command. The first "
      "index is 0");
  group.add_argument("--id").scan<'i', uint32_t>().help(
      "Choose display by its id received from `list` command");
  group.add_argument("--name").help(
      "Choose display by its name received from `list` command");
}
std::unique_ptr<IFilter> InitFilter(
    const argparse::ArgumentParser& subcommand) {
  if (subcommand.is_used("all")) {
    return std::make_unique<AllFilter>();
  } else if (subcommand.is_used("index")) {
    return std::make_unique<IndexFilter>(subcommand.get<size_t>("index"));
  } else if (subcommand.is_used("id")) {
    return std::make_unique<TargetIdFilter>(subcommand.get<uint32_t>("id"));
  } else if (subcommand.is_used("name")) {
    return std::make_unique<DisplayNameFilter>(
        subcommand.get<std::string>("name"));
  } else {
    return std::make_unique<IndexFilter>(0);
  }
}

int main(int argc, char** argv) {
  try {
    argparse::ArgumentParser program("HDRSwitcher", "1.2");

    argparse::ArgumentParser list_command("list");
    list_command.add_description("List available displays");
    program.add_subparser(list_command);

    argparse::ArgumentParser status_command("status");
    status_command.add_description("Print current display's mode");
    add_filters(status_command);
    program.add_subparser(status_command);

    argparse::ArgumentParser enable_command("enable");
    enable_command.add_description("Enable HDR");
    add_filters(enable_command);
    program.add_subparser(enable_command);

    argparse::ArgumentParser disable_command("disable");
    disable_command.add_description("Disable HDR");
    add_filters(disable_command);
    program.add_subparser(disable_command);

    argparse::ArgumentParser toggle_command("toggle");
    toggle_command.add_description("Toggle HDR settings");
    add_filters(toggle_command);
    program.add_subparser(toggle_command);

    program.parse_args(argc, argv);

    if (program.is_subcommand_used(list_command)) {
      ListDisplays();
    } else if (program.is_subcommand_used(status_command)) {
      PrintDisplayMode(InitFilter(status_command));
    } else if (program.is_subcommand_used(enable_command)) {
      ChangeHDR(Operation::enable, InitFilter(enable_command));
    } else if (program.is_subcommand_used(disable_command)) {
      ChangeHDR(Operation::disable, InitFilter(disable_command));
    } else if (program.is_subcommand_used(toggle_command)) {
      ChangeHDR(Operation::toggle, InitFilter(toggle_command));
    } else {
      std::cout << program.usage() << std::endl;
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
}
