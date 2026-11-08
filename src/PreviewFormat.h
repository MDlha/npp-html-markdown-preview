#pragma once
#include <optional>
#include <string_view>

enum class PreviewFormat { Html, Markdown };

inline std::optional<PreviewFormat> FormatFromFilename(std::wstring_view filename) {
    const auto dot = filename.find_last_of(L'.');
    if (dot == filename.npos) return std::nullopt;
    auto extension = filename.substr(dot);
    const auto matches = [extension](std::wstring_view expected) {
        if (extension.size() != expected.size()) return false;
        for (size_t i = 0; i < extension.size(); ++i) {
            wchar_t c = extension[i];
            if (c >= L'A' && c <= L'Z') c += L'a' - L'A';
            if (c != expected[i]) return false;
        }
        return true;
    };
    if (matches(L".html") || matches(L".htm")) return PreviewFormat::Html;
    if (matches(L".md") || matches(L".markdown")) return PreviewFormat::Markdown;
    return std::nullopt;
}
