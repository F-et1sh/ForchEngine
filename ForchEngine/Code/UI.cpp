#include "pch.h"
#include "UI.h"

bool InputText(const std::string& label, std::string& value, int text_width = 100) {
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    strncpy_s(buffer, sizeof(buffer), value.c_str(), sizeof(buffer));

    ImGui::PushID(label.c_str());

    ImGui::Text(label.c_str());

    ImGui::SameLine();

    bool changed = false;

    ImGui::PushItemWidth(text_width);

    if (ImGui::InputText(("##input" + label).c_str(), buffer, sizeof(buffer))) {
        value = std::string(buffer);
        changed = true;
    }

    ImGui::PopItemWidth();

    ImGui::PopID();

    return changed;
}

void UI::Render() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::Begin("##ui", nullptr, flags);

    switch (this->m_ProcessScanner->GetState()) {
    case ScanState::InputProcessName: {
        this->OnInputProcessName();
    }
    break;
    case ScanState::InputVariable: {
        this->OnInputVariable();
    }
    break;
    case ScanState::Scanning: {
        this->OnScanning();
    }
    break;
    case ScanState::Filtering: {
        this->OnFiltering();
    }
    break;
    case ScanState::VariableEditing: {
        this->OnVariableEditing();
    }
    break;
    }

    if (!m_ErrorLog.empty()) {
        ImGui::TextColored(ImVec4(0.9f, 0.22f, 0.2f, 1.00f), "ERROR : %s", m_ErrorLog.c_str());
    }

    for (auto& variable : m_ProcessScanner->GetFoundVariables()) {
        ImGui::Begin(variable.name.c_str());
        
        InputValue(variable.type, variable.value);
        
        if (ImGui::Button("Apply"))
            m_ProcessScanner->Write(variable.address, variable.type, variable.value);
        
        ImGui::End();
    }
        
    ImGui::End();
}

#define CentralizeWindow() \
ImVec2 window_size = ImGui::GetWindowSize(); \
ImVec2 content_size = { 300.0f, 120.0f }; \
float x = (window_size.x - content_size.x) * 0.5f; \
float y = (window_size.y - content_size.y) * 0.5f; \
ImGui::SetCursorPos({ x, y }); \

void UI::OnInputProcessName() {
    CentralizeWindow();
    
    ImGui::BeginChild("##window", content_size, false, ImGuiWindowFlags_NoScrollbar);
    
    if (InputText("Enter process name", m_EnteringProcessName, content_size.x - 155))
        m_ErrorLog.clear();

    ImGui::Spacing();

    if (ImGui::Button("Search", ImVec2(content_size.x - 20, 0))) {
        try {
            this->m_ProcessScanner->FindProcess(FE2D::string_to_wstring(m_EnteringProcessName));
            this->m_ProcessScanner->SetState(ScanState::InputVariable);
        }
        catch (const std::exception& e) {
            m_ErrorLog = e.what();
        }
    }

    ImGui::EndChild();
}

void UI::OnInputVariable() {
    CentralizeWindow();

    ImGui::BeginChild("##window", content_size, false, ImGuiWindowFlags_NoScrollbar);

    ImGui::Text("Select type of the variable");

    if (ImGui::Button("Float32")) m_ProcessScanner->SetScanType(ScanType::Float32);
    ImGui::SameLine();
    if (ImGui::Button("Float64")) m_ProcessScanner->SetScanType(ScanType::Float64);
    ImGui::SameLine();
    if (ImGui::Button("Int32"  )) m_ProcessScanner->SetScanType(ScanType::Int32);
    ImGui::SameLine();
    if (ImGui::Button("Int64"  )) m_ProcessScanner->SetScanType(ScanType::Int64);

    if (m_ProcessScanner->GetScanType() != ScanType::None)
        ImGui::Text("Input variable value");

    static Value value = 0.0;
    InputValue(m_ProcessScanner->GetScanType(), value);

    m_ProcessScanner->SetValue(value);

    if (m_ProcessScanner->GetScanType() != ScanType::None && ImGui::Button("Search")) {
        m_ProcessScanner->SetState(ScanState::Scanning);
    }

    ImGui::EndChild();
}

void UI::OnScanning() {
    CentralizeWindow();

    ImGui::BeginChild("##window", content_size, false, ImGuiWindowFlags_NoScrollbar);

    size_t found_addresses = m_ProcessScanner->GetFoundAddressesCount();
    if (!found_addresses) {
        ImGui::Text("Scanning may take some time");
        m_ProcessScanner->Scan();
    }
    m_ProcessScanner->SetState(ScanState::Filtering);

    ImGui::EndChild();
}

void UI::OnFiltering() {
    CentralizeWindow();

    ImGui::BeginChild("##window", content_size, false, ImGuiWindowFlags_NoScrollbar);

    size_t found_addresses = m_ProcessScanner->GetFoundAddressesCount();

    if (found_addresses == 1)
        m_ProcessScanner->SetState(ScanState::VariableEditing);
    else if (found_addresses == 2) { // there may be a bug when one variable turns into two variables
        m_ProcessScanner->GetFoundAddresses().pop_back();
        m_ProcessScanner->SetState(ScanState::VariableEditing);
    }
    else if (!found_addresses) {
        m_ErrorLog = "Failed to filter. No matches found. Scanning again";
        m_ProcessScanner->SetState(ScanState::Scanning);
    }

    std::string text = std::string("Found ") + std::to_string(found_addresses) + std::string(" results");
    ImVec2 text_size = ImGui::CalcTextSize(text.c_str());
    ImGui::SetCursorPosX(text_size.x / 2);
    ImGui::Text(text.c_str());

    ImGui::Text("Input new value in game");

    static Value value = 0.0;
    InputValue(m_ProcessScanner->GetScanType(), value);

    if (ImGui::Button("Filter")) {
        m_ErrorLog.clear();
        try {
            m_ProcessScanner->Filter(value);
        }
        catch (const std::exception& e) {
            m_ErrorLog = e.what();
            m_ProcessScanner->SetState(ScanState::InputProcessName);
        }
    }

    ImGui::EndChild();
}

void UI::OnVariableEditing() {
    CentralizeWindow();

    ImGui::BeginChild("##window", content_size, false, ImGuiWindowFlags_NoScrollbar);

    ImGui::Text("Variable found. Input its name");

    static std::string variable_name;
    InputText("Variable name", variable_name);

    if (ImGui::Button("Create")) {
        FoundVariable variable;
        variable.address = m_ProcessScanner->GetFoundAddresses().back().address;
        variable.name = variable_name;
        variable.type = m_ProcessScanner->GetScanType();

        m_ProcessScanner->AddNewVariable(variable);

        m_ProcessScanner->SetState(ScanState::InputVariable);
        m_ProcessScanner->ResetFoundAddresses();
    }

    ImGui::EndChild();
}

void UI::InputValue(ScanType type, Value& value) {
    switch (type) {
    case ScanType::Float32: {
        float temp_value = value;
        ImGui::InputFloat("##label", &temp_value);
        value = temp_value;
    }
    break;
    case ScanType::Float64: {
        double temp_value = value;
        ImGui::InputDouble("##label", &temp_value);
        value = temp_value;
    }
    break;
    case ScanType::Int32: {
        int temp_value = value;
        ImGui::InputInt("##label", &temp_value);
        value = temp_value;
    }
    break;
    case ScanType::Int64: {
        long long temp_value = value;
        ImGui::InputScalar("##label", ImGuiDataType_S64, &temp_value);
        value = temp_value;
    }
    }
}
