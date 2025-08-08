#pragma once
#include "Process.h"

class UI {
public:
    UI(std::shared_ptr<ProcessScanner> process_scanner)
        : m_ProcessScanner(std::move(process_scanner)) {}
    ~UI() = default;

    void Render();

private:
    void OnInputProcessName();
    void OnInputVariable();
    void OnScanning();
    void OnFiltering();
    void OnVariableEditing();

    void InputValue(ScanType type, Value& value);

private:
    std::string m_EnteringProcessName;
    std::string m_ErrorLog;

    Value m_InputValue = 0.0;

private:
    std::shared_ptr<ProcessScanner> m_ProcessScanner;
};