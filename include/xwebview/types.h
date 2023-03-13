// Copyright 2023 xWebview
// Author: Marc Ortuño

#pragma once

#include <utility>
#include <codecvt>
#include <functional>
#include <string>

namespace xwebview {
  using ViewSize = std::pair<std::size_t, std::size_t>;
  using ViewRect = struct {
    std::size_t L;
    std::size_t R;
    std::size_t T;
    std::size_t B;
  };

  inline const std::wstring s2ws(const std::string& str) {
    return std::wstring(std::begin(str), std::end(str));
  }

  inline const std::string ws2s(const std::wstring& str) {
    using convertType = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convertType, wchar_t> converter;
    return converter.to_bytes(str);
  }

  using MessageCallback = std::function<void(std::string)>;
}  // namespace xwebview
