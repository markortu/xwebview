// Copyright 2023 xWebview
// Author: Marc Ortuño

#pragma once

#include <memory>

namespace xwebview {

  class Window {
    struct Impl;

  public:
    Window(void* hwnd = nullptr);
    ~Window();

    // Process
    void run();
    //void destroy();
    //void attachWebview();

    // Window Style
    //void setTitle();
    //void setSize();
    //void setMaxSize();
    //void setMinSize();
    //void setResizable();
    void hide();
    void show();

    void* getNativeWindow();
  private:
    std::unique_ptr<Impl> pImpl_{nullptr};
    std::pair<std::size_t, std::size_t> minSize_, maxSize_;

  };
}  // namespace xwebview
