// Copyright 2023 xWebview
// Author: Marc Ortuño

#include <WebView2.h>
#include <WebView2EnvironmentOptions.h>
#include <wil/com.h>
#include <wil/stl.h>
#include <wil/win32_helpers.h>
#include <wrl.h>

#include <atomic>
#include <optional>

#include "xwebview/webview.h"

namespace xwebview {
  struct Webview::Impl {
    bool initWebView(HWND hWnd, bool enableRemoteDebugging = false);
    wil::com_ptr<ICoreWebView2Controller> webviewController_;
    wil::com_ptr<ICoreWebView2> webview_;
    std::vector<LPCWSTR> injectedScripts_;
  };

  inline bool Webview::Impl::initWebView(HWND hWnd, bool enableRemoteDebugging) {
    using namespace Microsoft::WRL;

    ComPtr options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
    if (enableRemoteDebugging) {
      options->put_AdditionalBrowserArguments(L"--remote-debugging-port=9222");
    }

    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    std::wstring temp;
    wil::GetEnvironmentVariableW(L"TEMP", temp);

    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    flag.test_and_set();

    CreateCoreWebView2EnvironmentWithOptions(
        nullptr, temp.c_str(), options.Get(),
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [&](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
              env->CreateCoreWebView2Controller(
                  hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                            [&](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                              webviewController_ = controller;
                              webviewController_->get_CoreWebView2(&webview_);
                              flag.clear();
                              return S_OK;
                            })
                            .Get());
              return S_OK;
            })
            .Get());

    MSG msg = {};
    while (flag.test_and_set() && GetMessage(&msg, nullptr, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    if (!webviewController_ || !webview_) {
      return false;
    }

    wil::com_ptr<ICoreWebView2Settings> settings;
    webview_->get_Settings(&settings);
    settings->put_AreDevToolsEnabled(false);
    settings->put_AreDefaultContextMenusEnabled(false);
    settings->put_IsZoomControlEnabled(false);
    if (auto settings3 = settings.try_query<ICoreWebView2Settings3>(); settings3) {
      settings3->put_AreBrowserAcceleratorKeysEnabled(false);
    }

    return true;
  }
}  // namespace xwebview