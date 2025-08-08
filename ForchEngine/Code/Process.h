#pragma once

using ProcessID = DWORD;
using ProcessHandle = HANDLE;
using Value = double;

enum class ScanType { None, Float32, Float64, Int32, Int64 };
enum class ScanState { InputProcessName, InputVariable, Scanning, Filtering, VariableEditing };

struct FoundAddress {
    uintptr_t address = 0;
    Value value = 0.0;
};

struct FoundVariable {
    std::string name;
    uintptr_t address = 0;
    ScanType type = ScanType::None;
    Value value = 0.0;
};

using FoundAddressesContainer = std::vector<FoundAddress>;
using FoundVariablesContainer = std::vector<FoundVariable>;

template<typename T>
concept CValue = requires(T type)
{
    requires std::is_same_v<T, float>
|| std::is_same_v<T, Value>
|| std::is_same_v<T, int>
|| std::is_same_v<T, long long>;
};

class ProcessScanner {
public:
    ProcessScanner() = default;
    ~ProcessScanner();

    void FindProcess(const std::wstring& process_name);

    void ValidateOrThrow();

    void Scan();
    void Filter(Value new_value);
    void Write(Value new_value);
    void Write(uintptr_t address, ScanType type, Value value);

    inline size_t GetFoundAddressesCount()const noexcept { return m_FoundAddresses.size(); }
    inline FoundAddressesContainer& GetFoundAddresses()noexcept { return m_FoundAddresses; }
    inline FoundVariablesContainer& GetFoundVariables()noexcept { return m_FoundVariables; }

    inline Value GetValue()const noexcept { return m_Value; }
    inline void SetValue(Value value)noexcept { m_Value = value; }

    inline ScanType GetScanType()noexcept { return m_ScanType; }
    inline void SetScanType(ScanType type)noexcept { m_ScanType = type; }

    inline void SetState(ScanState state)noexcept { m_State = state; }
    inline ScanState GetState()const noexcept { return m_State; }

    inline void AddNewVariable(const FoundVariable& variable) { m_FoundVariables.emplace_back(variable); }
    inline void ResetFoundAddresses()noexcept { m_FoundAddresses.clear(); }

private:
    ScanState m_State = ScanState::InputProcessName;

    std::wstring m_ProcessName;

    ProcessID m_ProcessID = 0;
    ProcessHandle m_ProcessHandle = 0;

    Value m_Value = 0.0;
    ScanType m_ScanType = ScanType::None;

    FoundAddressesContainer m_FoundAddresses;
    FoundVariablesContainer m_FoundVariables;

private:
    static ProcessID GetProcessID(const std::wstring& process_name);

    template<CValue T>
    inline static bool MatchValue(T val, T target, Value epsilon) {
        if constexpr (std::is_floating_point_v<T>)
            return fabs(val - target) < epsilon;
        else return val == target;
    }

    template<CValue T>
    inline static FoundAddressesContainer ScanMemory(ProcessHandle process_handle, T target_value, Value epsilon) {
        FoundAddressesContainer addresses;
        addresses.reserve(5000);

        MEMORY_BASIC_INFORMATION mbi;
        uintptr_t addr = 0;
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        while (addr < (uintptr_t)si.lpMaximumApplicationAddress &&
            VirtualQueryEx(process_handle, (LPCVOID)addr, &mbi, sizeof(mbi)) == sizeof(mbi)) {
            if ((mbi.State == MEM_COMMIT) && !(mbi.Protect & PAGE_GUARD) && !(mbi.Protect & PAGE_NOACCESS)) {
                size_t regionSize = mbi.RegionSize;
                std::vector<char> buffer(regionSize);
                SIZE_T bytesRead;
                if (ReadProcessMemory(process_handle, mbi.BaseAddress, buffer.data(), regionSize, &bytesRead)) {
                    for (size_t i = 0; i + sizeof(T) <= bytesRead; i++) {
                        T val = *reinterpret_cast<T*>(&buffer[i]);
                        if (MatchValue(val, target_value, epsilon))
                            addresses.emplace_back(FoundAddress{ (uintptr_t)mbi.BaseAddress + i, (Value)val });
                    }
                }
            }
            addr += mbi.RegionSize;
        }
        return addresses;
    }

    template<CValue T>
    inline static void FilterResults(ProcessHandle process_handle, FoundAddressesContainer& results, T new_value, Value epsilon) {
        size_t write_i = 0;
        for (size_t read_i = 0; read_i < results.size(); read_i++) {
            T val;
            if (ReadProcessMemory(process_handle, (LPCVOID)results[read_i].address, &val, sizeof(val), nullptr) && MatchValue(val, new_value, epsilon)) {
                results[write_i++] = { results[read_i].address, (Value)val };
            }
        }
        results.resize(write_i);
    }

    static bool WriteValue(ProcessHandle process_handle, uintptr_t address, ScanType type, Value new_value);
};