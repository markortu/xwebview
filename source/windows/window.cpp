// Copyright 2023 xWebview
// Author: Marc Ortuño

#include <system_error>

#include "window_impl.h"

using namespace xwebview;

Window::Window(void* hwnd) : pImpl_{std::make_unique<Impl>()} {
  if (hwnd != nullptr) {
    pImpl_->hwnd = static_cast<HWND>(hwnd);
    SetWindowSubclass(pImpl_->hwnd, &Impl::customWindowProc, 1, reinterpret_cast<DWORD_PTR>(this));
  } else {
    // Register window class
    pImpl_->registerWindowClass();

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
  SetWindowLongPtr(pImpl_->hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

  pImpl_->enableDpiAwareness();
}

Window::~Window() {}

void Window::run() {
  MSG uMsg;
  while (GetMessage(&uMsg, nullptr, 0, 0)) {
    TranslateMessage(&uMsg);
    DispatchMessage(&uMsg);
  }
}

void Window::setTitle(const std::string& title) {
  SetWindowText(pImpl_->hwnd, s2ws(title).c_str());
}

void Window::setSize(const ViewSize& size) {
  if (!pImpl_->isThreadSafe()) {
    return pImpl_->postMessageSafe([=] { setSize(size); });
  }
  RECT r;
  r.left = r.top = 0;
  r.right = size.first;
  r.bottom = size.second;
  AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, 0);
  SetWindowPos(pImpl_->hwnd, nullptr, r.left, r.top, r.right - r.left, r.bottom - r.top,
               SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_FRAMECHANGED);
}

ViewSize Window::getSize() const {
  if (!pImpl_->isThreadSafe()) {
    return pImpl_->postMessageSafe([=] { return getSize(); });
  }
  RECT rect;
  GetClientRect(pImpl_->hwnd, &rect);
  return ViewSize{rect.right - rect.left, rect.bottom - rect.top};
}

void Window::setMaxSize(const ViewSize& size) { maxSize_ = size; }

ViewSize Window::getMaxSize() const { return maxSize_; }

void Window::setMinSize(const ViewSize& size) { minSize_ = size; }

ViewSize Window::getMinSize() const { return minSize_; }

void Window::setResizable(bool state) {
  auto currentStyle = GetWindowLong(pImpl_->hwnd, GWL_STYLE);
  if (state) {
    currentStyle |= (WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
  } else {
    currentStyle &= ~(WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
  }
  SetWindowLong(pImpl_->hwnd, GWL_STYLE, currentStyle);
}

void Window::hide() {
  if (!pImpl_->isThreadSafe()) {
    return pImpl_->postMessageSafe([=]() { hide(); });
  }
  ShowWindow(pImpl_->hwnd, SW_HIDE);
}

void Window::show() {
  if (!pImpl_->isThreadSafe()) {
    return pImpl_->postMessageSafe([=]() { show(); });
  }
  ShowWindow(pImpl_->hwnd, SW_SHOW);
}

void Window::close() {
  if (!pImpl_->isThreadSafe()) {
    return pImpl_->postMessageSafe([=] { close(); });
  }
  DestroyWindow(pImpl_->hwnd);
}

void* Window::getNativeWindow() { return static_cast<void*>(pImpl_->hwnd); }
