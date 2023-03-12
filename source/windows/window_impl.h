// Copyright 2023 xWebview
// Author: Marc Ortuño

#pragma once

#include "xwebview/window.h"

#include <windows.h>
#include <future>
#include <optional>
#include <CommCtrl.h>

namespace xwebview {
  const wchar_t CLASS_NAME[] = L"xWebView Window Class";

  struct Window::Impl {
    WNDCLASS wc = {};
    HWND hwnd;
    static inline HINSTANCE hInstance;
    static LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void registerWindowClass();
    void enableDpiAwareness();

    std::thread::id windowThreadId = std::this_thread::get_id();
    bool isThreadSafe() const;
    static const inline UINT WM_POSTMESSAGESAFE = RegisterWindowMessage(L"PostMessageSafe");
    template <typename Func> auto postMessageSafe(Func &&);

    static LRESULT CALLBACK customWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                             UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
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
    using ResultType = typename decltype(std::function(func))::result_type;

    if constexpr (std::is_same_v<ResultType, void>) {
      PostMessage(hwnd, WM_POSTMESSAGESAFE, 0,
                  reinterpret_cast<LPARAM>(new SafeCallMessage<ResultType>(std::function(func))));
    } else {
      std::promise<ResultType> result;
      PostMessage(
          hwnd, WM_POSTMESSAGESAFE, 0,
          reinterpret_cast<LPARAM>(new SafeCallMessage<ResultType>(std::function(func), result)));

      return result.get_future().get();
    }
  }

  inline LRESULT Window::Impl::windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    auto *window = reinterpret_cast<class Window *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    if (window) {
      switch (uMsg) {
        case WM_GETMINMAXINFO: {
          auto *info = reinterpret_cast<MINMAXINFO *>(lParam);
          if (window->maxSize_.first) {
            info->ptMaxTrackSize.x = static_cast<int>(window->maxSize_.first);
          }
          if (window->maxSize_.second) {
            info->ptMaxTrackSize.y = static_cast<int>(window->maxSize_.second);
          }
          if (window->minSize_.first) {
            info->ptMinTrackSize.x = static_cast<int>(window->minSize_.first);
          }
          if (window->minSize_.second) {
            info->ptMinTrackSize.y = static_cast<int>(window->minSize_.second);
          }
        } break;
        case WM_SHOWWINDOW:
          if (window->onShowWindow) window->onShowWindow(static_cast<BOOL>(wParam));
          break;
        case WM_SIZE:
          if (window->onWindowResize) window->onWindowResize(window->getSize());
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
      if (uMsg == window->pImpl_->WM_POSTMESSAGESAFE) {
        delete reinterpret_cast<SafeCall *>(lParam);
      }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
  }

  inline LRESULT Window::Impl::customWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                                  LPARAM lParam, UINT_PTR uIdSubclass,
                                                  DWORD_PTR dwRefData) {
    auto *window = reinterpret_cast<class Window *>(dwRefData);

    if (window) {
      switch (uMsg) {
        case WM_GETMINMAXINFO: {
          auto *info = reinterpret_cast<MINMAXINFO *>(lParam);
          if (window->maxSize_.first) {
            info->ptMaxTrackSize.x = static_cast<int>(window->maxSize_.first);
          }
          if (window->maxSize_.second) {
            info->ptMaxTrackSize.y = static_cast<int>(window->maxSize_.second);
          }
          if (window->minSize_.first) {
            info->ptMinTrackSize.x = static_cast<int>(window->minSize_.first);
          }
          if (window->minSize_.second) {
            info->ptMinTrackSize.y = static_cast<int>(window->minSize_.second);
          }
        } break;
        case WM_SHOWWINDOW:
          if (window->onShowWindow) window->onShowWindow(static_cast<BOOL>(wParam));
          break;
        case WM_SIZE:
          if (window->onWindowResize) window->onWindowResize(window->getSize());
          break;
      }
      if (uMsg == window->pImpl_->WM_POSTMESSAGESAFE) {
        delete reinterpret_cast<SafeCall *>(lParam);
        return NULL;
      }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
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

  inline void SetupDPIAwarenessPreWin81() {
    HINSTANCE user32Dll = LoadLibrary(L"user32.dll");

    if (!user32Dll) {
      return;
    }

    typedef BOOL(WINAPI * SetProcessDPIAware)(void);
    SetProcessDPIAware setProcessDPIAware
        = (SetProcessDPIAware)(GetProcAddress(user32Dll, "SetProcessDPIAware"));

    if (setProcessDPIAware) setProcessDPIAware();

    FreeLibrary(user32Dll);
  }

  inline void Window::Impl::enableDpiAwareness() {
    HMODULE shCoreDll = LoadLibraryW(L"Shcore.dll");
    if (shCoreDll) {
      enum ProcessDpiAwareness {
        ProcessDpiUnaware = 0,
        ProcessSystemDpiAware = 1,
        ProcessPerMonitorDpiAware = 2
      };

      typedef HRESULT(WINAPI * SetProcessDpiAwareness)(ProcessDpiAwareness);
      SetProcessDpiAwareness setProcessDpiAwareness
          = (SetProcessDpiAwareness)(GetProcAddress(shCoreDll, "SetProcessDpiAwareness"));

      if (!setProcessDpiAwareness
          || setProcessDpiAwareness(ProcessPerMonitorDpiAware) == E_INVALIDARG) {
        SetupDPIAwarenessPreWin81();
      }

      FreeLibrary(shCoreDll);
    } else
      SetupDPIAwarenessPreWin81();
  }

}  // namespace xwebview
