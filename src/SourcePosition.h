#pragma once
#include <cstddef>
#include <optional>
#include <string_view>

// Return a zero-based source line only for a unique literal match.
std::optional<size_t> FindSourceLine(std::string_view source, std::string_view text, size_t utf8Offset);
