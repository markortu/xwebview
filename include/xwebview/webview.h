// Copyright 2023 xWebview
// Author: Marc Ortuño

#pragma once

#include <xwebview/window.h>

#include <memory>

namespace xwebview {

  class WebView {
    struct Impl;

  public:
    WebView();
    ~WebView();

    void* getPlatformWebview();

    // Content
    void setUrl();
    void setHtml();

    // Interoperability
    void addCallback();
    void removeCallback();
    void evaluate();

    // Embedding

  private:
    std::unique_ptr<Impl> pImpl_{nullptr};
    std::unique_ptr<Window> pWindow_{nullptr};
  };
}  // namespace xwebview
