#include "BluetoothManager.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

// BlueZ D-Bus constants
const std::string BLUEZ_SERVICE          = "org.bluez";
const std::string ADAPTER_INTERFACE      = "org.bluez.Adapter1";
const std::string DEVICE_INTERFACE       = "org.bluez.Device1";
const std::string GATT_SERVICE_INTERFACE = "org.bluez.GattService1";
const std::string GATT_CHAR_INTERFACE    = "org.bluez.GattCharacteristic1";
const std::string PROPERTIES_INTERFACE   = "org.freedesktop.DBus.Properties";
const std::string OBJECT_MANAGER_INTERFACE =
  "org.freedesktop.DBus.ObjectManager";

const bool        USE_DEFAULT_ADAPTER  = true;
const std::string DEFAULT_ADAPTER_PATH = "/org/bluez/hci1";

BluetoothManager::BluetoothManager()
{
  m_connection  = sdbus::createSystemBusConnection();
  m_adapterPath = findAdapter();

  if (m_adapterPath.empty())
  {
    throw std::runtime_error("No Bluetooth adapter found");
  }

  std::cout << "Using Bluetooth adapter: " << m_adapterPath << std::endl;
}

BluetoothManager::~BluetoothManager()
{
  stopDiscovery();
}

std::string BluetoothManager::findAdapter()
{
  try
  {
    // auto proxy = sdbus::createProxy(*m_connection, BLUEZ_SERVICE,
    // std::string("/"));
    auto proxy = sdbus::createProxy(
      *m_connection, sdbus::ServiceName(BLUEZ_SERVICE), sdbus::ObjectPath{"/"});

    std::map<sdbus::ObjectPath,
             std::map<std::string, std::map<std::string, sdbus::Variant>>>
      objects;
    proxy->callMethod("GetManagedObjects")
      .onInterface(OBJECT_MANAGER_INTERFACE)
      .storeResultsTo(objects);

    // if default path available, then try to use it
    for (const auto& [path, interfaces] : objects)
    {
      if (USE_DEFAULT_ADAPTER &&
          (path == sdbus::ObjectPath{DEFAULT_ADAPTER_PATH}))
      {
        return path;
      }
    }

    // otherwise return the first adapter found
    for (const auto& [path, interfaces] : objects)
    {
      if (interfaces.find(ADAPTER_INTERFACE) != interfaces.end())
      {
        return path;
      }
    }
  }
  catch (const sdbus::Error& e)
  {
    std::cerr << "Error finding adapter: " << e.what() << std::endl;
  }

  return "";
}

std::string BluetoothManager::getAdapterPath()
{
  return m_adapterPath;
}

