#include <xwebview/window.h>
#include <xwebview/webview.h>

#include <thread>
#include <sstream>
#include <iostream>

void InterfaceThread(xwebview::Window* window) { window->show(); }

void OnMessage(std::string message){ 
    std::cout << message << std::endl;
};

int main() {
  auto window = xwebview::Window();
  std::thread interfaceThread([&] {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    InterfaceThread(&window);
  });
  interfaceThread.detach();

  auto webview = xwebview::Webview(window.getNativeWindow());
  webview.setResizable(true);
  webview.setTitle("Mark Webview");
  webview.setMinSize({600, 600});
  webview.enableDevTools(true);
  webview.enableAcceleratorKeys(true);
  webview.enableContextMenu(true);
  std::ostringstream fileNameStream;
  fileNameStream << "file://C:\\ProgramData\\VoicemodVoiceDesigner\\frontend\\index.html";
  webview.navigate("http://localhost:8080/");
  webview.addCallback("externalCallback", &OnMessage);

  std::thread removeCallback([&] {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    webview.removeCallback("externalCallback");
  });
  removeCallback.detach();

  window.run();
}
