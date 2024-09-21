# HdrSwitcher

## Description
This application provides a command-line interface for managing High Dynamic Range (HDR) settings on Windows systems. It allows users to list available displays, enable or disable HDR, and toggle HDR settings.

## Features
- **List Displays**: Lists available display devices.
- **Enable HDR**: Activates HDR on supported displays.
- **Disable HDR**: Deactivates HDR.
- **Toggle HDR**: Switches HDR state between on and off.
- **Status**: Reports whether HDR or SDR is currently enabled.

## Building the Application
Run `build.bat` to compile the application for a release build using CMake.

## Download
To download the latest version of the HDR Control Application, visit the releases section: https://github.com/Vaiz/HdrSwitcher/releases.

## Usage
- `HdrSwitcher.exe list` - Lists displays.
- `HdrSwitcher.exe enable  [[--all]|[--index <i>]|[--id <id>]|[--name <name>]]` - Enables HDR.
- `HdrSwitcher.exe disable [[--all]|[--index <i>]|[--id <id>]|[--name <name>]]` - Disables HDR.
- `HdrSwitcher.exe toggle  [[--all]|[--index <i>]|[--id <id>]|[--name <name>]]` - Toggles HDR.
- `HdrSwitcher.exe status  [[--all]|[--index <i>]|[--id <id>]|[--name <name>]]` - Shows current mode.
- `HdrSwitcher.exe --help` - Shows help.
- `HdrSwitcher.exe <command> --help` - Shows help for specific command.

