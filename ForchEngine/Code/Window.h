#pragma once

class Window {
public:
	Window(size_t resolution_x, size_t resolution_y, const std::string& title);
	~Window();

	void CheckClose();
	void ClearScreen();

	void BeginImGuiFrame();
	void EndImGuiFrame();

	void SwapBuffers();
	void PollEvents();

	inline bool IsOpen() const noexcept { return m_IsRunning; }
	inline void CallCloseRequest()const noexcept { glfwSetWindowShouldClose(m_Window, GLFW_TRUE); }

	void EnableTransparentMode(bool enable);
	void EnableAlwaysOnTop(bool enable);

private:
	void ConfigureImGuiStyle()const;

private:
	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

private:
	bool m_IsRunning = false;
	GLFWwindow* m_Window = nullptr;

private:
	bool m_IsDragging = false;

	double m_CursorPositionX = 0.0;
	double m_CursorPositionY = 0.0;

	double m_CursorOffsetX = 0.0;
	double m_CursorOffsetY = 0.0;

private:
	bool m_IsThemeTransparent = true;
	bool m_IsThemeDark = false;
};