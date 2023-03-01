// Copyright 2023 xWebview
// Author: Marc Ortuño

#pragma once

#include <memory>

namespace xwebview {

  namespace detail {
    struct window_impl;
  }

  class window {
  public:
    window();
    ~window();

    // Process
    void run();
    void destroy();
    void attach_webview();

    // Window Style
    void set_title();
    void set_size();
    void set_max_size();
    void set_min_size();
    void set_resizable();
    void hide();
    void show();

  private:
    std::unique_ptr<detail::window_impl> pimpl_;
  };
}  // namespace xwebview
