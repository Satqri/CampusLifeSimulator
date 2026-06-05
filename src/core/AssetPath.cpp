#include "core/AssetPath.h"

#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace cls {
namespace {

namespace fs = std::filesystem;

void addStartPath(std::vector<fs::path>& starts, const fs::path& path) {
    if (path.empty()) return;
    for (const auto& existing : starts) {
        if (existing == path) return;
    }
    starts.push_back(path);
}

fs::path executableDirectory() {
#ifdef _WIN32
    std::wstring buffer(MAX_PATH, L'\0');
    DWORD size = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (size == 0) return {};

    while (size == buffer.size()) {
        buffer.resize(buffer.size() * 2);
        size = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (size == 0) return {};
    }

    buffer.resize(size);
    return fs::path(buffer).parent_path();
#else
    return {};
#endif
}

bool findFrom(fs::path start, const fs::path& relativePath, fs::path& resolved) {
    std::error_code ec;
    start = fs::absolute(start, ec);
    if (ec) return false;

    while (!start.empty()) {
        const fs::path candidate = start / relativePath;
        if (fs::exists(candidate, ec) && !ec) {
            resolved = candidate;
            return true;
        }

        const fs::path parent = start.parent_path();
        if (parent == start) break;
        start = parent;
    }
    return false;
}

} // namespace

std::string resolveAssetPath(const std::string& relativePath) {
    const fs::path requested(relativePath);
    if (requested.is_absolute()) {
        return requested.lexically_normal().string();
    }

    std::vector<fs::path> starts;
    std::error_code ec;
    addStartPath(starts, fs::current_path(ec));
    if (!ec) {
        addStartPath(starts, fs::current_path(ec) / "build" / "Release");
    }
    addStartPath(starts, executableDirectory());

    fs::path resolved;
    for (const auto& start : starts) {
        if (findFrom(start, requested, resolved)) {
            return resolved.lexically_normal().string();
        }
    }

    return relativePath;
}

} // namespace cls
