#include <iostream>
#include <string>

#include "boot_module/bluetooth_cli.hpp"


int main(int /*argc*/, char* /*argv*/[])
{
  try
  {
    boot_module::BluetoothCLI cli;
    cli.run();
    return 0;
  }
  catch (const std::exception& e)
  {
    std::cerr << "Fatal error: " << e.what() << std::endl;
    return 1;
  }
}
