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
    this->OnMainMenuBar();

    // begin background window
    {
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
    }

    if (!m_ScannersManager->IsInitialized())
        this->OnInputProcessName();

    this->DrawAllScannerWindows();

    if (m_ScannersManager->IsInitialized() && !m_UI_ScannersData.size()) {
        constexpr std::string_view text = "There is no variables yet. To create a new variable go to Tool -> New Variable";
        float text_width = ImGui::CalcTextSize(text.data()).x;
        ImVec2 center = ImGui::GetIO().DisplaySize;
        center.x -= text_width;
        center.x /= 2;
        center.y /= 2;
        ImGui::SetCursorPos(center);
        ImGui::Text(text.data());
    }

    if (!m_ErrorLog.empty()) {
        ImGui::TextColored(ImVec4(0.9f, 0.22f, 0.2f, 1.00f), "ERROR : %s", m_ErrorLog.c_str());
    }

    ImGui::End();
}

void UI::RemoveScanner(size_t i) {
    m_ScannersManager->RemoveScanner(i);
    m_UI_ScannersData.erase(m_UI_ScannersData.begin() + i);
}

void UI::AddScanner() {
    m_ScannersManager->AddScanner();

    m_UI_ScannersData.emplace_back();
    auto& ui_scanner = m_UI_ScannersData.back();
    this->GenerateUniqueName(ui_scanner);

    ui_scanner.is_need_adjust_position = true;

    ui_scanner.window_position = ImGui::GetIO().DisplaySize;
    ui_scanner.window_position.x -= 250;
    ui_scanner.window_position.y -= 150;
    ui_scanner.window_position.x /= 2;
    ui_scanner.window_position.y /= 2;

    constexpr static size_t window_shift = 5;

    size_t windows_in_queue = 0;
    for (auto& e : m_UI_ScannersData) {
        for (size_t i = 0; i < m_UI_ScannersData.size(); i++) {
            ImVec2 shift = {};
            shift.x = i * window_shift;
            shift.y = i * window_shift;
            
            shift.x += ui_scanner.window_position.x;
            shift.y += ui_scanner.window_position.y;
            
            if (e.window_position.x == shift.x &&
                e.window_position.y == shift.y)
                windows_in_queue++;
        }
    }

    if (windows_in_queue) windows_in_queue -= 1;

    ui_scanner.window_position.x += windows_in_queue * window_shift;
    ui_scanner.window_position.y += windows_in_queue * window_shift;
}

void UI::GenerateUniqueName(UI_ScannerData& scanner_data) {
    scanner_data.name = "New Variable";
    std::string name_buffer = scanner_data.name;

    for (size_t i = 0; i < m_UI_ScannersData.size() - 1; i++) {
        if (scanner_data.name == m_UI_ScannersData[i].name) {
            scanner_data.name = name_buffer + " (" + std::to_string(i + 1) + ")";
            i = 0;
        }
    };
}

void UI::OnMainMenuBar() {
    if (!ImGui::BeginMainMenuBar()) return;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Exit"))
            m_Window->CallCloseRequest();

        ImGui::EndMenu();
    }

    if (m_ScannersManager->IsInitialized() && ImGui::BeginMenu("Tool")) {
        if (ImGui::MenuItem("New Variable")) {
            this->AddScanner();
        }

        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void UI::OnInputProcessName() {
    ImVec2 window_size = ImGui::GetWindowSize();
    ImVec2 content_size = { 300.0f, 120.0f };
    float x = (window_size.x - content_size.x) * 0.5f;
    float y = (window_size.y - content_size.y) * 0.5f;
    ImGui::SetCursorPos({ x, y });

    ImGui::BeginChild("##window", content_size, false, ImGuiWindowFlags_NoScrollbar);

    if (InputText("Enter process name", m_EnteringProcessName, content_size.x - 155))
        m_ErrorLog.clear();

    ImGui::Spacing();

    if (ImGui::Button("Search", ImVec2(content_size.x - 20, 0)) || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
        try {
            m_ScannersManager->Initialize(FE2D::string_to_wstring(m_EnteringProcessName));
        }
        catch (const std::exception& e) {
            m_ErrorLog = e.what();
        }
    }

    ImGui::EndChild();
}

void UI::DrawAllScannerWindows() {
    for (size_t i = 0; i < m_ScannersManager->GetScanners().size(); i++) {
        std::string id = std::string("##ScannerWindow_") + std::to_string(i);
        ImGui::PushID(id.c_str());

        auto& scanner = m_ScannersManager->GetScanners()[i];
        auto& ui_scanner = m_UI_ScannersData[i];
        
        ImGui::SetNextWindowSizeConstraints({ 250, 150 }, {1000, 600});

        if (ui_scanner.is_need_adjust_position) {
            ImGui::SetNextWindowPos(ui_scanner.window_position);
            ui_scanner.is_need_adjust_position = false;
        }

        std::string window_label = ui_scanner.name + id;
        ImGui::Begin(window_label.c_str(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDocking);

        ui_scanner.window_position = ImGui::GetWindowPos();

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Rename")) {
                ui_scanner.is_renaming = true;
            }
            if (ImGui::MenuItem("Remove")) {
                ui_scanner.is_removing = true;
            }
            ImGui::EndPopup();
        }

        this->HandlePopups(ui_scanner, i);

        switch (scanner.state) {
            case ScanState::InputVariable  : this->OnInputVariable  (scanner); break;
            case ScanState::Scanning       : this->OnScanning       (scanner); break;
            case ScanState::Filtering      : this->OnFiltering      (scanner); break;
            case ScanState::VariableEditing: this->OnVariableEditing(scanner); break;
        }

        ImGui::End();
        ImGui::PopID();
    }
}

