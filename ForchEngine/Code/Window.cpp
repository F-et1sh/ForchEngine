#include "pch.h"
#include "Window.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Window::Window(size_t resolution_x, size_t resolution_y, const std::string& title) {
    if (!glfwInit()) {
        std::cerr << "ERROR : Failed to Initialize GLFW" << std::endl;
        return;
    }
    glewExperimental = GL_TRUE;
    glEnable(GL_DEPTH_TEST);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    m_Window = glfwCreateWindow(resolution_x, resolution_y, title.c_str(), nullptr, nullptr);
    if (!m_Window) {
        glfwTerminate();
        std::cerr << "ERROR : Failed to Initialize Window" << std::endl;
        return;
    }

    glfwSetWindowUserPointer(m_Window, this);
    glfwSetCursorPosCallback(m_Window, cursor_position_callback);
    glfwSetMouseButtonCallback(m_Window, mouse_button_callback);

#ifdef WIN32
    HWND hwnd = glfwGetWin32Window(m_Window);
    SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

#endif // WIN32
    
    glfwMakeContextCurrent(m_Window);
    
    if (glewInit() != GLEW_OK) {
        glfwDestroyWindow(m_Window);
        std::cerr << "ERROR : Failed to Initialize GLEW" << std::endl;
        return;
    }

    GLFWimage images[1];
    
                           images[0].pixels = stbi_load("Icons/icon3_small.png", &images[0].width, &images[0].height, nullptr, STBI_rgb_alpha); // Visual Studio
    if (!images[0].pixels) images[0].pixels = stbi_load("icon3_small.png"      , &images[0].width, &images[0].height, nullptr, STBI_rgb_alpha); // Build
    
    glfwSetWindowIcon(m_Window, 1, images);
    stbi_image_free(images[0].pixels);

    // enable vsyn
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    io.IniFilename = nullptr;
    
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    this->ConfigureImGuiStyle();

    m_IsRunning = true;
}