void BluetoothManager::startDiscovery(const std::string& serviceUUID)
{
  try
  {
    auto adapter = sdbus::createProxy(*m_connection,
                                      sdbus::ServiceName(BLUEZ_SERVICE),
                                      sdbus::ObjectPath(m_adapterPath));

    // Set discovery filter if service UUID is provided
    if (!serviceUUID.empty())
    {
      std::map<std::string, sdbus::Variant> filter;
      std::vector<std::string>              uuids = {serviceUUID};
      filter["UUIDs"]                             = sdbus::Variant(uuids);

      adapter->callMethod("SetDiscoveryFilter")
        .onInterface(ADAPTER_INTERFACE)
        .withArguments(filter);
    }

    adapter->callMethod("StartDiscovery").onInterface(ADAPTER_INTERFACE);
    std::cout << "Discovery started" << std::endl;

    // Give some time for devices to be discovered
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  catch (const sdbus::Error& e)
  {
    std::cerr << "Error starting discovery: " << e.what() << std::endl;
  }
}

void BluetoothManager::stopDiscovery()
{
  try
  {
    auto adapter = sdbus::createProxy(*m_connection,
                                      sdbus::ServiceName(BLUEZ_SERVICE),
                                      sdbus::ObjectPath(m_adapterPath));
    adapter->callMethod("StopDiscovery").onInterface(ADAPTER_INTERFACE);
    std::cout << "Discovery stopped" << std::endl;
  }
  catch (const sdbus::Error& e)
  {
    // Ignore error if discovery is not active
  }
}

std::map<std::string, sdbus::Variant> BluetoothManager::getProperties(
  const std::string& objectPath,
  const std::string& interface)
{
  std::map<std::string, sdbus::Variant> properties;
  sdbus::ObjectPath                     path(objectPath);

  try
  {
    auto proxy = sdbus::createProxy(
      *m_connection, sdbus::ServiceName(BLUEZ_SERVICE), path);
    proxy->callMethod("GetAll")
      .onInterface(PROPERTIES_INTERFACE)
      .withArguments(interface)
      .storeResultsTo(properties);
  }
  catch (const sdbus::Error& e)
  {
    std::cerr << "Error getting properties: " << e.what() << std::endl;
  }

  return properties;
}

void BluetoothManager::setProperty(const std::string&    objectPath,
                                   const std::string&    interface,
                                   const std::string&    property,
                                   const sdbus::Variant& value)
{
  sdbus::ObjectPath path(objectPath);
  try
  {
    auto proxy = sdbus::createProxy(
      *m_connection, sdbus::ServiceName(BLUEZ_SERVICE), path);
    proxy->callMethod("Set")
      .onInterface(PROPERTIES_INTERFACE)
      .withArguments(interface, property, value);
  }
  catch (const sdbus::Error& e)
  {
    std::cerr << "Error setting property: " << e.what() << std::endl;
    throw;
  }
}

std::vector<DeviceInfo> BluetoothManager::getDevices(
  const std::string& filterServiceUUID)
{
  std::vector<DeviceInfo> devices;
  sdbus::ObjectPath       path("/");
  try
  {
    auto proxy = sdbus::createProxy(
      *m_connection, sdbus::ServiceName(BLUEZ_SERVICE), path);

    std::map<sdbus::ObjectPath,
             std::map<std::string, std::map<std::string, sdbus::Variant>>>
      objects;
    proxy->callMethod("GetManagedObjects")
      .onInterface(OBJECT_MANAGER_INTERFACE)
      .storeResultsTo(objects);

    for (const auto& [path, interfaces] : objects)
    {
      auto deviceIt = interfaces.find(DEVICE_INTERFACE);
      if (deviceIt != interfaces.end())
      {
        DeviceInfo  info;
        const auto& props = deviceIt->second;

        // Extract device properties
        if (props.count("Address"))
        {
          info.address = props.at("Address").get<std::string>();
        }
        if (props.count("Name"))
        {
          info.name = props.at("Name").get<std::string>();
        }
        if (props.count("Alias"))
        {
          info.alias = props.at("Alias").get<std::string>();
        }
        if (props.count("Paired"))
        {
          info.paired = props.at("Paired").get<bool>();
        }
        if (props.count("Connected"))
        {
          info.connected = props.at("Connected").get<bool>();
        }
        if (props.count("Trusted"))
        {
          info.trusted = props.at("Trusted").get<bool>();
        }
        if (props.count("UUIDs"))
        {
          info.uuids = props.at("UUIDs").get<std::vector<std::string>>();
        }
        if (props.count("RSSI"))
        {
          info.rssi = props.at("RSSI").get<int16_t>();
        }

        // Filter by service UUID if provided
        if (!filterServiceUUID.empty())
        {
          bool hasService = std::find(info.uuids.begin(),
                                      info.uuids.end(),
                                      filterServiceUUID) != info.uuids.end();
          if (!hasService)
          {
            continue;
          }
        }

        devices.push_back(info);
      }
    }
  }
  catch (const sdbus::Error& e)
  {
    std::cerr << "Error getting devices: " << e.what() << std::endl;
  }

  return devices;
}

std::string BluetoothManager::getDevicePath(const std::string& address)
{
  // Convert address format (XX:XX:XX:XX:XX:XX) to BlueZ format
  // (dev_XX_XX_XX_XX_XX_XX)
  std::string devAddress = address;
  std::replace(devAddress.begin(), devAddress.end(), ':', '_');
  return m_adapterPath + "/dev_" + devAddress;
}

bool BluetoothManager::connectDevice(const std::string& address)
{
  try
  {
    std::string       devicePath = getDevicePath(address);
    sdbus::ObjectPath path(devicePath);
    auto              device = sdbus::createProxy(
      *m_connection, sdbus::ServiceName(BLUEZ_SERVICE), path);

    std::cout << "Connecting to device: " << address << std::endl;
    device->callMethod("Connect").onInterface(DEVICE_INTERFACE);

    // Wait for connection to establish
    for (int i = 0; i < 50; i++)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      auto props = getProperties(devicePath, DEVICE_INTERFACE);
      if (props.count("Connected") && props.at("Connected").get<bool>())
      {
        std::cout << "Device connected successfully" << std::endl;
        return true;
      }
    }

    std::cerr << "Connection timeout" << std::endl;
    return false;
  }
  catch (const sdbus::Error& e)
  {
    std::cerr << "Error connecting to device: " << e.what() << std::endl;
    return false;
  }
}

