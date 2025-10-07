# bscm-sdbus-cpp
C++ app to use bluez and sdbus-c++ to connect to a bscm

## Description

This application provides a command-line interface for managing Bluetooth Low Energy (BLE) devices using BlueZ and sdbus-c++. It allows you to:

- Scan for Bluetooth devices (all devices or filtered by service UUID)
- Connect, disconnect, and forget devices
- Request MTU (Maximum Transmission Unit) of 250 bytes after connection
- Discover and list GATT services and characteristics
- Read and write characteristic values
- Enable/disable notifications on characteristics with output printed to terminal
- Send commands to device characteristics

## Features

- **Device Scanning**: Discover all available Bluetooth devices or filter by specific service UUID
- **Device Management**: Connect, disconnect, and remove (forget) devices
- **MTU Configuration**: Automatically requests 250-byte MTU after connection
- **GATT Operations**: Browse services and characteristics, read/write values
- **Notifications**: Enable notifications on characteristics and display data in real-time
- **Interactive CLI**: User-friendly menu-driven interface

## Requirements

- Linux system with BlueZ installed
- sdbus-c++ library
- CMake 3.10 or higher
- C++17 compatible compiler

## Installation

### Dependencies

Install the required dependencies:

```bash
# On Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y bluez libbluetooth-dev libsdbus-c++-dev cmake g++

# On Fedora/RHEL
sudo dnf install -y bluez bluez-libs-devel sdbus-cpp-devel cmake gcc-c++

# On Arch Linux
sudo pacman -S bluez sdbus-cpp cmake gcc
```

### Building

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

Run the application:

```bash
sudo ./bscm
```

**Note**: Root privileges (sudo) are typically required to access Bluetooth functionality through D-Bus.

### Main Menu Options

1. **Scan for all devices**: Discovers all nearby Bluetooth devices
2. **Scan for devices with specific service**: Filter scan by service UUID
3. **Connect to device**: Connect to a discovered device (automatically requests 250-byte MTU)
4. **Disconnect from device**: Disconnect from currently connected device
5. **Forget device**: Remove device from system (unpair)
6. **List services**: Show GATT services of connected device
7. **List characteristics**: Show characteristics for a selected service
8. **Read characteristic**: Read value from a characteristic
9. **Write to characteristic**: Send data to a characteristic
10. **Enable notifications**: Start receiving notifications from a characteristic
11. **Disable notifications**: Stop notifications from a characteristic
0. **Exit**: Quit the application

### Example Workflow

1. Start the application: `sudo ./bscm`
2. Scan for devices (option 1 or 2)
3. Connect to a device (option 3)
4. List services (option 6)
5. List characteristics of a service (option 7)
6. Enable notifications on a characteristic (option 10) - notifications will be printed to terminal
7. Write commands to a characteristic (option 9)

### Writing Data

When writing to a characteristic, enter hex bytes separated by spaces:
```
Enter hex bytes (space-separated, e.g., 01 02 03): 48 65 6c 6c 6f
```

## Architecture

The application consists of:

- **BluetoothManager**: C++ class wrapping BlueZ D-Bus API via sdbus-c++
- **BluetoothCLI**: Interactive command-line interface
- **main.cpp**: Application entry point

## License

MIT License - See LICENSE file for details
