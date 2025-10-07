#include "BluetoothManager.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <thread>
#include <atomic>

class BluetoothCLI {
public:
    BluetoothCLI() : m_running(true), m_connectedDevice("") {
        try {
            m_manager = std::make_unique<BluetoothManager>();
        } catch (const std::exception& e) {
            std::cerr << "Failed to initialize Bluetooth Manager: " << e.what() << std::endl;
            throw;
        }
    }

    void run() {
        while (m_running) {
            printMainMenu();
            int choice = getChoice();
            
            switch (choice) {
                case 1:
                    scanDevices();
                    break;
                case 2:
                    scanDevicesWithService();
                    break;
                case 3:
                    connectToDevice();
                    break;
                case 4:
                    disconnectFromDevice();
                    break;
                case 5:
                    forgetDevice();
                    break;
                case 6:
                    listServices();
                    break;
                case 7:
                    listCharacteristics();
                    break;
                case 8:
                    readCharacteristic();
                    break;
                case 9:
                    writeCharacteristic();
                    break;
                case 10:
                    enableNotifications();
                    break;
                case 11:
                    disableNotifications();
                    break;
                case 0:
                    m_running = false;
                    std::cout << "Exiting..." << std::endl;
                    break;
                default:
                    std::cout << "Invalid choice. Please try again." << std::endl;
            }
        }
    }

private:
    std::unique_ptr<BluetoothManager> m_manager;
    bool m_running;
    std::string m_connectedDevice;
    std::vector<DeviceInfo> m_cachedDevices;
    std::vector<ServiceInfo> m_cachedServices;
    std::vector<CharacteristicInfo> m_cachedCharacteristics;
    std::string m_currentServicePath;
    std::atomic<bool> m_notifyActive{false};

    void printMainMenu() {
        std::cout << "\n=== Bluetooth Device Manager ===" << std::endl;
        std::cout << "1.  Scan for all devices" << std::endl;
        std::cout << "2.  Scan for devices with specific service" << std::endl;
        std::cout << "3.  Connect to device" << std::endl;
        std::cout << "4.  Disconnect from device" << std::endl;
        std::cout << "5.  Forget device" << std::endl;
        std::cout << "6.  List services" << std::endl;
        std::cout << "7.  List characteristics" << std::endl;
        std::cout << "8.  Read characteristic" << std::endl;
        std::cout << "9.  Write to characteristic" << std::endl;
        std::cout << "10. Enable notifications" << std::endl;
        std::cout << "11. Disable notifications" << std::endl;
        std::cout << "0.  Exit" << std::endl;
        std::cout << "Choice: ";
    }

    int getChoice() {
        std::string input;
        std::getline(std::cin, input);
        
        try {
            return std::stoi(input);
        } catch (...) {
            return -1;
        }
    }

    std::string getInput(const std::string& prompt) {
        std::cout << prompt;
        std::string input;
        std::getline(std::cin, input);
        return input;
    }

    void scanDevices() {
        std::cout << "\nStarting device scan..." << std::endl;
        m_manager->startDiscovery();
        
        std::cout << "Scanning for 5 seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        m_manager->stopDiscovery();
        
        m_cachedDevices = m_manager->getDevices();
        
        std::cout << "\nFound " << m_cachedDevices.size() << " device(s):" << std::endl;
        for (size_t i = 0; i < m_cachedDevices.size(); i++) {
            const auto& dev = m_cachedDevices[i];
            std::cout << i + 1 << ". " << dev.address;
            if (!dev.name.empty()) {
                std::cout << " (" << dev.name << ")";
            } else if (!dev.alias.empty()) {
                std::cout << " (" << dev.alias << ")";
            }
            std::cout << " [" << (dev.connected ? "Connected" : "Disconnected")
                      << ", " << (dev.paired ? "Paired" : "Not Paired") << "]";
            std::cout << std::endl;
        }
    }

    void scanDevicesWithService() {
        std::string serviceUUID = getInput("Enter service UUID (e.g., 0000180f-0000-1000-8000-00805f9b34fb): ");
        
        std::cout << "\nStarting device scan with service filter..." << std::endl;
        m_manager->startDiscovery(serviceUUID);
        
        std::cout << "Scanning for 5 seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        m_manager->stopDiscovery();
        
        m_cachedDevices = m_manager->getDevices(serviceUUID);
        
        std::cout << "\nFound " << m_cachedDevices.size() << " device(s) with service " << serviceUUID << ":" << std::endl;
        for (size_t i = 0; i < m_cachedDevices.size(); i++) {
            const auto& dev = m_cachedDevices[i];
            std::cout << i + 1 << ". " << dev.address;
            if (!dev.name.empty()) {
                std::cout << " (" << dev.name << ")";
            }
            std::cout << " [" << (dev.connected ? "Connected" : "Disconnected") << "]";
            std::cout << std::endl;
        }
    }

