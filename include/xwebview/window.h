// Copyright 2023 xWebview
// Author: Marc Ortuño

#pragma once

#include "types.h"

#include <memory>
#include <string>
#include <functional>

namespace xwebview {
    using OnWindowResize = std::function<void(ViewSize)>;
    using OnShowWindow = std::function<void(bool)>;

  class Window {
    struct Impl;

  public:
    Window(void* hwnd = nullptr);
    ~Window();

    // Process
    void run();
    void close();

    // Window Style
    void setTitle(const std::string& title);
    void setSize(const ViewSize& size);
    ViewSize getSize() const;
    void setMaxSize(const ViewSize& size);
    ViewSize getMaxSize() const;
    void setMinSize(const ViewSize& size);
    ViewSize getMinSize() const;
    void setResizable(bool state);
    void hide();
    void show();

    void* getNativeWindow();

    OnWindowResize onWindowResize;
    OnShowWindow onShowWindow;
  protected:
    std::unique_ptr<Impl> pImpl_{nullptr};

  private:
    ViewSize minSize_, maxSize_;

  };
}  // namespace xwebview
