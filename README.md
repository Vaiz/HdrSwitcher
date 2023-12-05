# HdrSwitcher

## Description
This application provides a command-line interface for managing High Dynamic Range (HDR) settings on Windows systems. It allows users to list available displays, enable or disable HDR, and toggle HDR settings.

## Features
- **List Displays**: Lists available display devices.
- **Enable HDR**: Activates HDR on supported displays.
- **Disable HDR**: Deactivates HDR.
- **Toggle HDR**: Switches HDR state between on and off.

## Limitation
Currently, the program operates only on the first display returned by the system. Future updates may include support for multiple displays.

## Building the Application
Run `build.bat` to compile the application for a release build using CMake.

## Download
To download the latest version of the HDR Control Application, visit the releases section: https://github.com/Vaiz/HdrSwitcher/releases.

## Usage
- `HdrSwitcher.exe list` - Lists displays.
- `HdrSwitcher.exe enable` - Enables HDR.
- `HdrSwitcher.exe disable` - Disables HDR.
- `HdrSwitcher.exe toggle` - Toggles HDR.
