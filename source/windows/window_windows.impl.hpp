// Copyright 2023 xWebview
// Author: Marc Ortuño

#pragma once
#include <xwebview/window.h>

#include <Windows.h>
#include <future>
#include <optional>

namespace xwebview {
 struct Window::Impl
    {
        HWND hwnd;
        WNDCLASSW wnd_class;
        std::pair<std::size_t, std::size_t> min_size, max_size;

        bool is_thread_safe() const;
        template <typename Func> auto post_safe(Func &&);
        std::thread::id m_creation_thread = std::this_thread::get_id();

        static inline HMODULE instance;
        static inline std::atomic<std::size_t> open_windows{0};
        static LRESULT CALLBACK wnd_proc(HWND, UINT, WPARAM, LPARAM);
        static const inline UINT WM_SAFE_CALL = RegisterWindowMessageW(L"safe_call");

        static std::wstring widen(const std::string &);
        static std::string narrow(const std::wstring &);

        void enable_transparency(bool, bool = false, bool = false);
        void set_background_color(const std::tuple<std::size_t, std::size_t, std::size_t, std::size_t> &);

        bool blur_enabled{false};
        bool transparency_enabled{false};
        std::tuple<std::size_t, std::size_t, std::size_t, std::size_t> background_color;
    };
}
