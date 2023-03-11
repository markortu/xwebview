// Copyright 2023 xWebview
// Author: Marc Ortuño

#pragma once

#include <windows.h>

#include <future>
#include <optional>

#include "xwebview/window.h"

namespace xwebview {
  const wchar_t CLASS_NAME[] = L"xWebView Window Class";

  struct Window::Impl {
    WNDCLASS wc = {};
    HWND hwnd;
    static inline HINSTANCE hInstance;
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void registerWindowClass();
    void enableDpiAwareness();

    static const inline UINT WM_POSTMESSAGESAFE = RegisterWindowMessageW(L"PostMessageSafe");
    bool isThreadSafe() const;
    template <typename Func> auto postMessageSafe(Func &&);
    std::thread::id windowThreadId = std::this_thread::get_id();
  };

  inline bool Window::Impl::isThreadSafe() const {
    return std::this_thread::get_id() == windowThreadId;
  }

  class SafeCall {
  public:
    virtual ~SafeCall() = default;
  };

  template <typename Return> class SafeCallMessage : public SafeCall {
    using MessageCallback = std::function<Return()>;

  private:
    MessageCallback func_;
    std::promise<Return> &result;

  public:
    SafeCallMessage(MessageCallback &&func, std::promise<Return> &result)
        : func_(func), result(result) {}
    ~SafeCallMessage() override { result.set_value(func_()); }
  };

  template <> class SafeCallMessage<void> : public SafeCall {
    using MessageCallback = std::function<void()>;

  private:
    MessageCallback func_;

  public:
    SafeCallMessage(MessageCallback &&func) : func_(func) {}
    ~SafeCallMessage() override { func_(); }
  };

  template <typename Func> inline auto Window::Impl::postMessageSafe(Func &&func) {
    using return_t = typename decltype(std::function(func))::result_type;

    if constexpr (std::is_same_v<return_t, void>) {
      PostMessage(hwnd, WM_POSTMESSAGESAFE, 0,
                  reinterpret_cast<LPARAM>(new SafeCallMessage<return_t>(std::function(func))));
    } else {
      std::promise<return_t> result;
      PostMessage(
          hwnd, WM_POSTMESSAGESAFE, 0,
          reinterpret_cast<LPARAM>(new SafeCallMessage<return_t>(std::function(func), result)));

      return result.get_future().get();
    }
  }

  inline LRESULT Window::Impl::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    auto *window = reinterpret_cast<Window::Impl *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    if (window) {
      switch (uMsg) {
        case WM_GETMINMAXINFO: {
        } break;
        case WM_SIZE:
          break;
        case WM_DESTROY:
        case WM_CLOSE:
          PostQuitMessage(0);
          break;
        case WM_MOVING:
          break;
        case WM_EXITSIZEMOVE:
          break;
      }
      if (uMsg == window->WM_POSTMESSAGESAFE) {
        delete reinterpret_cast<SafeCall *>(lParam);
      }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }

  inline void Window::Impl::registerWindowClass() {
    if (!hInstance) {
      hInstance = GetModuleHandleW(nullptr);

      wc.lpfnWndProc = windowProc;
      wc.hInstance = hInstance;
      wc.lpszClassName = CLASS_NAME;

      if (!RegisterClass(&wc)) {
        throw std::system_error(static_cast<int>(GetLastError()), std::system_category());
      }
    }
  }

  inline void Window::Impl::enableDpiAwareness() {
    auto *shcoredll = LoadLibraryW(L"Shcore.dll");
    auto set_process_dpi_awareness = reinterpret_cast<HRESULT(CALLBACK *)(DWORD)>(
        GetProcAddress(shcoredll, "SetProcessDpiAwareness"));

    if (set_process_dpi_awareness) {
      set_process_dpi_awareness(2);
    } else {
      auto *user32dll = LoadLibraryW(L"user32.dll");
      auto set_process_dpi_aware
          = reinterpret_cast<bool(CALLBACK *)()>(GetProcAddress(user32dll, "SetProcessDPIAware"));
      if (set_process_dpi_aware) {
        set_process_dpi_aware();
      }
    }
  }

}  // namespace xwebview
