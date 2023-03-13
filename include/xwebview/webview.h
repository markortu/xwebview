// Copyright 2023 xWebview
// Author: Marc Ortuño

#pragma once

#include <xwebview/window.h>
#include <xwebview/types.h>

#include <memory>
#include <unordered_map>

namespace xwebview {

  class Webview : public Window {
    struct Impl;

  public:
    Webview(void* hWnd = nullptr);
    ~Webview();

    //Settings
    void enableDevTools(bool state);
    void enableContextMenu(bool state);
    void enableZoom(bool state);
    void enableAcceleratorKeys(bool state);

    // View
    void setWebviewPosition(const ViewRect& rect);
    void showWebview(bool state);

    // Content
    void navigate(const std::string& url);
    const std::string& getUrl();
    void setHtml(const std::string& html);
    void onSourceChanged(const std::string& url);
    void onContentLoaded(bool success);

    // functionality
    void injectScript(const std::string& script);
    void executeScript(const std::string& script);
    void addCallback(const std::string& name, MessageCallback callback);
    void removeCallback(const std::string& name);
    void onMessage(const std::string& message);

    //// Interoperability
    //void addCallback();
    //void removeCallback();
    //void evaluate();

    // Embedding

  private:
    void resizeWebview(const ViewSize& size);

    std::unique_ptr<Impl> pImpl_{nullptr};
    std::unordered_map<std::string, MessageCallback> callbacks_;
  };
}  // namespace xwebview
