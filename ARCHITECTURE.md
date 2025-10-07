# Architecture Documentation

## Overview

This application provides a C++ interface to BlueZ (Linux Bluetooth stack) using the sdbus-c++ library for D-Bus communication. It enables scanning, connecting, and interacting with Bluetooth Low Energy (BLE) devices through a command-line interface.

## Components

### 1. BluetoothManager Class

**File**: `src/BluetoothManager.cpp`, `include/BluetoothManager.h`

The core component that handles all Bluetooth operations through D-Bus.

#### Key Responsibilities:
- D-Bus connection management to BlueZ service
- Bluetooth adapter discovery and initialization
- Device discovery and filtering
- GATT service and characteristic operations
- Notification handling and event processing

#### Public Interface:

**Device Discovery:**
- `startDiscovery(serviceUUID)` - Begin scanning for devices, optionally filtered by service
- `stopDiscovery()` - Stop active scanning
- `getDevices(filterServiceUUID)` - Retrieve list of discovered devices

**Device Management:**
- `connectDevice(address)` - Establish connection to a device
- `disconnectDevice(address)` - Terminate connection
- `removeDevice(address)` - Remove/forget a paired device

**MTU Configuration:**
- `requestMTU(address, mtu)` - Request specific MTU size (default: 250 bytes)

**GATT Operations:**
- `getServices(deviceAddress)` - List GATT services
- `getCharacteristics(servicePath)` - List characteristics for a service
- `readCharacteristic(path)` - Read characteristic value
- `writeCharacteristic(path, data)` - Write data to characteristic

**Notifications:**
- `enableNotifications(path, callback)` - Enable and register callback for notifications
- `disableNotifications(path)` - Disable notifications
- `processEvents(timeout)` - Process pending D-Bus events

#### D-Bus Interfaces Used:

1. **org.bluez.Adapter1** - Bluetooth adapter control
   - StartDiscovery, StopDiscovery, RemoveDevice
   
2. **org.bluez.Device1** - Device management
   - Connect, Disconnect, device properties
   
3. **org.bluez.GattService1** - GATT service representation
   - UUID, characteristics listing
   
4. **org.bluez.GattCharacteristic1** - GATT characteristic operations
   - ReadValue, WriteValue, StartNotify, StopNotify
   
5. **org.freedesktop.DBus.ObjectManager** - Object discovery
   - GetManagedObjects for listing devices, services, characteristics
   
6. **org.freedesktop.DBus.Properties** - Property access
   - Get, GetAll, Set for object properties

### 2. BluetoothCLI Class

**File**: `src/main.cpp`

Interactive command-line interface for user interaction.

#### Key Responsibilities:
- Menu presentation and user input handling
- State management (connected device, cached services, etc.)
- Display formatting for device lists, characteristics, and data
- Coordination between user commands and BluetoothManager operations

#### State Management:
- `m_connectedDevice` - Currently connected device address
- `m_cachedDevices` - Results from last device scan
- `m_cachedServices` - Services for connected device
- `m_cachedCharacteristics` - Characteristics for selected service
- `m_notifyActive` - Flag for notification listening state

## Data Flow

### Device Connection Flow

```
User selects "Connect to device"
    ↓
CLI displays cached devices
    ↓
User selects device number
    ↓
CLI calls BluetoothManager::connectDevice()
    ↓
BluetoothManager calls D-Bus Device1.Connect()
    ↓
Polls device properties until Connected == true
    ↓
BluetoothManager calls requestMTU(250)
    ↓
Returns success/failure to CLI
```

### Notification Flow

```
User enables notifications on characteristic
    ↓
CLI registers callback function
    ↓
BluetoothManager::enableNotifications()
    ↓
Registers PropertiesChanged signal handler
    ↓
Calls D-Bus GattCharacteristic1.StartNotify()
    ↓
Event processing thread starts
    ↓
When notification arrives:
    - D-Bus signal triggers callback
    - Callback prints data to terminal
    ↓
User presses Enter to stop
    ↓
Event thread terminates
```

## Threading Model

The application uses a simple threading model:

1. **Main Thread**: Runs the CLI event loop, handles user input
2. **Event Processing Thread**: Created when notifications are enabled
   - Calls `processPendingRequest()` in a loop
   - Processes D-Bus events and triggers callbacks
   - Terminates when `m_notifyActive` becomes false

## Error Handling

The application uses multiple layers of error handling:

1. **sdbus::Error** - Caught and logged for D-Bus operation failures
2. **std::exception** - Caught at top level for fatal errors
3. **Return codes** - Boolean returns for success/failure indication
4. **User feedback** - All errors displayed to user via std::cerr

## BlueZ D-Bus Object Paths

Example object hierarchy:
```
/org/bluez                              (Root)
  └── /org/bluez/hci0                   (Adapter)
      └── /org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX  (Device)
          ├── service0001               (GATT Service)
          │   ├── char0001              (Characteristic)
          │   └── char0002              (Characteristic)
          └── service0002               (GATT Service)
              └── char0003              (Characteristic)
```

## Build System

**CMake Configuration** (`CMakeLists.txt`):
- C++17 standard required
- Finds sdbus-c++ via pkg-config
- Compiles with `-Wall -Wextra` for warnings
- Links against sdbus-c++ and systemd libraries

## Dependencies

### External Libraries:
- **sdbus-c++ (>=1.0)** - C++ bindings for D-Bus
- **libsystemd** - System service management (required by sdbus-c++)

### System Requirements:
- **BlueZ (>=5.0)** - Linux Bluetooth protocol stack
- **D-Bus** - Inter-process communication system

## Memory Management

- **Smart Pointers**: Uses `std::unique_ptr` for owned resources
- **Connection Management**: sdbus::IConnection managed via unique_ptr
- **Proxy Objects**: Created on-demand for D-Bus operations
- **Callback Storage**: std::map stores notification callbacks by path

## Security Considerations

1. **Permissions**: Requires root or bluetooth group membership for D-Bus access
2. **Input Validation**: User hex input validated before conversion
3. **Path Validation**: Device addresses converted to BlueZ path format
4. **Error Handling**: D-Bus errors caught to prevent crashes

## Performance

- **Async Operations**: Device connection polls with 200ms intervals
- **Discovery Timeout**: Default 5-second scan period
- **Event Processing**: 100ms request processing with 50ms sleep in notification thread
- **Caching**: Device/service/characteristic lists cached to avoid repeated D-Bus calls

## Extensibility

The architecture allows for easy extension:

1. **Additional Operations**: Add new methods to BluetoothManager
2. **Custom Filters**: Modify discovery filters in `startDiscovery()`
3. **Data Formats**: Update display formatting in CLI methods
4. **Notification Handling**: Modify callback logic for custom processing

## Testing Approach

While automated tests are not included, manual testing should cover:

1. Device discovery with and without service filters
2. Connection/disconnection sequences
3. Service and characteristic enumeration
4. Read/write operations with various data sizes
5. Notification enable/disable cycles
6. Error handling (no adapter, device not found, etc.)

## Known Limitations

1. **Single Device**: Only one device connection at a time
2. **MTU Negotiation**: Limited support, depends on BlueZ version
3. **Pairing**: No automatic pairing; devices should be paired via bluetoothctl first
4. **Signal Strength**: RSSI not displayed in device list
5. **Multiple Adapters**: Uses first adapter found only
