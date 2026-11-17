#include "path.h"

#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include <mutex>
#include <vector>

namespace platform {

// The current directory as a path the program put there. On macOS getcwd()
// resolves symbolic links (/var becomes /private/var), which Windows does not
// do, so the directory is remembered the way it was named, for as long as it
// is still the current directory.
static std::string g_logicalCwd;
static bool g_logicalCwdInit = false;
static std::mutex g_cwdMutex;

static bool SameDirectory(const std::string& a, const char* b)
{
    struct stat sa, sb;
    return stat(a.c_str(), &sa) == 0 && stat(b, &sb) == 0 && sa.st_dev == sb.st_dev && sa.st_ino == sb.st_ino;
}

std::string CurrentDirectory()
{
    std::lock_guard<std::mutex> lock(g_cwdMutex);
    if (!g_logicalCwdInit) {
        g_logicalCwdInit = true;
        const char* pwd = getenv("PWD"); // the shell's logical directory
        if (pwd != NULL && pwd[0] == '/' && SameDirectory(pwd, ".")) {
            g_logicalCwd = pwd;
        }
    }
    if (!g_logicalCwd.empty() && SameDirectory(g_logicalCwd, ".")) {
        return g_logicalCwd;
    }
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        return std::string();
    }
    return cwd;
}

int ChangeDirectory(const std::string& posixPath)
{
    // Lexical absolute form, relative to the directory we were in.
    std::string absolute = AbsolutePath(posixPath.c_str());
    if (chdir(posixPath.c_str()) != 0) {
        return -1;
    }
    std::lock_guard<std::mutex> lock(g_cwdMutex);
    g_logicalCwdInit = true;
    g_logicalCwd = SameDirectory(absolute, ".") ? absolute : std::string();
    return 0;
}

static bool Exists(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

static bool EqualNoCase(const char* a, const char* b)
{
    for (;; a++, b++) {
        unsigned char ca = static_cast<unsigned char>(*a);
        unsigned char cb = static_cast<unsigned char>(*b);
        if (ca >= 'A' && ca <= 'Z') ca = static_cast<unsigned char>(ca + 32);
        if (cb >= 'A' && cb <= 'Z') cb = static_cast<unsigned char>(cb + 32);
        if (ca != cb) return false;
        if (ca == 0) return true;
    }
}

// Resolves `s` (slashes already converted) component by component.
static std::string ResolveCase(const std::string& s)
{
    if (s.empty()) {
        return s;
    }

    // Fast path: the path exists as written.
    if (Exists(s)) {
        return s;
    }

    std::string result;
    size_t pos = 0;
    if (s[0] == '/') {
        result = "/";
        pos = 1;
    }

    bool missing = false; // once a component is missing, the rest is kept
    while (pos < s.size()) {
        size_t end = s.find('/', pos);
        bool last = (end == std::string::npos);
        std::string comp = s.substr(pos, last ? std::string::npos : end - pos);
        pos = last ? s.size() : end + 1;

        if (comp.empty()) {
            continue;
        }

        std::string name = comp;
        if (!missing && comp != "." && comp != "..") {
            std::string candidate = result + comp;
            if (!Exists(candidate)) {
                std::string dirPath = result.empty() ? std::string(".") : result;
                DIR* dir = opendir(dirPath.c_str());
                bool found = false;
                if (dir != NULL) {
                    while (struct dirent* de = readdir(dir)) {
                        if (EqualNoCase(de->d_name, comp.c_str())) {
                            name = de->d_name;
                            found = true;
                            break;
                        }
                    }
                    closedir(dir);
                }
                if (!found) {
                    missing = true;
                }
            }
        }

        result += name;
        if (!last) {
            result += '/';
        }
    }

    // A trailing slash is kept.
    if (!s.empty() && s[s.size() - 1] == '/' && (result.empty() || result[result.size() - 1] != '/')) {
        result += '/';
    }

    return result;
}

std::string TranslatePath(const char* winPath)
{
    if (winPath == NULL) {
        return std::string();
    }

    std::string s(winPath);
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\\') {
            s[i] = '/';
        }
    }

    if (s.size() >= 2 && s[1] == ':'
        && ((s[0] >= 'A' && s[0] <= 'Z') || (s[0] >= 'a' && s[0] <= 'z'))) {
        std::string rest = s.substr(2);
        if (!rest.empty() && rest[0] == '/') {
            std::string resolved = ResolveCase(rest);
            if (Exists(resolved)) {
                return resolved;
            }
        }
        // Kept as is (a relative `C:foo` or a path that does not exist here).
        return ResolveCase(s);
    }

    return ResolveCase(s);
}

std::string AbsolutePath(const char* path)
{
    std::string s(path != NULL ? path : "");
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '\\') {
            s[i] = '/';
        }
    }

    std::string prefix;
    if (s.size() >= 2 && s[1] == ':') {
        prefix = s.substr(0, 2);
        s = s.substr(2);
    }

    if (s.empty() || s[0] != '/') {
        if (prefix.empty()) {
            std::string cwd = CurrentDirectory();
            if (cwd.empty()) {
                cwd = "/";
            }
            s = cwd + "/" + s;
        } else {
            s = "/" + s;
        }
    }

    std::vector<std::string> parts;
    size_t pos = 0;
    while (pos <= s.size()) {
        size_t end = s.find('/', pos);
        if (end == std::string::npos) end = s.size();
        std::string comp = s.substr(pos, end - pos);
        pos = end + 1;
        if (comp.empty() || comp == ".") {
            continue;
        }
        if (comp == "..") {
            if (!parts.empty()) parts.pop_back();
            continue;
        }
        parts.push_back(comp);
    }

    std::string out = prefix;
    for (size_t i = 0; i < parts.size(); i++) {
        out += "/";
        out += parts[i];
    }
    if (parts.empty()) {
        out += "/";
    }
    return out;
}

} // namespace platform