void UI::HandlePopups(UI_ScannerData& scanner_data, size_t i) {
    if (scanner_data.is_renaming) ImGui::OpenPopup("Rename Variable");
    if (scanner_data.is_removing) ImGui::OpenPopup("Remove Variable");

    if (ImGui::BeginPopup("Rename Variable", ImGuiWindowFlags_NoResize)) {
        InputText("Name", scanner_data.rename_buffer);

        if (ImGui::Button("Apply") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            scanner_data.name = scanner_data.rename_buffer;
            scanner_data.rename_buffer.clear();
            scanner_data.is_renaming = false;
            
            scanner_data.is_need_adjust_position = true;

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            scanner_data.rename_buffer.clear();
            scanner_data.is_renaming = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Remove Variable", ImGuiWindowFlags_NoResize)) {
        ImGui::Text("Remove the variable?");

        if (ImGui::Button("Yes") || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            this->RemoveScanner(i);
        }

        ImGui::SameLine();

        if (ImGui::Button("No") || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            scanner_data.is_removing = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void UI::OnInputVariable(ScannerData& scanner_data) {
    ImGui::Text("Select type of the variable");

    if (ImGui::Button("Float32")) scanner_data.scan_type = ScanType::Float32;
    ImGui::SameLine();
    if (ImGui::Button("Float64")) scanner_data.scan_type = ScanType::Float64;
    ImGui::SameLine();
    if (ImGui::Button("Int32"  )) scanner_data.scan_type = ScanType::Int32;
    ImGui::SameLine();
    if (ImGui::Button("Int64"  )) scanner_data.scan_type = ScanType::Int64;

    if (scanner_data.scan_type != ScanType::None) {
        ImGui::Text("Select start value of the variable");

        InputValue(scanner_data);

        if (ImGui::Button("Search"))
            scanner_data.state = ScanState::Scanning;
    }
}

void UI::OnScanning(ScannerData& scanner_data) {
    size_t found_addresses = scanner_data.found_addresses.size();
    if (!found_addresses)
        m_ScannersManager->Scan(scanner_data);
    scanner_data.state = ScanState::Filtering;
}

void UI::OnFiltering(ScannerData& scanner_data) {
    size_t found_addresses = scanner_data.found_addresses.size();

    if (found_addresses == 1)
        scanner_data.state = ScanState::VariableEditing;
    else if (found_addresses == 2) { // there may be a bug when one variable turns into two variables
        scanner_data.found_addresses.pop_back();
        scanner_data.state = ScanState::VariableEditing;
    }
    else if (!found_addresses) {
        m_ErrorLog = "Failed to filter. No matches found. Scanning again";
        scanner_data.state = ScanState::Scanning;
    }

    std::string text = std::string("Found ") + std::to_string(found_addresses) + std::string(" results");
    ImVec2 text_size = ImGui::CalcTextSize(text.c_str());
    ImGui::SetCursorPosX(text_size.x / 2);
    ImGui::Text(text.c_str());

    ImGui::Text("Input new value in game");

    InputValue(scanner_data);

    if (ImGui::Button("Filter")) {
        m_ErrorLog.clear();
        try {
            m_ScannersManager->Filter(scanner_data);
        }
        catch (const std::exception& e) {
            m_ErrorLog = e.what();
            scanner_data.state = ScanState::InputVariable;
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Back")) {
        m_ErrorLog.clear();
        scanner_data.state = ScanState::InputVariable;
        scanner_data.found_addresses.clear();
        scanner_data.value = 0.0;
    }
}

void UI::OnVariableEditing(ScannerData& scanner_data) {
    ImGui::Text("Control the value");
    if (InputValue(scanner_data))
        m_ScannersManager->Write(scanner_data);
}

bool UI::InputValue(ScannerData& scanner_data) {
    switch (scanner_data.scan_type) {
    case ScanType::Float32: {
        float temp_value = scanner_data.value;
        bool result = ImGui::InputFloat("##label", &temp_value);
        scanner_data.value = temp_value;
        return result;
    }
    break;
    case ScanType::Float64: {
        double temp_value = scanner_data.value;
        bool result = ImGui::InputDouble("##label", &temp_value);
        scanner_data.value = temp_value;
        return result;
    }
    break;
    case ScanType::Int32: {
        int temp_value = scanner_data.value;
        bool result = ImGui::InputInt("##label", &temp_value);
        scanner_data.value = temp_value;
        return result;
    }
    break;
    case ScanType::Int64: {
        long long temp_value = scanner_data.value;
        bool result = ImGui::InputScalar("##label", ImGuiDataType_S64, &temp_value);
        scanner_data.value = temp_value;
        return result;
    }
    }
}
