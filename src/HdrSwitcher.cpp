#include "util.h"

#include "display.h"
#include "filters.h"
#include "winapi.h"

#include <argparse/argparse.hpp>

std::string_view HdrStatusToString(HdrStatus status) {
  switch (status) {
    case HdrStatus::NotSupported:
      return "SDR (HDR not supported)";
    case HdrStatus::Enabled:
      return "HDR";
    case HdrStatus::Disabled:
      return "SDR";
    case HdrStatus::Unknown:
      return "Unknown";
  }
  unreachable();
}

void PrintDisplayInfo(Display& display) {
  std::println("Target ID     : {}", display.getTargetId());
  std::println("Target name   : {}", display.getName());
  std::println("Adapter ID    : {}", FormatLUID(display.getAdapterId()));
  std::println("HDR status    : {}", HdrStatusToString(display.getHdrStatus()));
  std::cout << std::endl;
}

void ListDisplays() {
  auto displays = Display::QueryAllDisplays();
  for (auto& display : displays) {
    PrintDisplayInfo(display);
    std::println();
  }
}

void PrintDisplayMode(std::unique_ptr<IFilter> filter) {
  auto displays = Display::QueryAllDisplays();
  displays = filter->Apply(displays);

  for (const auto& display : displays) {
    std::cout << (display.isHdrEnabled() ? "HDR" : "SDR") << "\n";
  }
}

enum class Operation {
  enable,
  disable,
  toggle,
};

void ChangeHDR(Operation oper, std::unique_ptr<IFilter> filter) {
  auto displays = Display::QueryAllDisplays();
  displays = filter->Apply(displays);

  for (auto& display : displays) {
    PrintDisplayInfo(display);

    switch (oper) {
      case Operation::enable:
        display.enableHdr();
        break;
      case Operation::disable:
        display.disableHdr();
        break;
      case Operation::toggle:
        display.toggleHdr();
        break;
    }

    std::println("New status    : {}",
                 HdrStatusToString(display.getHdrStatus()));
    std::println();
  }
  std::wcout.flush();
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
    argparse::ArgumentParser program("HDRSwitcher", "1.3");
    program.add_epilog(
        "You can specify a monitor by using "
        "`--all`, `--index`, `--id`, or `--name` argument after a command.\n"
        "For more information, use `HDRSwitcher <command> --help`.");

    program.add_argument("--debug")
        .help("Enables debug output")
        .default_value(false)
        .implicit_value(true);

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

    if (program.is_used("debug")) {
      EnableDebugOutput();
    }

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
