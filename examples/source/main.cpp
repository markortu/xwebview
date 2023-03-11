#include <xwebview/window.h>

#include <thread>

void InterfaceThread(xwebview::Window* window) { window->show(); }

int main() {
  auto window = xwebview::Window();
  std::thread interfaceThread([&] {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    InterfaceThread(&window);
  });
  interfaceThread.detach();

  std::thread secondWindow([&] {
    auto window2 = xwebview::Window(window.getNativeWindow());
    window2.show();
    });

  secondWindow.detach();
  window.run();
}