bool BluetoothManager::disconnectDevice(const std::string& address)
{
  try
  {
    std::string       devicePath = getDevicePath(address);
    sdbus::ObjectPath path(devicePath);
    auto              device = sdbus::createProxy(
      *m_connection, sdbus::ServiceName(BLUEZ_SERVICE), path);

    std::cout << "Disconnecting device: " << address << std::endl;
    device->callMethod("Disconnect").onInterface(DEVICE_INTERFACE);

    std::cout << "Device disconnected successfully" << std::endl;
    return true;
  }
  catch (const sdbus::Error& e)
  {
    std::cerr << "Error disconnecting device: " << e.what() << std::endl;
    return false;
  }
}

bool BluetoothManager::removeDevice(const std::string& address)
{
  try
  {
    std::string       devicePath = getDevicePath(address);
    sdbus::ObjectPath path(m_adapterPath);
    auto              adapter = sdbus::createProxy(
      *m_connection, sdbus::ServiceName(BLUEZ_SERVICE), path);

    std::cout << "Removing (forgetting) device: " << address << std::endl;
    adapter->callMethod("RemoveDevice")
      .onInterface(ADAPTER_INTERFACE)
      .withArguments(sdbus::ObjectPath(devicePath));

    std::cout << "Device removed successfully" << std::endl;
    return true;
  }
  catch (const sdbus::Error& e)
  {
    std::cerr << "Error removing device: " << e.what() << std::endl;
    return false;
  }
}

void BluetoothManager::registerDeviceDisconnectHandler(
  const std::string&                      devicePath,
  std::function<void(const std::string&)> onDisconnect)
{
  auto proxy = sdbus::createProxy(*m_connection,
                                  sdbus::ServiceName(BLUEZ_SERVICE),
                                  sdbus::ObjectPath(devicePath));
  proxy->uponSignal("PropertiesChanged")
    .onInterface("org.freedesktop.DBus.Properties")
    .call([this, devicePath, onDisconnect](
            const std::string&                           interface,
            const std::map<std::string, sdbus::Variant>& changed,
            const std::vector<std::string>&) {
      if (interface == "org.bluez.Device1" && changed.count("Connected"))
      {
        bool connected = changed.at("Connected").get<bool>();
        if (!connected)
        {
          std::cout << "[BluetoothManager] Device disconnected: " << devicePath
                    << std::endl;
          onDisconnect(devicePath);
          // Remove this proxy
          m_deviceDisconnectProxies.erase(devicePath);
        }
      }
    });
  m_deviceDisconnectProxies[devicePath] = std::move(proxy);
}

void BluetoothManager::cleanupDevice(const std::string& devicePath)
{
  // Remove notification proxies and callbacks for all characteristics belonging
  // to device
  std::vector<std::string> to_cleanup;
  for (const auto& [charPath, _] : m_deviceProxies)
  {
    if (charPath.find(devicePath) == 0)
    {
      to_cleanup.push_back(charPath);
    }
  }
  for (const auto& charPath : to_cleanup)
  {
    m_deviceProxies.erase(charPath);
    m_notifyCallbacks.erase(charPath);
  }
}

