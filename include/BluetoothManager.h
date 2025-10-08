#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <sdbus-c++/sdbus-c++.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

struct DeviceInfo
{
  std::string              address;
  std::string              name;
  std::string              alias;
  bool                     paired;
  bool                     connected;
  bool                     trusted;
  std::vector<std::string> uuids;
  int16_t                  rssi = 0;
};

struct CharacteristicInfo
{
  std::string              path;
  std::string              uuid;
  std::vector<std::string> flags;
};

struct ServiceInfo
{
  std::string                     path;
  std::string                     uuid;
  std::vector<CharacteristicInfo> characteristics;
};

class BluetoothManager
{
public:
  BluetoothManager();
  ~BluetoothManager();

  // Device scanning and discovery
  void                    startDiscovery(const std::string& serviceUUID = "");
  void                    stopDiscovery();
  std::vector<DeviceInfo> getDevices(const std::string& filterServiceUUID = "");

  // Device operations
  bool connectDevice(const std::string& address);
  bool disconnectDevice(const std::string& address);
  bool removeDevice(const std::string& address);

  // MTU operations
  bool requestMTU(const std::string& deviceAddress, uint16_t mtu);

  // GATT operations
  std::vector<ServiceInfo>        getServices(const std::string& deviceAddress);
  std::vector<CharacteristicInfo> getCharacteristics(
    const std::string& servicePath);

  // Characteristic operations
  bool enableNotifications(
    const std::string&                               characteristicPath,
    std::function<void(const std::vector<uint8_t>&)> callback);
  bool disableNotifications(const std::string& characteristicPath);
  bool writeCharacteristic(const std::string&          characteristicPath,
                           const std::vector<uint8_t>& data);
  std::vector<uint8_t> readCharacteristic(
    const std::string& characteristicPath);

  // Utility
  std::string getAdapterPath();
  void        processEvents(int timeoutMs = 100);

private:
  std::unique_ptr<sdbus::IConnection>                   m_connection;
  std::string                                           m_adapterPath;
  std::map<std::string, std::unique_ptr<sdbus::IProxy>> m_deviceProxies;
  std::map<std::string, std::function<void(const std::vector<uint8_t>&)>>
    m_notifyCallbacks;

  std::string findAdapter();
  std::string getDevicePath(const std::string& address);
  std::map<std::string, sdbus::Variant> getProperties(
    const std::string& objectPath,
    const std::string& interface);
  void                     setProperty(const std::string&    objectPath,
                                       const std::string&    interface,
                                       const std::string&    property,
                                       const sdbus::Variant& value);
  std::vector<std::string> getManagedObjects(const std::string& basePath);
};

#endif  // BLUETOOTH_MANAGER_H
