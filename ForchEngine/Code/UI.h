#pragma once
#include "Window.h"
#include "ScannersManager.h"

struct UI_ScannerData {
    bool is_renaming = false;
    bool is_removing = false;
    bool is_resetting = false;
    
    std::string name;
    std::string rename_buffer;

    ImVec2 window_position;
    bool is_need_adjust_position = false;
};

class UI {
public:
    UI(std::shared_ptr<Window> window, std::shared_ptr<ScannersManager> scanners_manager)
        : m_Window(std::move(window)), m_ScannersManager(std::move(scanners_manager)) {}
    ~UI() = default;

    void Render();

private:
    void RemoveScanner(size_t i);
    void AddScanner();
    void GenerateUniqueName(UI_ScannerData& scanner_data);

    void OnMainMenuBar();
    void OnInputProcessName();
    void DrawAllScannerWindows();

    void HandlePopups(UI_ScannerData& scanner_data, size_t i);

    void OnInputVariable(ScannerData& scanner_data);
    void OnScanning(ScannerData& scanner_data);
    void OnFiltering(ScannerData& scanner_data);
    void OnVariableEditing(ScannerData& scanner_data);

    bool InputValue(ScannerData& scanner_data);

private:
    std::string m_EnteringProcessName;
    std::string m_ErrorLog;

    std::vector<UI_ScannerData> m_UI_ScannersData;

private:
    std::shared_ptr<Window> m_Window;
    std::shared_ptr<ScannersManager> m_ScannersManager;
};