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

private:
	bool m_IsRunning = false;
	GLFWwindow* m_Window = nullptr;
};