    void connectToDevice() {
        if (m_cachedDevices.empty()) {
            std::cout << "No devices cached. Please scan first." << std::endl;
            return;
        }

        std::cout << "\nAvailable devices:" << std::endl;
        for (size_t i = 0; i < m_cachedDevices.size(); i++) {
            const auto& dev = m_cachedDevices[i];
            std::cout << i + 1 << ". " << dev.address;
            if (!dev.name.empty()) {
                std::cout << " (" << dev.name << ")";
            }
            std::cout << std::endl;
        }

        int choice = std::stoi(getInput("\nSelect device number: "));
        if (choice < 1 || choice > static_cast<int>(m_cachedDevices.size())) {
            std::cout << "Invalid selection." << std::endl;
            return;
        }

        const auto& device = m_cachedDevices[choice - 1];
        if (m_manager->connectDevice(device.address)) {
            m_connectedDevice = device.address;
            
            // Request MTU of 250 bytes
            std::cout << "Requesting MTU of 250 bytes..." << std::endl;
            m_manager->requestMTU(device.address, 250);
            
            std::cout << "Successfully connected to " << device.address << std::endl;
        } else {
            std::cout << "Failed to connect to device." << std::endl;
        }
    }

    void disconnectFromDevice() {
        if (m_connectedDevice.empty()) {
            std::cout << "No device currently connected." << std::endl;
            return;
        }

        if (m_manager->disconnectDevice(m_connectedDevice)) {
            std::cout << "Disconnected from " << m_connectedDevice << std::endl;
            m_connectedDevice.clear();
            m_cachedServices.clear();
            m_cachedCharacteristics.clear();
        } else {
            std::cout << "Failed to disconnect from device." << std::endl;
        }
    }

    void forgetDevice() {
        if (m_cachedDevices.empty()) {
            std::cout << "No devices cached. Please scan first." << std::endl;
            return;
        }

        std::cout << "\nAvailable devices:" << std::endl;
        for (size_t i = 0; i < m_cachedDevices.size(); i++) {
            const auto& dev = m_cachedDevices[i];
            std::cout << i + 1 << ". " << dev.address;
            if (!dev.name.empty()) {
                std::cout << " (" << dev.name << ")";
            }
            std::cout << std::endl;
        }

        int choice = std::stoi(getInput("\nSelect device number to forget: "));
        if (choice < 1 || choice > static_cast<int>(m_cachedDevices.size())) {
            std::cout << "Invalid selection." << std::endl;
            return;
        }

        const auto& device = m_cachedDevices[choice - 1];
        if (m_manager->removeDevice(device.address)) {
            std::cout << "Device forgotten successfully." << std::endl;
            if (m_connectedDevice == device.address) {
                m_connectedDevice.clear();
            }
            // Refresh device list
            m_cachedDevices = m_manager->getDevices();
        } else {
            std::cout << "Failed to forget device." << std::endl;
        }
    }

    void listServices() {
        if (m_connectedDevice.empty()) {
            std::cout << "No device connected. Please connect first." << std::endl;
            return;
        }

        std::cout << "\nDiscovering services..." << std::endl;
        m_cachedServices = m_manager->getServices(m_connectedDevice);

        std::cout << "\nFound " << m_cachedServices.size() << " service(s):" << std::endl;
        for (size_t i = 0; i < m_cachedServices.size(); i++) {
            const auto& service = m_cachedServices[i];
            std::cout << i + 1 << ". UUID: " << service.uuid << std::endl;
            std::cout << "   Path: " << service.path << std::endl;
        }
    }

    void listCharacteristics() {
        if (m_cachedServices.empty()) {
            std::cout << "No services cached. Please list services first." << std::endl;
            return;
        }

        std::cout << "\nAvailable services:" << std::endl;
        for (size_t i = 0; i < m_cachedServices.size(); i++) {
            std::cout << i + 1 << ". " << m_cachedServices[i].uuid << std::endl;
        }

        int choice = std::stoi(getInput("\nSelect service number: "));
        if (choice < 1 || choice > static_cast<int>(m_cachedServices.size())) {
            std::cout << "Invalid selection." << std::endl;
            return;
        }

        const auto& service = m_cachedServices[choice - 1];
        m_currentServicePath = service.path;
        m_cachedCharacteristics = m_manager->getCharacteristics(service.path);

        std::cout << "\nFound " << m_cachedCharacteristics.size() << " characteristic(s):" << std::endl;
        for (size_t i = 0; i < m_cachedCharacteristics.size(); i++) {
            const auto& characteristic = m_cachedCharacteristics[i];
            std::cout << i + 1 << ". UUID: " << characteristic.uuid << std::endl;
            std::cout << "   Flags: ";
            for (const auto& flag : characteristic.flags) {
                std::cout << flag << " ";
            }
            std::cout << std::endl;
        }
    }

