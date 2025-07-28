#include <pylon/PylonIncludes.h>
#include <iostream>
#include <string>

using namespace Pylon;

int main()
{
    try {
        // Initialize Pylon
        PylonInitialize();
        std::cout << "Pylon initialized successfully" << std::endl;
        
        // Get the transport layer factory
        CTlFactory& tlFactory = CTlFactory::GetInstance();
        
        // Get all attached devices
        DeviceInfoList_t allDevices;
        tlFactory.EnumerateDevices(allDevices);
        
        std::cout << "Total devices found: " << allDevices.size() << std::endl;
        
        for (size_t i = 0; i < allDevices.size(); ++i) {
            std::cout << "\nDevice " << i << ":" << std::endl;
            std::cout << "  Friendly Name: " << allDevices[i].GetFriendlyName() << std::endl;
            std::cout << "  Model Name: " << allDevices[i].GetModelName() << std::endl;
            std::cout << "  Serial Number: " << allDevices[i].GetSerialNumber() << std::endl;
            std::cout << "  Device Class: " << allDevices[i].GetDeviceClass() << std::endl;
            
            try {
                String_t ipAddress = allDevices[i].GetIpAddress();
                if (!ipAddress.empty()) {
                    std::cout << "  IP Address: " << ipAddress.c_str() << std::endl;
                } else {
                    std::cout << "  IP Address: Not available" << std::endl;
                }
            }
            catch (const GenericException& e) {
                std::cout << "  IP Address: Error getting IP - " << e.GetDescription() << std::endl;
            }
            
            try {
                String_t macAddress = allDevices[i].GetMacAddress();
                if (!macAddress.empty()) {
                    std::cout << "  MAC Address: " << macAddress.c_str() << std::endl;
                } else {
                    std::cout << "  MAC Address: Not available" << std::endl;
                }
            }
            catch (const GenericException& e) {
                std::cout << "  MAC Address: Error getting MAC - " << e.GetDescription() << std::endl;
            }
        }
        
        // Terminate Pylon
        PylonTerminate();
        std::cout << "\nPylon terminated successfully" << std::endl;
    }
    catch (const GenericException& e) {
        std::cerr << "Error: " << e.GetDescription() << std::endl;
        return 1;
    }
    
    return 0;
} 