#include "pch.h"
#include "Process.h"

ProcessScanner::~ProcessScanner() {
    CloseHandle(m_ProcessHandle);
}

void ProcessScanner::FindProcess(const std::wstring& process_name) {
    m_ProcessName = process_name;
    if (m_ProcessName.empty()) {
        throw std::runtime_error("Process name was empty");
    }

    m_ProcessID = GetProcessID(m_ProcessName);
    if (!m_ProcessID) {
        throw std::runtime_error("Failed to find process id");
    }

    m_ProcessHandle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, m_ProcessID);
    if (!m_ProcessHandle) {
        throw std::runtime_error("Failed to open process");
    }
}

void ProcessScanner::ValidateOrThrow() {
    if (m_ProcessName.empty())        throw std::runtime_error("Process name was empty");
    if (!m_ProcessID)                 throw std::runtime_error("Process ID was zero");
    if (!m_ProcessHandle)             throw std::runtime_error("Process handle was zero");
    if (m_ScanType == ScanType::None) throw std::runtime_error("Scan type was none");
}

void ProcessScanner::Scan() {
    this->ValidateOrThrow();

    Value epsilon = 0.0;

    switch (m_ScanType) {
    case ScanType::Float32:
        epsilon = 1e-6;
        m_FoundAddresses = ScanMemory<float>(m_ProcessHandle, m_Value, epsilon);
        break;
    case ScanType::Float64:
        epsilon = 1e-10;
        m_FoundAddresses = ScanMemory<Value>(m_ProcessHandle, m_Value, epsilon);
        break;
    case ScanType::Int32:
        epsilon = 0.0;
        m_FoundAddresses = ScanMemory<int>(m_ProcessHandle, m_Value, epsilon);
        break;
    case ScanType::Int64:
        epsilon = 0.0;
        m_FoundAddresses = ScanMemory<long long>(m_ProcessHandle, m_Value, epsilon);
        break;
    }
}

void ProcessScanner::Filter(Value new_value) {
    this->ValidateOrThrow();

    Value epsilon = 0.0;

    switch (m_ScanType) {
    case ScanType::Float32:
        epsilon = 1e-6;
        FilterResults<float>(m_ProcessHandle, m_FoundAddresses, new_value, epsilon);
        break;
    case ScanType::Float64:
        epsilon = 1e-10;
        FilterResults<double>(m_ProcessHandle, m_FoundAddresses, new_value, epsilon);
        break;
    case ScanType::Int32:
        epsilon = 0.0;
        FilterResults<int>(m_ProcessHandle, m_FoundAddresses, new_value, epsilon);
        break;
    case ScanType::Int64:
        epsilon = 0.0;
        FilterResults<long long>(m_ProcessHandle, m_FoundAddresses, new_value, epsilon);
        break;
    }
}

void ProcessScanner::Write(Value new_value) {
    this->ValidateOrThrow();
    WriteValue(m_ProcessHandle, m_FoundAddresses.back().address, m_ScanType, new_value);
}

void ProcessScanner::Write(uintptr_t address, ScanType type, Value value) {
    this->ValidateOrThrow();
    WriteValue(m_ProcessHandle, address, type, value);
}

ProcessID ProcessScanner::GetProcessID(const std::wstring& process_name) {
    ProcessID process_id = 0;
    ProcessHandle snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(entry);

        if (Process32First(snapshot, &entry)) {
            do {
                if (!_wcsicmp(entry.szExeFile, process_name.c_str())) {
                    process_id = entry.th32ProcessID;
                    break;
                }
            } while (Process32Next(snapshot, &entry));
        }

        CloseHandle(snapshot);
    }

    return process_id;
}

bool ProcessScanner::WriteValue(ProcessHandle process_handle, uintptr_t address, ScanType type, Value new_value) {
    SIZE_T size = 0;
    switch (type) {
        case ScanType::Float32: { float     v = (float)new_value    ; size = sizeof(v); return WriteProcessMemory(process_handle, (LPVOID)address, &v, size, nullptr); }
        case ScanType::Float64: { Value    v = new_value           ; size = sizeof(v); return WriteProcessMemory(process_handle, (LPVOID)address, &v, size, nullptr); }
        case ScanType::Int32  : { int       v = (int)new_value      ; size = sizeof(v); return WriteProcessMemory(process_handle, (LPVOID)address, &v, size, nullptr); }
        case ScanType::Int64  : { long long v = (long long)new_value; size = sizeof(v); return WriteProcessMemory(process_handle, (LPVOID)address, &v, size, nullptr); }
    }
    return false;
}
