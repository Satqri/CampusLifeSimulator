#include "utils/AssetPath.h"

#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace cls {
namespace {

#ifdef _WIN32

bool pathExists(const std::string& path) {
    return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

bool isAbsolutePath(const std::string& path) {
    if (path.size() >= 3 && path[1] == ':'
        && (path[2] == '\\' || path[2] == '/')) {
        return true;
    }
    return path.size() >= 2 && path[0] == '\\' && path[1] == '\\';
}

std::string normalizeSeparators(std::string path) {
    for (char& c : path) {
        if (c == '/') c = '\\';
    }
    return path;
}

std::string joinPath(const std::string& base, const std::string& relative) {
    if (base.empty() || isAbsolutePath(relative)) return normalizeSeparators(relative);
    if (base.back() == '\\' || base.back() == '/') return normalizeSeparators(base + relative);
    return normalizeSeparators(base + "\\" + relative);
}

std::string fullPath(const std::string& path) {
    const DWORD needed = GetFullPathNameA(path.c_str(), 0, nullptr, nullptr);
    if (needed == 0) return normalizeSeparators(path);

    std::vector<char> buffer(needed + 1, '\0');
    const DWORD written = GetFullPathNameA(path.c_str(), static_cast<DWORD>(buffer.size()),
                                           buffer.data(), nullptr);
    if (written == 0 || written >= buffer.size()) return normalizeSeparators(path);
    return normalizeSeparators(std::string(buffer.data(), written));
}

std::string currentDirectory() {
    const DWORD needed = GetCurrentDirectoryA(0, nullptr);
    if (needed == 0) return {};

    std::vector<char> buffer(needed + 1, '\0');
    const DWORD written = GetCurrentDirectoryA(static_cast<DWORD>(buffer.size()), buffer.data());
    if (written == 0 || written >= buffer.size()) return {};
    return normalizeSeparators(std::string(buffer.data(), written));
}

std::string executableDirectory() {
    std::vector<char> buffer(MAX_PATH, '\0');
    DWORD written = GetModuleFileNameA(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (written == 0) return {};

    while (written >= buffer.size() - 1) {
        buffer.assign(buffer.size() * 2, '\0');
        written = GetModuleFileNameA(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (written == 0) return {};
    }

    std::string path(buffer.data(), written);
    path = normalizeSeparators(path);
    const std::size_t slash = path.find_last_of("\\");
    if (slash == std::string::npos) return {};
    return path.substr(0, slash);
}

std::string parentDirectory(std::string path) {
    path = normalizeSeparators(path);
    while (path.size() > 3 && path.back() == '\\') {
        path.pop_back();
    }

    const std::size_t slash = path.find_last_of("\\");
    if (slash == std::string::npos) return {};
    if (slash == 2 && path.size() >= 3 && path[1] == ':') return path.substr(0, 3);
    return path.substr(0, slash);
}

void addCandidateRoot(std::vector<std::string>& roots, const std::string& root) {
    if (root.empty()) return;
    const std::string normalized = normalizeSeparators(root);
    for (const auto& existing : roots) {
        if (existing == normalized) return;
    }
    roots.push_back(normalized);
}

void addRootAndParents(std::vector<std::string>& roots, std::string start) {
    start = fullPath(start);
    while (!start.empty()) {
        addCandidateRoot(roots, start);
        const std::string parent = parentDirectory(start);
        if (parent.empty() || parent == start) break;
        start = parent;
    }
}

#endif

} // namespace

std::string resolveAssetPath(const std::string& relativePath) {
#ifdef _WIN32
    if (relativePath.empty()) return relativePath;
    if (isAbsolutePath(relativePath)) return fullPath(relativePath);

    std::vector<std::string> roots;
    addRootAndParents(roots, currentDirectory());
    addRootAndParents(roots, executableDirectory());

    for (const auto& root : roots) {
        const std::string candidate = joinPath(root, relativePath);
        if (pathExists(candidate)) return fullPath(candidate);
    }

    return normalizeSeparators(relativePath);
#else
    return relativePath;
#endif
}

} // namespace cls