    void readCharacteristic() {
        if (m_cachedCharacteristics.empty()) {
            std::cout << "No characteristics cached. Please list characteristics first." << std::endl;
            return;
        }

        std::cout << "\nAvailable characteristics:" << std::endl;
        for (size_t i = 0; i < m_cachedCharacteristics.size(); i++) {
            std::cout << i + 1 << ". " << m_cachedCharacteristics[i].uuid << std::endl;
        }

        int choice = std::stoi(getInput("\nSelect characteristic number: "));
        if (choice < 1 || choice > static_cast<int>(m_cachedCharacteristics.size())) {
            std::cout << "Invalid selection." << std::endl;
            return;
        }

        const auto& characteristic = m_cachedCharacteristics[choice - 1];
        auto value = m_manager->readCharacteristic(characteristic.path);

        std::cout << "Value (" << value.size() << " bytes): ";
        for (uint8_t byte : value) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
        }
        std::cout << std::dec << std::endl;
    }

    void writeCharacteristic() {
        if (m_cachedCharacteristics.empty()) {
            std::cout << "No characteristics cached. Please list characteristics first." << std::endl;
            return;
        }

        std::cout << "\nAvailable characteristics:" << std::endl;
        for (size_t i = 0; i < m_cachedCharacteristics.size(); i++) {
            std::cout << i + 1 << ". " << m_cachedCharacteristics[i].uuid << std::endl;
        }

        int choice = std::stoi(getInput("\nSelect characteristic number: "));
        if (choice < 1 || choice > static_cast<int>(m_cachedCharacteristics.size())) {
            std::cout << "Invalid selection." << std::endl;
            return;
        }

        const auto& characteristic = m_cachedCharacteristics[choice - 1];

        std::string dataStr = getInput("Enter hex bytes (space-separated, e.g., 01 02 03): ");
        std::vector<uint8_t> data;
        
        std::istringstream iss(dataStr);
        std::string byteStr;
        while (iss >> byteStr) {
            try {
                data.push_back(static_cast<uint8_t>(std::stoi(byteStr, nullptr, 16)));
            } catch (...) {
                std::cout << "Invalid hex value: " << byteStr << std::endl;
                return;
            }
        }

        if (m_manager->writeCharacteristic(characteristic.path, data)) {
            std::cout << "Successfully wrote " << data.size() << " bytes." << std::endl;
        } else {
            std::cout << "Failed to write to characteristic." << std::endl;
        }
    }

    void enableNotifications() {
        if (m_cachedCharacteristics.empty()) {
            std::cout << "No characteristics cached. Please list characteristics first." << std::endl;
            return;
        }

        std::cout << "\nAvailable characteristics:" << std::endl;
        for (size_t i = 0; i < m_cachedCharacteristics.size(); i++) {
            std::cout << i + 1 << ". " << m_cachedCharacteristics[i].uuid << std::endl;
        }

        int choice = std::stoi(getInput("\nSelect characteristic number: "));
        if (choice < 1 || choice > static_cast<int>(m_cachedCharacteristics.size())) {
            std::cout << "Invalid selection." << std::endl;
            return;
        }

        const auto& characteristic = m_cachedCharacteristics[choice - 1];

        auto callback = [](const std::vector<uint8_t>& data) {
            std::cout << "\n>>> Notification received (" << data.size() << " bytes): ";
            for (uint8_t byte : data) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
            }
            std::cout << std::dec << std::endl;
            std::cout << ">>> ";
            std::cout.flush();
        };

        if (m_manager->enableNotifications(characteristic.path, callback)) {
            m_notifyActive = true;
            std::cout << "Notifications enabled. Listening for notifications..." << std::endl;
            std::cout << "Press Enter to return to menu..." << std::endl;
            
            // Start a thread to process events
            std::thread eventThread([this]() {
                while (m_notifyActive) {
                    m_manager->processEvents(100);
                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
            });
            
            // Wait for user input
            std::string dummy;
            std::getline(std::cin, dummy);
            
            m_notifyActive = false;
            eventThread.join();
        } else {
            std::cout << "Failed to enable notifications." << std::endl;
        }
    }

    void disableNotifications() {
        if (m_cachedCharacteristics.empty()) {
            std::cout << "No characteristics cached. Please list characteristics first." << std::endl;
            return;
        }

        std::cout << "\nAvailable characteristics:" << std::endl;
        for (size_t i = 0; i < m_cachedCharacteristics.size(); i++) {
            std::cout << i + 1 << ". " << m_cachedCharacteristics[i].uuid << std::endl;
        }

        int choice = std::stoi(getInput("\nSelect characteristic number: "));
        if (choice < 1 || choice > static_cast<int>(m_cachedCharacteristics.size())) {
            std::cout << "Invalid selection." << std::endl;
            return;
        }

        const auto& characteristic = m_cachedCharacteristics[choice - 1];

        if (m_manager->disableNotifications(characteristic.path)) {
            std::cout << "Notifications disabled." << std::endl;
        } else {
            std::cout << "Failed to disable notifications." << std::endl;
        }
    }
};

int main(int /*argc*/, char* /*argv*/[]) {
    try {
        BluetoothCLI cli;
        cli.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}
