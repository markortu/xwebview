// Copyright 2023 xWebview
// Author: Marc Ortuño

#include "xwebview/window.h"

#include <system_error>

#include "window_impl.h"

using namespace xwebview;

Window::Window(void* hwnd) : pImpl_{std::make_unique<Impl>()} {
  // Register window class
  pImpl_->registerWindowClass();

  if (hwnd != nullptr) {
    pImpl_->hwnd = static_cast<HWND>(hwnd);
  } else {
    pImpl_->hwnd = CreateWindowEx(0,                    // Optional window styles.
                                  CLASS_NAME,           // Window class
                                  L"xWebView Window",   // Window text
                                  WS_OVERLAPPEDWINDOW,  // Window style

                                  // Size and position
                                  CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

                                  nullptr,            // Parent window
                                  nullptr,            // Menu
                                  pImpl_->hInstance,  // Instance handle
                                  nullptr             // Additional application data
    );

    if (!pImpl_->hwnd) {
      throw std::system_error(static_cast<int>(GetLastError()), std::system_category());
    }
  }

  pImpl_->enableDpiAwareness();

  SetWindowLongPtr(pImpl_->hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&pImpl_));
}

Window::~Window() {}

void Window::run() {
  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

void Window::hide() {
  if (!pImpl_->isThreadSafe()) {
    return pImpl_->postMessageSafe([this]() { hide(); });
  }
  ShowWindow(pImpl_->hwnd, SW_HIDE);
}

void Window::show() {
  if (!pImpl_->isThreadSafe()) {
    return pImpl_->postMessageSafe([this]() { show(); });
  }
  ShowWindow(pImpl_->hwnd, SW_SHOW);
}

void* Window::getNativeWindow() { return static_cast<void*>(pImpl_->hwnd); }
