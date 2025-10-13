#include <atomic>
#include <string>

#include "boot_module/bluetooth_manager.hpp"

namespace boot_module
{
class BluetoothCLI
{
public:
  BluetoothCLI();

  void run();

private:
  std::unique_ptr<BluetoothManager> m_manager;
  bool                              m_running;
  std::string                       m_connectedDevice;
  std::vector<DeviceInfo>           m_cachedDevices;
  std::vector<ServiceInfo>          m_cachedServices;
  std::vector<CharacteristicInfo>   m_cachedCharacteristics;
  std::string                       m_currentServicePath;
  std::atomic<bool>                 m_notifyActive{false};

  void printMainMenu();

  int getChoice();

  std::string getInput(const std::string& prompt);

  void scanDevices();

  void scanDevicesWithService();

  void connectToDevice();

  void disconnectFromDevice();

  void forgetDevice();

  void listServices();

  void listCharacteristics();

  void readCharacteristic();

  void writeCharacteristic();

  void enableNotifications();

  void disableNotifications();
};
}  // namespace boot_module