Window::~Window() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void Window::CheckClose() {
    if (glfwWindowShouldClose(m_Window)) {
        ImGui::OpenPopup("Close the application?");

        if (ImGui::BeginPopupModal("Close the application?", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar)) {
            ImGui::Text("Do you really want to close the application?");
            ImGui::Separator();

            if (ImGui::Button("Yes")) {
                m_IsRunning = false;
            }

            ImGui::SameLine();

            if (ImGui::Button("No") || glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(m_Window, GLFW_FALSE);
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }
}

void Window::ClearScreen() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::BeginImGuiFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}

void Window::EndImGuiFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Window::SwapBuffers() {
    glfwSwapBuffers(m_Window);
}

void Window::PollEvents() {
    glfwPollEvents();
}

void Window::EnableTransparentMode(bool enable) {
    HWND hwnd = glfwGetWin32Window(m_Window);

    if (enable) {
        SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

        m_IsThemeTransparent = true;
    }
    else {
        LONG ex_style = GetWindowLong(hwnd, GWL_EXSTYLE);
        ex_style &= ~WS_EX_LAYERED;
        SetWindowLong(hwnd, GWL_EXSTYLE, ex_style);
        
        m_IsThemeTransparent = false;
    }
    
    this->ConfigureImGuiStyle(); // update imgui style
}

void Window::EnableAlwaysOnTop(bool enable) {
#ifdef WIN32
    HWND hwnd = glfwGetWin32Window(m_Window);
    SetWindowPos(hwnd, enable ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
#endif // WIN32
}

void Window::ConfigureImGuiStyle()const {
    ImGuiIO& io = ImGui::GetIO();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImVec4* colors = style.Colors;
    
    ImVec4 col_bg                         = ImVec4(0.043f, 0.043f, 0.043f, 1.00f);

    ImVec4 col_red                        = ImVec4(0.898f, 0.224f, 0.208f, 1.00f);
    ImVec4 col_orange                     = ImVec4(1.000f, 0.596f, 0.000f, 1.00f);
    ImVec4 col_white                      = ImVec4(1.000f, 1.000f, 1.000f, 1.00f);
    ImVec4 col_grey                       = ImVec4(0.100f, 0.100f, 0.100f, 1.00f);
    ImVec4 col_dark_grey                  = ImVec4(0.500f, 0.500f, 0.500f, 1.00f);
    ImVec4 col_dark_grey2                 = ImVec4(0.150f, 0.150f, 0.150f, 1.00f);
    ImVec4 col_dark_grey3                 = ImVec4(0.250f, 0.250f, 0.250f, 1.00f);
    ImVec4 col_dark_grey4                 = ImVec4(0.080f, 0.080f, 0.080f, 1.00f);
    ImVec4 col_dark_grey5                 = ImVec4(0.120f, 0.120f, 0.120f, 1.00f);
    ImVec4 col_empty                      = ImVec4(0.000f, 0.000f, 0.000f, 0.00f);
    
    if (!m_IsThemeDark) {
        col_bg         = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
        col_white      = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        col_dark_grey  = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
        col_dark_grey2 = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
        col_dark_grey3 = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
        col_dark_grey4 = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
        col_dark_grey5 = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
        col_red        = ImVec4(0.80f, 0.20f, 0.20f, 1.00f);
        col_orange     = ImVec4(0.95f, 0.55f, 0.00f, 1.00f);
    }

    if (m_IsThemeTransparent) col_bg      = ImVec4(0.000f, 0.000f, 0.000f, 0.00f);
    
    colors[ImGuiCol_WindowBg]             = col_bg;
    colors[ImGuiCol_ChildBg]              = col_bg;
    colors[ImGuiCol_PopupBg]              = col_dark_grey4;
    
    colors[ImGuiCol_Text]                 = col_white;
    colors[ImGuiCol_TextDisabled]         = col_dark_grey;
    
    colors[ImGuiCol_FrameBg]              = col_dark_grey5;
    colors[ImGuiCol_FrameBgHovered]       = col_orange;
    colors[ImGuiCol_FrameBgActive]        = col_red;
    
    colors[ImGuiCol_Button]               = col_red;
    colors[ImGuiCol_ButtonHovered]        = col_orange;
    colors[ImGuiCol_ButtonActive]         = col_red;
    
    colors[ImGuiCol_Header]               = col_red;
    colors[ImGuiCol_HeaderHovered]        = col_orange;
    colors[ImGuiCol_HeaderActive]         = col_red;
    
    colors[ImGuiCol_CheckMark]            = col_orange;
    colors[ImGuiCol_SliderGrab]           = col_orange;
    colors[ImGuiCol_SliderGrabActive]     = col_red;
    
    colors[ImGuiCol_ScrollbarBg]          = col_dark_grey;
    colors[ImGuiCol_ScrollbarGrab]        = col_dark_grey3;
    colors[ImGuiCol_ScrollbarGrabHovered] = col_orange;
    colors[ImGuiCol_ScrollbarGrabActive]  = col_red;
    
    colors[ImGuiCol_Tab]                  = col_dark_grey2;
    colors[ImGuiCol_TabHovered]           = col_orange;
    colors[ImGuiCol_TabActive]            = col_red;
    colors[ImGuiCol_TabUnfocused]         = col_dark_grey2;
    colors[ImGuiCol_TabUnfocusedActive]   = col_red;
    
    colors[ImGuiCol_Border]               = col_red;
    colors[ImGuiCol_BorderShadow]         = col_empty;

    colors[ImGuiCol_TitleBg]              = col_red;
    colors[ImGuiCol_TitleBgActive]        = col_red;
    colors[ImGuiCol_TitleBgCollapsed]     = col_dark_grey;

    colors[ImGuiCol_ResizeGrip]           = col_bg;
    colors[ImGuiCol_ResizeGripHovered]    = col_orange;
    colors[ImGuiCol_ResizeGripActive]     = col_orange;
    
    style.FrameRounding     = 4.0f;
    style.GrabRounding      = 3.0f;
    style.WindowRounding    = 4.0f;
    style.ScrollbarRounding = 3.0f;
}

void Window::cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    Window* user_window = static_cast<Window*>(glfwGetWindowUserPointer(window));

    user_window->m_CursorPositionX = xpos;
    user_window->m_CursorPositionY = ypos;
    if (!user_window->m_IsDragging) return;

    int window_x = 0;
    int window_y = 0;
    glfwGetWindowPos(window, &window_x, &window_y);
    
    glfwSetWindowPos(window, window_x + user_window->m_CursorPositionX - user_window->m_CursorOffsetX,
                             window_y + user_window->m_CursorPositionY - user_window->m_CursorOffsetY);
}

void Window::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    Window* user_window = static_cast<Window*>(glfwGetWindowUserPointer(window));

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {

            // check if mouse is hover the main menu bar
            ImGuiIO& io = ImGui::GetIO();
            ImVec2 menu_bar_min = ImVec2(0, 0);
            ImVec2 menu_bar_max = ImVec2(io.DisplaySize.x, ImGui::GetFrameHeight());

            if (ImGui::IsMouseHoveringRect(menu_bar_min, menu_bar_max, false)) { // check the menubar rect

                user_window->m_IsDragging = true;
                glfwGetCursorPos(window, &user_window->m_CursorOffsetX, &user_window->m_CursorOffsetY);
            }
            else user_window->m_IsDragging = false; // don't drag if not over menu
        }
        else if (action == GLFW_RELEASE) {
            user_window->m_IsDragging = false;
        }
    }
}
