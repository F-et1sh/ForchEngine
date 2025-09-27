#include "pch.h"
#include "ScannersManager.h"

void ScannersManager::Reset() {
    m_ProcessName.clear();
    m_ProcessID = 0;
    m_ProcessHandle = 0;
    m_ScannersData.clear();
}

void ScannersManager::Initialize(const std::wstring& process_name) {
    m_ProcessName = process_name;
    if (m_ProcessName.empty()) {
        this->Reset();
        throw std::runtime_error("Process name was empty");
    }

    m_ProcessID = GetProcessID(m_ProcessName);
    if (!m_ProcessID) {
        this->Reset();
        throw std::runtime_error("Failed to find process id");
    }

    m_ProcessHandle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, m_ProcessID);
    if (!m_ProcessHandle) {
        this->Reset();
        throw std::runtime_error("Failed to open process");
    }
}

void ScannersManager::RemoveScanner(size_t i) {
    m_ScannersData.erase(m_ScannersData.begin() + i);
}

void ScannersManager::AddScanner() {
    m_ScannersData.emplace_back();
}

void ScannersManager::Scan(ScannerData& scanner_data) {
    Value epsilon = 0.0;

    switch (scanner_data.scan_type) {
    case ScanType::Float32:
        epsilon = 1e-6;
        scanner_data.found_addresses = ScanMemory<float     >(m_ProcessHandle, scanner_data.value, epsilon);
        break;
    case ScanType::Float64:
        epsilon = 1e-10;
        scanner_data.found_addresses = ScanMemory<Value     >(m_ProcessHandle, scanner_data.value, epsilon);
        break;
    case ScanType::Int32:
        epsilon = 0.0;
        scanner_data.found_addresses = ScanMemory<int       >(m_ProcessHandle, scanner_data.value, epsilon);
        break;
    case ScanType::Int64:
        epsilon = 0.0;
        scanner_data.found_addresses = ScanMemory<long long>(m_ProcessHandle, scanner_data.value, epsilon);
        break;
    }
}

void ScannersManager::Filter(ScannerData& scanner_data) {
    Value epsilon = 0.0;

    switch (scanner_data.scan_type) {
    case ScanType::Float32:
        epsilon = 1e-6;
        FilterResults<float    >(m_ProcessHandle, scanner_data.found_addresses, scanner_data.value, epsilon);
        break;                 
    case ScanType::Float64:    
        epsilon = 1e-10;       
        FilterResults<double   >(m_ProcessHandle, scanner_data.found_addresses, scanner_data.value, epsilon);
        break;                 
    case ScanType::Int32:      
        epsilon = 0.0;         
        FilterResults<int      >(m_ProcessHandle, scanner_data.found_addresses, scanner_data.value, epsilon);
        break;
    case ScanType::Int64:
        epsilon = 0.0;
        FilterResults<long long>(m_ProcessHandle, scanner_data.found_addresses, scanner_data.value, epsilon);
        break;
    }
}

bool ScannersManager::Write(ScannerData& scanner_data) {
    return WriteValue(m_ProcessHandle, scanner_data.found_addresses.back().address, scanner_data.scan_type, scanner_data.value);
}

ScannersManager::~ScannersManager() {
    CloseHandle(m_ProcessHandle);
}
