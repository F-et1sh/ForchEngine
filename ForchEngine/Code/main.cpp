#include "pch.h"
#include "Window.h"
#include "UI.h"

constexpr int WINDOW_RESOLUTION_WIDTH = 800;
constexpr int WINDOW_RESOLUTION_HEIGHT = 400;
constexpr std::string_view WINDOW_TITLE = "Forch Engine";

int main() {
    std::unique_ptr<Window> window = std::make_unique<Window>(WINDOW_RESOLUTION_WIDTH, WINDOW_RESOLUTION_HEIGHT, WINDOW_TITLE.data());
    std::shared_ptr<ProcessScanner> process_scanner = std::make_shared<ProcessScanner>();
    std::unique_ptr<UI> ui = std::make_unique<UI>(process_scanner);

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
