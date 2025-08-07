#include "pch.h"
#include "Window.h"

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

    // enable vsyn
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

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

        if (ImGui::BeginPopupModal("Close the application?", 0, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Do you really want to close the application?");
            ImGui::Separator();

            if (ImGui::Button("Yes")) {
                // TODO : Saving here
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
