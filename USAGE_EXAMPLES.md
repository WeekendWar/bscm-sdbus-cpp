# Usage Examples

This document provides practical examples of using the bscm-sdbus-cpp Bluetooth manager application.

## Prerequisites

Before running the application, ensure:
1. BlueZ is installed and running: `sudo systemctl status bluetooth`
2. Your Bluetooth adapter is enabled: `bluetoothctl show`
3. You have permissions to access D-Bus (run with `sudo` if needed)

## Basic Workflow

### 1. Starting the Application

```bash
sudo ./bscm
```

You should see the main menu:
```
=== Bluetooth Device Manager ===
1.  Scan for all devices
2.  Scan for devices with specific service
3.  Connect to device
...
```

### 2. Scanning for All Devices

Select option `1` from the main menu. The application will:
- Start Bluetooth discovery
- Scan for 5 seconds
- Display all discovered devices with their addresses, names, and connection status

Example output:
```
Found 3 device(s):
1. AA:BB:CC:DD:EE:FF (My Device) [Disconnected, Paired]
2. 11:22:33:44:55:66 (Heart Rate Monitor) [Disconnected, Not Paired]
3. FF:EE:DD:CC:BB:AA (SmartWatch) [Connected, Paired]
```

### 3. Scanning for Devices with Specific Service

Select option `2` to filter devices by service UUID. Common service UUIDs:

- **Battery Service**: `0000180f-0000-1000-8000-00805f9b34fb`
- **Heart Rate**: `0000180d-0000-1000-8000-00805f9b34fb`
- **Device Information**: `0000180a-0000-1000-8000-00805f9b34fb`

Example:
```
Enter service UUID: 0000180f-0000-1000-8000-00805f9b34fb
Scanning for 5 seconds...
Found 1 device(s) with service 0000180f-0000-1000-8000-00805f9b34fb:
1. AA:BB:CC:DD:EE:FF (My Device) [Disconnected]
```

### 4. Connecting to a Device

Select option `3`, then choose a device number from your cached scan results:

```
Select device number: 1
Connecting to device: AA:BB:CC:DD:EE:FF
Device connected successfully
Requesting MTU of 250 bytes...
MTU request noted (will be negotiated during GATT operations)
Successfully connected to AA:BB:CC:DD:EE:FF
```

**Note**: The application automatically requests an MTU of 250 bytes after connection.

### 5. Exploring Services and Characteristics

After connecting, list available GATT services:

Select option `6`:
```
Discovering services...
Found 3 service(s):
1. UUID: 0000180f-0000-1000-8000-00805f9b34fb
   Path: /org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service0001
2. UUID: 0000180a-0000-1000-8000-00805f9b34fb
   Path: /org/bluez/hci0/dev_AA_BB_CC_DD_EE_FF/service0002
...
```

List characteristics for a service with option `7`:
```
Select service number: 1
Found 2 characteristic(s):
1. UUID: 00002a19-0000-1000-8000-00805f9b34fb
   Flags: read notify
2. UUID: 00002a1a-0000-1000-8000-00805f9b34fb
   Flags: read write
```

### 6. Reading a Characteristic

Select option `8`:
```
Select characteristic number: 1
Value (1 bytes): 64
```

This shows the characteristic value in hexadecimal (0x64 = 100 in decimal, representing 100% battery).

### 7. Writing to a Characteristic

Select option `9` and enter hex bytes:
```
Select characteristic number: 2
Enter hex bytes (space-separated, e.g., 01 02 03): 01 FF
Written 2 bytes to characteristic
```

### 8. Enabling Notifications

Select option `10` to receive real-time updates:
```
Select characteristic number: 1
Notifications enabled. Listening for notifications...
Press Enter to return to menu...

>>> Notification received (1 bytes): 63
>>> Notification received (1 bytes): 62
>>> Notification received (1 bytes): 61
```

The notifications are printed to the terminal in real-time. Press Enter to stop listening.

### 9. Disabling Notifications

Select option `11`:
```
Select characteristic number: 1
Notifications disabled.
```

### 10. Disconnecting

Select option `4` to disconnect from the current device:
```
Disconnecting device: AA:BB:CC:DD:EE:FF
Device disconnected successfully
Disconnected from AA:BB:CC:DD:EE:FF
```

### 11. Forgetting a Device

Select option `5` to remove (unpair) a device:
```
Select device number to forget: 1
Removing (forgetting) device: AA:BB:CC:DD:EE:FF
Device removed successfully
Device forgotten successfully.
```

## Common Use Cases

### Monitoring Sensor Data

1. Scan for devices with Heart Rate service: `0000180d-0000-1000-8000-00805f9b34fb`
2. Connect to the device
3. List services and find Heart Rate Service
4. List characteristics and find Heart Rate Measurement characteristic
5. Enable notifications to receive real-time heart rate data

### Controlling a Smart Device

1. Scan for your smart device
2. Connect to it
3. Explore services to find control characteristics
4. Write commands to the appropriate characteristic
5. Read response characteristics to verify commands

### Battery Level Monitoring

1. Connect to any BLE device
2. Find Battery Service (UUID: `0000180f-0000-1000-8000-00805f9b34fb`)
3. Read Battery Level characteristic (UUID: `00002a19-0000-1000-8000-00805f9b34fb`)
4. Enable notifications to monitor battery changes

## Troubleshooting

### "No Bluetooth adapter found"

- Ensure BlueZ is running: `sudo systemctl start bluetooth`
- Check adapter status: `hciconfig` or `bluetoothctl show`

### "Failed to connect to device"

- Ensure device is in pairing/discoverable mode
- Try pairing first with `bluetoothctl pair <address>`
- Check if device is already connected to another system

### "Error enabling notifications"

- Ensure the characteristic supports notifications (check flags)
- Some characteristics require bonding/pairing first

### Permission Denied Errors

- Run the application with sudo: `sudo ./bscm`
- Add your user to the `bluetooth` group: `sudo usermod -a -G bluetooth $USER`

## Tips

- **Service UUIDs**: Use shortened UUIDs for standard Bluetooth services (e.g., `180f` for Battery)
- **Hex Input**: When writing data, use hex format without `0x` prefix
- **MTU**: The application requests 250-byte MTU automatically, but actual MTU depends on device support
- **Caching**: Device lists are cached after scanning, so you don't need to rescan before connecting
