#pragma once
#include "Process.h"

class ScannersManager {
public:
	ScannersManager() = default;
	~ScannersManager();

	void Reset();

	void Initialize(const std::wstring& process_name);
	inline bool IsInitialized()const noexcept { return !m_ProcessName.empty(); }

	inline bool IsProcessStillRunning()const { return IsProcessRunning(m_ProcessHandle); }

	void RemoveScanner(size_t i);
	void AddScanner();
	inline std::vector<ScannerData>& GetScanners()noexcept { return m_ScannersData; }

	void Scan(ScannerData& scanner_data);
	void Filter(ScannerData& scanner_data);
	bool Write(ScannerData& scanner_data);

private:
	std::wstring m_ProcessName;

	ProcessID m_ProcessID = 0;
	ProcessHandle m_ProcessHandle = 0;

private:
	std::vector<ScannerData> m_ScannersData;

};