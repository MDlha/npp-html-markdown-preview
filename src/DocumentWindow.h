#pragma once
#include <windows.h>
#include <string>
#include <string_view>
#include <functional>
#include <cstdint>
struct HtmlSnapshot {
    uintptr_t buffer{};
    std::wstring title;
    std::string utf8;
    bool markdown{};
    std::function<void()> returnToEditor;
    // The window supplies a candidate text node and UTF-8 offset; the editor validates its snapshot.
    std::function<const wchar_t*(std::string_view,size_t)> jumpToSource;
    // Native diagnostic callback, never exposed to document scripts.
    std::function<void(bool,int,bool)> loadResult;
};
bool ShowHtmlSnapshot(HINSTANCE module, HWND owner, HtmlSnapshot snapshot);
void ForgetHtmlSnapshot(uintptr_t buffer);
void CloseHtmlSnapshots();
