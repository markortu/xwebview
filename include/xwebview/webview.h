// Copyright 2023 xWebview
// Author: Marc Ortuño

#pragma once

#include <xwebview/window.h>

#include <memory>

namespace xwebview {

  class WebView : public Window {
    struct Impl;

  public:
    WebView(void* hWnd = nullptr);
    ~WebView();

    //Settings
    void enableDevTools(bool state);
    void enableContextMenu(bool state);
    void enableZoom(bool state);
    void enableAcceleratorKeys(bool state);

    // View
    void resizeWebview(size_t width, size_t height);
    void showWebview(bool state);


    //// Content
    //void setUrl();
    //void setHtml();

    //// Interoperability
    //void addCallback();
    //void removeCallback();
    //void evaluate();

    // Embedding

  private:
    std::unique_ptr<Impl> pImpl_{nullptr};
  };
}  // namespace xwebview
