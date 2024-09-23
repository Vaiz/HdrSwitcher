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

## Examples

### Help
```
PS C:\> .\HdrSwitcher.exe
Usage: HDRSwitcher [--help] [--version] {disable,enable,list,status,toggle}

PS C:\> .\HdrSwitcher.exe enable --help
Usage: enable [--help] [--version] [[--all]|[--index VAR]|[--id VAR]|[--name VAR]]

Enable HDR

Optional arguments:
  -h, --help     shows help message and exits
  -v, --version  prints version information and exits
  --all          Apply to all displays
  --index        Choose display by its index received from `list` command. The first index is 0
  --id           Choose display by its id received from `list` command
  --name         Choose display by its name received from `list` command
```

### List displays
```
PS C:\> .\HdrSwitcher.exe list
Target ID   : 8388688
Target name : Acer XV275K
Adapter ID  : 0000000000000000-00000000
Adapter path: \\?\PCI#VEN_8086&BLAHBLAHBLAH

Target ID   : 12345
Target name : AOC Q27G3XMN
Adapter ID  : 0000000000000000-00000000
Adapter path: \\?\PCI#VEN_8086&BLAHBLAHBLAH
```

### --all
```
PS C:\> .\HdrSwitcher.exe status --all
SDR
SDR
```
```
PS C:\> .\HdrSwitcher.exe toggle --all
HDR is enabled for display with id 8388688
HDR is enabled for display with id 12345
```

### --index
```
PS C:\> .\HdrSwitcher.exe disable --index 0
HDR is disabled for display with id 8388688
```

### --id
```
PS C:\> .\HdrSwitcher.exe disable --id 12345
HDR is disabled for display with id 12345
```

### --name
```
PS C:\> .\HdrSwitcher.exe toggle --name "AOC Q27G3XMN"
HDR is enabled for display with id 12345
```
