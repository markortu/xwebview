// Copyright 2023 xWebview
// Author: Marc Ortuño

#pragma once

#include <xwebview/window.h>

class webview {
public:
  webview();
  ~webview();

  void* get_platform_webview();

  // Content
  void set_url();
  void set_html();

  // Interoperability
  void add_callback();
  void remove_callback();
  void evaluate();

  // Embedding

private:
};
