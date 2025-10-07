# Quick Start Guide

Get started with bscm-sdbus-cpp in 5 minutes!

## Installation

### 1. Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y bluez libbluetooth-dev libsdbus-c++-dev cmake g++
```

**Fedora/RHEL:**
```bash
sudo dnf install -y bluez bluez-libs-devel sdbus-cpp-devel cmake gcc-c++
```

**Arch Linux:**
```bash
sudo pacman -S bluez sdbus-cpp cmake gcc
```

### 2. Build

```bash
git clone https://github.com/WeekendWar/bscm-sdbus-cpp.git
cd bscm-sdbus-cpp
mkdir build && cd build
cmake ..
make
```

### 3. Run

```bash
sudo ./bscm
```

## First Steps

### Scan for Devices

1. At the main menu, press `1` and Enter
2. Wait 5 seconds for scanning to complete
3. View the list of discovered devices

### Connect to a Device

1. After scanning, press `3` and Enter
2. Enter the device number from the scan results
3. Wait for connection to complete (MTU of 250 bytes is requested automatically)

### Explore Services

1. After connecting, press `6` and Enter
2. View the list of GATT services
3. Press `7` and Enter to see characteristics
4. Enter the service number to explore

### Read Data

1. Press `8` and Enter
2. Select a characteristic number
3. View the hex data returned

### Enable Notifications

1. Press `10` and Enter
2. Select a characteristic that supports notifications
3. Watch real-time data appear on screen
4. Press Enter to stop

## Common Service UUIDs

| Service           | UUID                                   |
|-------------------|----------------------------------------|
| Battery           | 0000180f-0000-1000-8000-00805f9b34fb  |
| Heart Rate        | 0000180d-0000-1000-8000-00805f9b34fb  |
| Device Info       | 0000180a-0000-1000-8000-00805f9b34fb  |
| Blood Pressure    | 00001810-0000-1000-8000-00805f9b34fb  |
| Temperature       | 00001809-0000-1000-8000-00805f9b34fb  |

## Troubleshooting

**"No Bluetooth adapter found"**
```bash
sudo systemctl start bluetooth
sudo systemctl enable bluetooth
```

**"Permission denied"**
```bash
sudo usermod -a -G bluetooth $USER
# Then log out and back in
```

**Device won't connect**
- Ensure device is in pairing mode
- Try pairing first: `bluetoothctl pair XX:XX:XX:XX:XX:XX`

## Next Steps

- Read [USAGE_EXAMPLES.md](USAGE_EXAMPLES.md) for detailed usage scenarios
- Check [ARCHITECTURE.md](ARCHITECTURE.md) to understand the internals
- View [README.md](README.md) for complete documentation

## Getting Help

If you encounter issues:
1. Check that BlueZ is running: `systemctl status bluetooth`
2. Verify your adapter works: `bluetoothctl show`
3. Test with bluetoothctl first to isolate issues
4. Check the BlueZ logs: `journalctl -u bluetooth -f`

## Example Session

```
$ sudo ./bscm
Using Bluetooth adapter: /org/bluez/hci0

=== Bluetooth Device Manager ===
1.  Scan for all devices
...
Choice: 1

Starting device scan...
Scanning for 5 seconds...
Discovery stopped

Found 2 device(s):
1. AA:BB:CC:DD:EE:FF (My BLE Device) [Disconnected, Not Paired]
2. 11:22:33:44:55:66 (Sensor) [Disconnected, Paired]

=== Bluetooth Device Manager ===
...
Choice: 3

Available devices:
1. AA:BB:CC:DD:EE:FF (My BLE Device)
2. 11:22:33:44:55:66 (Sensor)

Select device number: 1
Connecting to device: AA:BB:CC:DD:EE:FF
Device connected successfully
Requesting MTU of 250 bytes...
Successfully connected to AA:BB:CC:DD:EE:FF
```

That's it! You're now ready to explore Bluetooth devices with bscm-sdbus-cpp.
