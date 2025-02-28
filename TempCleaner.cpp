#include <windows.h>
#include <Lmcons.h>  // For UNLEN (Username max length)
#include <iostream>
#include <string>
#include <filesystem>
#include <iomanip>

namespace fs = std::filesystem;

// ANSI Escape Codes for Colors
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

// Function to get the logged-in Windows username
std::string GetWindowsUsername() {
    char username[UNLEN + 1];
    DWORD size = UNLEN + 1;
    if (GetUserNameA(username, &size)) {
        return std::string(username);
    }
    return "Unknown User";
}

// Function to get OS Name
std::string GetOSName() {
    HKEY hKey;
    const char* subkey = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
    const char* value = "ProductName";
    char osName[256];
    DWORD size = sizeof(osName);
    
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subkey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegGetValueA(hKey, NULL, value, RRF_RT_REG_SZ, NULL, osName, &size) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return std::string(osName);
        }
        RegCloseKey(hKey);
    }
    return "Unknown OS";
}

// Function to get CPU Name
std::string GetCPUName() {
    HKEY hKey;
    const char* subkey = "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0";
    const char* value = "ProcessorNameString";
    char cpuName[256];
    DWORD size = sizeof(cpuName);

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subkey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegGetValueA(hKey, NULL, value, RRF_RT_REG_SZ, NULL, cpuName, &size) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return std::string(cpuName);
        }
        RegCloseKey(hKey);
    }
    return "Unknown CPU";
}

// Function to get GPU Name
std::string GetGPUName() {
    DISPLAY_DEVICEA displayDevice;
    displayDevice.cb = sizeof(DISPLAY_DEVICEA);
    if (EnumDisplayDevicesA(NULL, 0, &displayDevice, 0)) {
        return std::string(displayDevice.DeviceString);
    }
    return "Unknown GPU";
}

// Function to get RAM Size
std::string GetRAMSize() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(memInfo);
    if (GlobalMemoryStatusEx(&memInfo)) {
        return std::to_string(memInfo.ullTotalPhys / (1024 * 1024 * 1024)) + " GB";
    }
    return "Unknown RAM Size";
}

// Function to get Temp folder path dynamically
std::string GetTempFolderPath() {
    return "C:\\Users\\" + GetWindowsUsername() + "\\AppData\\Local\\Temp";
}

// Function to calculate folder size
uintmax_t GetFolderSize(const fs::path& folderPath) {
    uintmax_t totalSize = 0;
    try {
        for (const auto& entry : fs::recursive_directory_iterator(folderPath, fs::directory_options::skip_permission_denied)) {
            if (fs::is_regular_file(entry)) {
                totalSize += fs::file_size(entry);
            }
        }
    } catch (...) {}
    return totalSize;
}

// Function to delete files in the Temp folder
void CleanTempFolder(const fs::path& folderPath, uintmax_t& deletedSize, int& deletedFiles) {
    try {
        for (const auto& entry : fs::directory_iterator(folderPath, fs::directory_options::skip_permission_denied)) {
            try {
                if (fs::is_directory(entry)) {
                    fs::remove_all(entry);
                } else {
                    deletedSize += fs::file_size(entry);
                    deletedFiles++;
                    fs::remove(entry);
                }
            } catch (...) {}
        }
    } catch (...) {}
}

// Function to format bytes to MB or GB
std::string FormatSize(uintmax_t bytes) {
    double sizeInMB = bytes / (1024.0 * 1024.0);
    if (sizeInMB >= 1024) {
        return std::to_string(sizeInMB / 1024.0).substr(0, 4) + " GB";
    }
    return std::to_string(sizeInMB).substr(0, 4) + " MB";
}

// Function to display system information
void DisplaySystemInfo() {
    std::cout << CYAN << "****************************************\n";
    std::cout << "*           SYSTEM INFORMATION         *\n";
    std::cout << "****************************************" << RESET << std::endl;
    std::cout << BOLD << "OS: " << RESET << GetOSName() << std::endl;
    std::cout << BOLD << "User: " << RESET << GetWindowsUsername() << std::endl;
    std::cout << BOLD << "CPU: " << RESET << GetCPUName() << std::endl;
    std::cout << BOLD << "GPU: " << RESET << GetGPUName() << std::endl;
    std::cout << BOLD << "RAM: " << RESET << GetRAMSize() << std::endl;
}

// Function to start Temp Cleaner
void StartTempCleaner() {
    std::string tempFolderPath = GetTempFolderPath();
    std::cout << "\n" << YELLOW << "Temp Folder: " << RESET << tempFolderPath << std::endl;

    if (!fs::exists(tempFolderPath)) {
        std::cerr << RED << "Error: Temp folder not found!" << RESET << std::endl;
        return;
    }

    uintmax_t initialSize = GetFolderSize(tempFolderPath);
    std::cout << GREEN << "Current Temp Folder Size: " << FormatSize(initialSize) << RESET << std::endl;

    std::cout << "\nDo you want to clean the Temp folder? (y/n): ";
    char choice;
    std::cin >> choice;
    if (choice != 'y' && choice != 'Y') {
        std::cout << RED << "Operation canceled." << RESET << std::endl;
        return;
    }

    uintmax_t deletedSize = 0;
    int deletedFiles = 0;
    CleanTempFolder(tempFolderPath, deletedSize, deletedFiles);

    std::cout << BLUE << "\n" << deletedFiles << " files deleted." << RESET << std::endl;
    std::cout << GREEN << "Freed Space: " << FormatSize(deletedSize) << RESET << std::endl;
}

int main() {
    system("cls"); // Clear the console for a clean look
    DisplaySystemInfo();
    StartTempCleaner();

    std::cout << "\n" << CYAN << "Press Enter to Exit." << RESET << std::endl;
    std::cin.ignore();
    std::cin.get();
    return 0;
}
