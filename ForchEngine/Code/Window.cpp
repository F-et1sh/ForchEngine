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

    m_Window = glfwCreateWindow(resolution_x, resolution_y, title.c_str(), nullptr, nullptr);
    if (!m_Window) {
        glfwTerminate();
        std::cerr << "ERROR : Failed to Initialize Window" << std::endl;
        return;
    }
    
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
    
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    ImVec4* colors = style.Colors;
    
    ImVec4 col_bg                         = ImVec4(0.043f, 0.043f, 0.043f, 1.00f);
    ImVec4 col_red                        = ImVec4(0.898f, 0.224f, 0.208f, 1.00f);
    ImVec4 col_orange                     = ImVec4(1.000f, 0.596f, 0.000f, 1.00f);
    ImVec4 col_white                      = ImVec4(1.000f, 1.000f, 1.000f, 1.00f);
    
    colors[ImGuiCol_WindowBg]             = col_bg;
    colors[ImGuiCol_ChildBg]              = col_bg;
    colors[ImGuiCol_PopupBg]              = ImVec4(0.08f, 0.08f, 0.08f, 0.98f);
    
    colors[ImGuiCol_Text]                 = col_white;
    colors[ImGuiCol_TextDisabled]         = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    
    colors[ImGuiCol_FrameBg]              = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
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
    
    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = col_orange;
    colors[ImGuiCol_ScrollbarGrabActive]  = col_red;
    
    colors[ImGuiCol_Tab]                  = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TabHovered]           = col_orange;
    colors[ImGuiCol_TabActive]            = col_red;
    colors[ImGuiCol_TabUnfocused]         = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]   = col_red;
    
    colors[ImGuiCol_Border]               = col_red;
    colors[ImGuiCol_BorderShadow]         = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    colors[ImGuiCol_TitleBg]              = col_red;
    colors[ImGuiCol_TitleBgActive]        = col_red;
    colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);

    colors[ImGuiCol_ResizeGrip]           = col_bg;
    colors[ImGuiCol_ResizeGripHovered]    = col_orange;
    colors[ImGuiCol_ResizeGripActive]     = col_orange;
    
    style.FrameRounding     = 4.0f;
    style.GrabRounding      = 3.0f;
    style.WindowRounding    = 4.0f;
    style.ScrollbarRounding = 3.0f;

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
    glClearColor(0.085f, 0.085f, 0.085f, 1.0f);
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