bool BluetoothManager::requestMTU(const std::string& deviceAddress,
                                  uint16_t           mtu)
{
  try
  {
    std::string       devicePath = getDevicePath(deviceAddress);
    sdbus::ObjectPath path("/");

    // Get all GATT characteristics for the device
    auto proxy = sdbus::createProxy(
      *m_connection, sdbus::ServiceName(BLUEZ_SERVICE), path);

    std::map<sdbus::ObjectPath,
             std::map<std::string, std::map<std::string, sdbus::Variant>>>
      objects;
    proxy->callMethod("GetManagedObjects")
      .onInterface(OBJECT_MANAGER_INTERFACE)
      .storeResultsTo(objects);

    // Find any characteristic belonging to this device and use it to request
    // MTU
    for (const auto& [path, interfaces] : objects)
    {
      std::string pathStr = path;
      if (pathStr.find(devicePath) == 0 &&
          interfaces.find(GATT_CHAR_INTERFACE) != interfaces.end())
      {
        sdbus::ObjectPath charPath(path);

        // Try to acquire MTU through the characteristic
        try
        {
          auto charProxy = sdbus::createProxy(
            *m_connection, sdbus::ServiceName(BLUEZ_SERVICE), charPath);

          // Use AcquireWrite or AcquireNotify with MTU option
          std::map<std::string, sdbus::Variant> options;
          options["MTU"] = sdbus::Variant(mtu);

          // Try AcquireNotify first as it's more commonly available
          sdbus::UnixFd fd;
          uint16_t      resultMtu;
          charProxy->callMethod("AcquireNotify")
            .onInterface(GATT_CHAR_INTERFACE)
            .withArguments(options)
            .storeResultsTo(fd, resultMtu);

          std::cout << "MTU requested: " << mtu << ", negotiated: " << resultMtu
                    << std::endl;
          return true;
        }
        catch (const sdbus::Error& e)
        {
          // AcquireNotify might not be supported, try through device property
          std::cerr << "Note: Direct MTU negotiation not supported, using "
                       "default mechanism"
                    << std::endl;
        }
        break;
      }
    }

    std::cout << "MTU request noted (will be negotiated during GATT operations)"
              << std::endl;
    return true;
  }
  catch (const sdbus::Error& e)
  {
    std::cerr << "Error requesting MTU: " << e.what() << std::endl;
    return false;
  }
}

std::vector<ServiceInfo> BluetoothManager::getServices(
  const std::string& deviceAddress)
{
  std::vector<ServiceInfo> services;

  try
  {
    std::string       devicePath = getDevicePath(deviceAddress);
    sdbus::ObjectPath path("/");
    auto              proxy = sdbus::createProxy(
      *m_connection, sdbus::ServiceName(BLUEZ_SERVICE), path);

    std::map<sdbus::ObjectPath,
             std::map<std::string, std::map<std::string, sdbus::Variant>>>
      objects;
    proxy->callMethod("GetManagedObjects")
      .onInterface(OBJECT_MANAGER_INTERFACE)
      .storeResultsTo(objects);

    for (const auto& [path, interfaces] : objects)
    {
      std::string pathStr = path;
      if (pathStr.find(devicePath) == 0)
      {
        auto serviceIt = interfaces.find(GATT_SERVICE_INTERFACE);
        if (serviceIt != interfaces.end())
        {
          ServiceInfo service;
          service.path = pathStr;

          const auto& props = serviceIt->second;
          if (props.count("UUID"))
          {
            service.uuid = props.at("UUID").get<std::string>();
          }

          services.push_back(service);
        }
      }
    }
  }
  catch (const sdbus::Error& e)
  {
    std::cerr << "Error getting services: " << e.what() << std::endl;
  }

  return services;
}

std::vector<CharacteristicInfo> BluetoothManager::getCharacteristics(
  const std::string& servicePath)
{
  std::vector<CharacteristicInfo> characteristics;

  try
  {
    sdbus::ObjectPath path("/");
    auto              proxy = sdbus::createProxy(
      *m_connection, sdbus::ServiceName(BLUEZ_SERVICE), path);

    std::map<sdbus::ObjectPath,
             std::map<std::string, std::map<std::string, sdbus::Variant>>>
      objects;
    proxy->callMethod("GetManagedObjects")
      .onInterface(OBJECT_MANAGER_INTERFACE)
      .storeResultsTo(objects);

    for (const auto& [path, interfaces] : objects)
    {
      std::string pathStr = path;
      if (pathStr.find(servicePath) == 0 && pathStr != servicePath)
      {
        auto charIt = interfaces.find(GATT_CHAR_INTERFACE);
        if (charIt != interfaces.end())
        {
          CharacteristicInfo characteristic;
          characteristic.path = pathStr;

          const auto& props = charIt->second;
          if (props.count("UUID"))
          {
            characteristic.uuid = props.at("UUID").get<std::string>();
          }
          if (props.count("Flags"))
          {
            characteristic.flags =
              props.at("Flags").get<std::vector<std::string>>();
          }

          characteristics.push_back(characteristic);
        }
      }
    }
  }
  catch (const sdbus::Error& e)
  {
    std::cerr << "Error getting characteristics: " << e.what() << std::endl;
  }

  return characteristics;
}

