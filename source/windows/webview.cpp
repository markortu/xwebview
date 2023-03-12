// Copyright 2023 xWebview
// Author: Marc Ortuño

#include "webview_impl.h"
#include "window_impl.h"

using namespace xwebview;

WebView::WebView(void* hWnd) : Window{hWnd}, pImpl_(std::make_unique<Impl>()) {
  if (!pImpl_->initWebView(static_cast<HWND>(getNativeWindow()))) {
    throw std::exception("Cannot initialize webview");
  }

  onWindowResize = [=](WindowSize size) {
    resizeWebview(size.first, size.second);
  };

  onShowWindow = [=](bool state) { showWebview(state); };
}

WebView::~WebView() = default;

void WebView::enableDevTools(bool state) {
  if (!Window::pImpl_->isThreadSafe()) {
    return Window::pImpl_->postMessageSafe([=] { return enableDevTools(state); });
  }

  wil::com_ptr<ICoreWebView2Settings> settings;
  pImpl_->webview_->get_Settings(&settings);
  settings->put_AreDevToolsEnabled(state);
}

void WebView::enableContextMenu(bool state) {}

void WebView::enableZoom(bool state) {}

void WebView::enableAcceleratorKeys(bool state) {}

void WebView::resizeWebview(size_t width, size_t height) {
  if (pImpl_->webviewController_) {
    pImpl_->webviewController_->put_Bounds(RECT{0, 0, static_cast<LONG>(width), static_cast<LONG>(height)});
  }
}

void xwebview::WebView::showWebview(bool state) {
  if (pImpl_->webviewController_) {
    pImpl_->webviewController_->put_IsVisible(static_cast<BOOL>(state));
  }
}
