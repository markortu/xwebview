// Copyright 2023 xWebview
// Author: Marc Ortuño

#include "nlohmann/json.hpp"
#include "webview_impl.h"
#include "window_impl.h"

using namespace xwebview;
using namespace Microsoft::WRL;

Webview::Webview(void* hWnd) : Window{hWnd}, pImpl_(std::make_unique<Impl>()) {
  if (!pImpl_->initWebView(static_cast<HWND>(getNativeWindow()))) {
    throw std::exception("Cannot initialize webview");
  }

  pImpl_->webview_->add_WebMessageReceived(
      Callback<ICoreWebView2WebMessageReceivedEventHandler>(
          [=](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) {
            wil::unique_cotaskmem_string jsonString;
            args->get_WebMessageAsJson(&jsonString);
            onMessage(ws2s(jsonString.get()).c_str());
            return S_OK;
          })
          .Get(),
      nullptr);

  pImpl_->webview_->add_NavigationCompleted(
      Callback<ICoreWebView2NavigationCompletedEventHandler>(
          [=](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
            BOOL success;
            args->get_IsSuccess(&success);
            if (success) {
              onContentLoaded(success);
            }
            return S_OK;
          })
          .Get(),
      nullptr);

  pImpl_->webview_->add_SourceChanged(
      Callback<ICoreWebView2SourceChangedEventHandler>(
          [=](ICoreWebView2* sender, ICoreWebView2SourceChangedEventArgs* args) -> HRESULT {
            wil::unique_cotaskmem_string url;
            onSourceChanged(getUrl());
            return S_OK;
          })
          .Get(),
      nullptr);

  injectScript(R"(
                window.webview = {
                    async postMessage(message) 
                    {
                        window.chrome.webview.postMessage(message);
                    }
                };
                )");

  onWindowResize = [=](ViewSize size) { resizeWebview(size); };

  onShowWindow = [=](bool state) { showWebview(state); };
}

Webview::~Webview(){};

void Webview::enableDevTools(bool state) {
  if (!Window::pImpl_->isThreadSafe()) {
    return Window::pImpl_->postMessageSafe([=] { return enableDevTools(state); });
  }

  wil::com_ptr<ICoreWebView2Settings> settings;
  pImpl_->webview_->get_Settings(&settings);
  settings->put_AreDevToolsEnabled(state);
}

void Webview::enableContextMenu(bool state) {
  if (!Window::pImpl_->isThreadSafe()) {
    return Window::pImpl_->postMessageSafe([=] { return enableZoom(state); });
  }

  wil::com_ptr<ICoreWebView2Settings> settings;
  pImpl_->webview_->get_Settings(&settings);
  settings->put_AreDefaultContextMenusEnabled(state);
}

void Webview::enableZoom(bool state) {
  if (!Window::pImpl_->isThreadSafe()) {
    return Window::pImpl_->postMessageSafe([=] { return enableZoom(state); });
  }

  wil::com_ptr<ICoreWebView2Settings> settings;
  pImpl_->webview_->get_Settings(&settings);
  settings->put_IsZoomControlEnabled(state);
}

void Webview::enableAcceleratorKeys(bool state) {
  if (!Window::pImpl_->isThreadSafe()) {
    return Window::pImpl_->postMessageSafe([=] { return enableZoom(state); });
  }

  wil::com_ptr<ICoreWebView2Settings> settings;
  pImpl_->webview_->get_Settings(&settings);
  if (auto settings3 = settings.try_query<ICoreWebView2Settings3>(); settings3) {
    settings3->put_AreBrowserAcceleratorKeysEnabled(state);
  }
}

void Webview::resizeWebview(const ViewSize& size) {
  if (pImpl_->webviewController_) {
    pImpl_->webviewController_->put_Bounds(
        RECT{0, 0, static_cast<LONG>(size.first), static_cast<LONG>(size.second)});
  }
}

void Webview::setWebviewPosition(const ViewRect& rect) {
  if (pImpl_->webviewController_) {
    pImpl_->webviewController_->put_Bounds(
        RECT{static_cast<LONG>(rect.L), static_cast<LONG>(rect.T), static_cast<LONG>(rect.R),
             static_cast<LONG>(rect.B)});
  }
}

void Webview::showWebview(bool state) {
  if (pImpl_->webviewController_) {
    pImpl_->webviewController_->put_IsVisible(static_cast<BOOL>(state));
  }
}

void Webview::navigate(const std::string& url) {
  if (!Window::pImpl_->isThreadSafe()) {
    return Window::pImpl_->postMessageSafe([=] { navigate(url); });
  }

  pImpl_->webview_->Navigate(s2ws(url).c_str());
}

const std::string& xwebview::Webview::getUrl() {
  if (!Window::pImpl_->isThreadSafe()) {
    return Window::pImpl_->postMessageSafe([=] { return getUrl(); });
  }

  wil::unique_cotaskmem_string url;
  pImpl_->webview_->get_Source(&url);
  return ws2s(url.get());
}

void Webview::setHtml(const std::string& html) {
  if (!Window::pImpl_->isThreadSafe()) {
    return Window::pImpl_->postMessageSafe([=] { setHtml(html); });
  }

  pImpl_->webview_->NavigateToString(s2ws(html).c_str());
}

void Webview::onSourceChanged(const std::string& url) {}

void Webview::onContentLoaded(bool success) {}

void Webview::injectScript(const std::string& script) {
  if (!Window::pImpl_->isThreadSafe()) {
    return Window::pImpl_->postMessageSafe([=] { injectScript(script); });
  }

  pImpl_->webview_->AddScriptToExecuteOnDocumentCreated(s2ws(script).c_str(), nullptr);
}

void Webview::executeScript(const std::string& script) {
  if (!Window::pImpl_->isThreadSafe()) {
    return Window::pImpl_->postMessageSafe([=] { executeScript(script); });
  }
  pImpl_->webview_->ExecuteScript(s2ws(script).c_str(), nullptr);
}

void Webview::addCallback(const std::string& name, MessageCallback callback) {
  callbacks_.emplace(name, callback);
  auto script = "window['" + name + "'] = function(message) { const name = '" + name + "';" +
                R"(
                    window.webview.postMessage({
                              name: name,
                              message: message,
                            });
                    }
                )";
  injectScript(script);
  executeScript(script);
}

void Webview::removeCallback(const std::string& name) {
  callbacks_.erase(name);
  auto script = "delete window['" + name + "']";
  injectScript(script);
  executeScript(script);
}

void Webview::onMessage(const std::string& message) {
  auto json = nlohmann::json::parse(message);
  if (json.is_object()) {
    if (json.contains("name") || json.contains("message")) {
      auto callback = callbacks_.find(json["name"]);
      if (callback == callbacks_.end()) {
        // No callbacks defined
        return;
      }

      callback->second(json["message"].dump());
    }
  }
}
