#include <xwebview/window.h>
#include <xwebview/webview.h>
#include <thread>

void InterfaceThread(xwebview::Window* window) { window->show(); }

int main() {
  auto window = xwebview::Window();
  std::thread interfaceThread([&] {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    InterfaceThread(&window);
  });
  interfaceThread.detach();

  auto webview = xwebview::WebView(window.getNativeWindow());
  webview.setResizable(true);
  webview.setTitle("Mark Webview");
  webview.setMinSize(600, 600);

  webview.enableDevTools(true);

  window.run();
}
