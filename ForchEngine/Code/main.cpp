#include "pch.h"
#include "Window.h"
#include "UI.h"

constexpr int WINDOW_RESOLUTION_WIDTH = 800;
constexpr int WINDOW_RESOLUTION_HEIGHT = 400;
constexpr std::string_view WINDOW_TITLE = "Forch Engine";

#ifdef WIN32
	#define main int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
#else
	#define main int main()
#endif // WIN32

main {
    std::shared_ptr<Window> window = std::make_shared<Window>(WINDOW_RESOLUTION_WIDTH, WINDOW_RESOLUTION_HEIGHT, WINDOW_TITLE.data());
    std::shared_ptr<ScannersManager> scanners_manager = std::make_shared<ScannersManager>();
    std::unique_ptr<UI> ui = std::make_unique<UI>(window, scanners_manager);

    while (window->IsOpen()) {
        window->ClearScreen();
        window->BeginImGuiFrame();
        window->CheckClose();

        ui->Render();

        window->EndImGuiFrame();
        window->SwapBuffers();
        window->PollEvents();
    }

    return 0;
}