// In enableNotifications
bool BluetoothManager::enableNotifications(
  const std::string&                               characteristicPath,
  std::function<void(const std::vector<uint8_t>&)> callback)
{
  try
  {
    sdbus::ObjectPath path(characteristicPath);
    // Create and store the proxy so it's not destroyed
    auto charProxy = sdbus::createProxy(
      *m_connection, sdbus::ServiceName(BLUEZ_SERVICE), path);

    charProxy->uponSignal("PropertiesChanged")
      .onInterface(PROPERTIES_INTERFACE)
      .call([this, characteristicPath, callback](
              const std::string&                           interface,
              const std::map<std::string, sdbus::Variant>& changed,
              const std::vector<std::string>& /*invalidated*/) {
        if (interface == GATT_CHAR_INTERFACE && changed.count("Value"))
        {
          auto value = changed.at("Value").get<std::vector<uint8_t>>();
          if (callback)
          {
            callback(value);
          }
        }
      });

    // Store the proxy so it stays alive!
    m_deviceProxies[characteristicPath] = std::move(charProxy);

    // Start notifications
    m_deviceProxies[characteristicPath]
      ->callMethod("StartNotify")
      .onInterface(GATT_CHAR_INTERFACE);

    m_notifyCallbacks[characteristicPath] = callback;
    std::cout << "Notifications enabled for characteristic: "
              << characteristicPath << std::endl;
    return true;
  }
  catch (const sdbus::Error& e)
  {
    std::cerr << "Error enabling notifications for characteristic, "
              << characteristicPath << ": " << e.what() << std::endl;
    return false;
  }
}

bool BluetoothManager::disableNotifications(
  const std::string& characteristicPath)
{
  try
  {
    sdbus::ObjectPath path(characteristicPath);
    auto              charProxy = sdbus::createProxy(
      *m_connection, sdbus::ServiceName(BLUEZ_SERVICE), path);
    charProxy->callMethod("StopNotify").onInterface(GATT_CHAR_INTERFACE);

    m_notifyCallbacks.erase(characteristicPath);
    m_deviceProxies.erase(characteristicPath);
    std::cout << "Notifications disabled for characteristic" << std::endl;
    return true;
  }
  catch (const sdbus::Error& e)
  {
    std::cerr << "Error disabling notifications: " << e.what() << std::endl;
    return false;
  }
}

bool BluetoothManager::writeCharacteristic(
  const std::string&          characteristicPath,
  const std::vector<uint8_t>& data)
{
  try
  {
    sdbus::ObjectPath path(characteristicPath);
    auto              charProxy = sdbus::createProxy(
      *m_connection, sdbus::ServiceName(BLUEZ_SERVICE), path);

    std::map<std::string, sdbus::Variant> options;
    // Default write type is "request" which waits for response

    charProxy->callMethod("WriteValue")
      .onInterface(GATT_CHAR_INTERFACE)
      .withArguments(data, options);

    std::cout << "Written " << data.size() << " bytes to characteristic"
              << std::endl;
    return true;
  }
  catch (const sdbus::Error& e)
  {
    std::cerr << "Error writing characteristic: " << e.what() << std::endl;
    return false;
  }
}

std::vector<uint8_t> BluetoothManager::readCharacteristic(
  const std::string& characteristicPath)
{
  try
  {
    sdbus::ObjectPath path(characteristicPath);
    auto              charProxy = sdbus::createProxy(
      *m_connection, sdbus::ServiceName(BLUEZ_SERVICE), path);

    std::map<std::string, sdbus::Variant> options;
    std::vector<uint8_t>                  value;

    charProxy->callMethod("ReadValue")
      .onInterface(GATT_CHAR_INTERFACE)
      .withArguments(options)
      .storeResultsTo(value);

    std::cout << "Read " << value.size() << " bytes from characteristic"
              << std::endl;
    return value;
  }
  catch (const sdbus::Error& e)
  {
    std::cerr << "Error reading characteristic: " << e.what() << std::endl;
    return {};
  }
}

void BluetoothManager::processEvents(int timeoutMs)
{
  (void)timeoutMs;  // Unused parameter
  m_connection->processPendingEvent();
}
