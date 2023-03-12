// Copyright 2023 xWebview
// Author: Marc Ortuño

#pragma once

#include <memory>
#include <string>
#include <functional>

namespace xwebview {
    using WindowSize = std::pair<std::size_t, std::size_t>;
    using OnWindowResize = std::function<void(WindowSize)>;
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

    void setSize(std::size_t width, std::size_t height);
    WindowSize getSize() const;
    void setMaxSize(std::size_t width, std::size_t height);
    WindowSize getMaxSize() const;
    void setMinSize(std::size_t width, std::size_t height);
    WindowSize getMinSize() const;

    void setResizable(bool state);
    void hide();
    void show();

    void* getNativeWindow();

    OnWindowResize onWindowResize;
    OnShowWindow onShowWindow;
  protected:
    std::unique_ptr<Impl> pImpl_{nullptr};

  private:
    WindowSize minSize_, maxSize_;

  };
}  // namespace xwebview
