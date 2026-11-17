// Private profile (.ini) files with the rules of the Windows functions, and
// the registry (which does not exist here).

#include "path.h"
#include "platform.h"

#include <win32shim/winbase_file.h>
#include <win32shim/winbase_ini_registry.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace {

bool IsSpace(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f';
}

std::string Trim(const std::string& s)
{
    size_t b = 0, e = s.size();
    while (b < e && IsSpace(s[b])) b++;
    while (e > b && IsSpace(s[e - 1])) e--;
    return s.substr(b, e - b);
}

bool EqualNoCase(const std::string& a, const std::string& b)
{
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); i++) {
        char x = a[i], y = b[i];
        if (x >= 'A' && x <= 'Z') x = static_cast<char>(x + 32);
        if (y >= 'A' && y <= 'Z') y = static_cast<char>(y + 32);
        if (x != y) return false;
    }
    return true;
}

// One line of the file, kept as it was (without its terminator).
struct Line {
    std::string text;
    std::string eol; // "\r\n", "\n" or "" for an unterminated last line
};

struct IniFile {
    std::vector<Line> lines;
};

// Classification of a line.
enum LineKind { L_OTHER, L_SECTION, L_KEY };

LineKind Classify(const std::string& text, std::string* name, std::string* value)
{
    std::string t = Trim(text);
    if (t.empty() || t[0] == ';') {
        return L_OTHER;
    }
    if (t[0] == '[') {
        size_t close = t.find(']');
        if (close == std::string::npos) {
            return L_OTHER;
        }
        *name = t.substr(1, close - 1);
        return L_SECTION;
    }
    size_t eq = t.find('=');
    if (eq == std::string::npos) {
        return L_OTHER;
    }
    *name = Trim(t.substr(0, eq));
    std::string v = Trim(t.substr(eq + 1));
    if (v.size() >= 2 && (v[0] == '"' || v[0] == '\'') && v[v.size() - 1] == v[0]) {
        v = v.substr(1, v.size() - 2);
    }
    *value = v;
    return L_KEY;
}

void Parse(const std::string& data, IniFile* file)
{
    file->lines.clear();
    size_t pos = 0;
    while (pos < data.size()) {
        size_t nl = data.find('\n', pos);
        Line line;
        if (nl == std::string::npos) {
            line.text = data.substr(pos);
            pos = data.size();
        } else {
            line.text = data.substr(pos, nl - pos);
            line.eol = "\n";
            pos = nl + 1;
            if (!line.text.empty() && line.text[line.text.size() - 1] == '\r') {
                line.text.erase(line.text.size() - 1);
                line.eol = "\r\n";
            }
        }
        file->lines.push_back(line);
    }
}

std::string Serialize(const IniFile& file)
{
    std::string out;
    for (size_t i = 0; i < file.lines.size(); i++) {
        out += file.lines[i].text;
        out += file.lines[i].eol;
    }
    return out;
}

// --- file cache: contents keyed by path, checked against mtime and size ---

struct CacheEntry {
    bool valid;
    int64_t mtimeSec;
    int64_t mtimeNsec;
    int64_t size;
    std::string data;
    CacheEntry() : valid(false), mtimeSec(0), mtimeNsec(0), size(-1) {}
};

std::mutex g_iniMutex;
std::map<std::string, CacheEntry> g_iniCache;

bool ReadWhole(const std::string& path, std::string* data)
{
    int fd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        return false;
    }
    data->clear();
    char buffer[8192];
    for (;;) {
        ssize_t n = read(fd, buffer, sizeof(buffer));
        if (n < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return false;
        }
        if (n == 0) break;
        data->append(buffer, static_cast<size_t>(n));
    }
    close(fd);
    return true;
}

// Loads the file through the cache. Caller holds g_iniMutex.
bool LoadFile(const std::string& path, IniFile* file)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        g_iniCache.erase(path);
        file->lines.clear();
        return false;
    }
    CacheEntry& entry = g_iniCache[path];
    if (!entry.valid || entry.mtimeSec != st.st_mtimespec.tv_sec || entry.mtimeNsec != st.st_mtimespec.tv_nsec
        || entry.size != static_cast<int64_t>(st.st_size)) {
        if (!ReadWhole(path, &entry.data)) {
            g_iniCache.erase(path);
            file->lines.clear();
            return false;
        }
        entry.valid = true;
        entry.mtimeSec = st.st_mtimespec.tv_sec;
        entry.mtimeNsec = st.st_mtimespec.tv_nsec;
        entry.size = static_cast<int64_t>(st.st_size);
    }
    Parse(entry.data, file);
    return true;
}

bool SaveFile(const std::string& path, const IniFile& file)
{
    std::string data = Serialize(file);
    g_iniCache.erase(path); // invalidated by the write
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0666);
    if (fd < 0) {
        platform::SetLastErrorFromErrno();
        return false;
    }
    size_t done = 0;
    while (done < data.size()) {
        ssize_t n = write(fd, data.data() + done, data.size() - done);
        if (n < 0) {
            if (errno == EINTR) continue;
            platform::SetLastErrorFromErrno();
            close(fd);
            return false;
        }
        done += static_cast<size_t>(n);
    }
    close(fd);
    return true;
}

// Finds the value of `key` in `section` (first occurrence wins).
bool FindValue(const IniFile& file, const std::string& section, const std::string& key, std::string* value)
{
    bool inSection = false;
    for (size_t i = 0; i < file.lines.size(); i++) {
        std::string name, v;
        LineKind kind = Classify(file.lines[i].text, &name, &v);
        if (kind == L_SECTION) {
            inSection = EqualNoCase(name, section);
        } else if (kind == L_KEY && inSection && EqualNoCase(name, key)) {
            *value = v;
            return true;
        }
    }
    return false;
}

// Copies a NUL-separated, double-NUL-terminated list. Returns the number of
// characters copied, not counting the final NUL, like Windows.
DWORD CopyList(const std::vector<std::string>& items, LPSTR buffer, DWORD size)
{
    if (size == 0) {
        return 0;
    }
    if (size == 1) {
        buffer[0] = 0;
        return 0;
    }
    DWORD pos = 0;
    for (size_t i = 0; i < items.size(); i++) {
        const std::string& s = items[i];
        if (pos + s.size() + 2 > size) {
            // Truncated: as much as fits, and the list ends.
            DWORD room = size - 2 - pos;
            memcpy(buffer + pos, s.data(), room);
            pos += room;
            buffer[pos] = 0;
            buffer[pos + 1] = 0;
            return size - 2;
        }
        memcpy(buffer + pos, s.data(), s.size());
        pos += static_cast<DWORD>(s.size());
        buffer[pos++] = 0;
    }
    if (items.empty()) {
        buffer[0] = 0;
        buffer[1] = 0;
        return 0;
    }
    buffer[pos] = 0;
    return pos;
}

DWORD CopyString(const std::string& s, LPSTR buffer, DWORD size)
{
    if (size == 0) {
        return 0;
    }
    size_t n = s.size();
    if (n > size - 1) {
        n = size - 1;
    }
    memcpy(buffer, s.data(), n);
    buffer[n] = 0;
    return static_cast<DWORD>(n);
}

} // namespace

DWORD GetPrivateProfileStringA(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName)
{
    if (lpReturnedString == NULL || nSize == 0) {
        return 0;
    }

    IniFile file;
    {
        std::lock_guard<std::mutex> lock(g_iniMutex);
        LoadFile(platform::TranslatePath(lpFileName), &file);
    }

    if (lpAppName == NULL) {
        std::vector<std::string> names;
        for (size_t i = 0; i < file.lines.size(); i++) {
            std::string name, v;
            if (Classify(file.lines[i].text, &name, &v) == L_SECTION) {
                names.push_back(name);
            }
        }
        return CopyList(names, lpReturnedString, nSize);
    }

    std::string section(lpAppName);

    if (lpKeyName == NULL) {
        std::vector<std::string> keys;
        bool inSection = false;
        for (size_t i = 0; i < file.lines.size(); i++) {
            std::string name, v;
            LineKind kind = Classify(file.lines[i].text, &name, &v);
            if (kind == L_SECTION) {
                inSection = EqualNoCase(name, section);
            } else if (kind == L_KEY && inSection) {
                keys.push_back(name);
            }
        }
        return CopyList(keys, lpReturnedString, nSize);
    }

    std::string value;
    if (FindValue(file, section, lpKeyName, &value)) {
        return CopyString(value, lpReturnedString, nSize);
    }
    return CopyString(lpDefault != NULL ? lpDefault : "", lpReturnedString, nSize);
}

UINT GetPrivateProfileIntA(LPCSTR lpAppName, LPCSTR lpKeyName, INT nDefault, LPCSTR lpFileName)
{
    if (lpAppName == NULL || lpKeyName == NULL) {
        return static_cast<UINT>(nDefault);
    }

    IniFile file;
    std::string value;
    bool found;
    {
        std::lock_guard<std::mutex> lock(g_iniMutex);
        LoadFile(platform::TranslatePath(lpFileName), &file);
        found = FindValue(file, lpAppName, lpKeyName, &value);
    }
    // A missing key, or one with an empty value, gives the default.
    if (!found || value.empty()) {
        return static_cast<UINT>(nDefault);
    }

    const char* p = value.c_str();
    while (IsSpace(*p)) p++;
    bool negative = false;
    if (*p == '-') {
        negative = true;
        p++;
    } else if (*p == '+') {
        p++;
    }
    uint32_t result = 0; // wraps as an unsigned 32-bit value
    while (*p >= '0' && *p <= '9') {
        result = result * 10u + static_cast<uint32_t>(*p - '0');
        p++;
    }
    return negative ? static_cast<UINT>(0u - result) : result;
}

BOOL WritePrivateProfileStringA(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString, LPCSTR lpFileName)
{
    if (lpAppName == NULL || lpFileName == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    std::string path = platform::TranslatePath(lpFileName);
    std::lock_guard<std::mutex> lock(g_iniMutex);

    IniFile file;
    bool existed = LoadFile(path, &file);
    std::string section(lpAppName);

    // Line ending of the file; new files and lines use \r\n.
    const char* eol = "\r\n";

    // Locate the section: first line index after the header, and the end.
    size_t sectionHeader = std::string::npos;
    size_t sectionEnd = file.lines.size(); // one past the last line of the section
    {
        bool inSection = false;
        for (size_t i = 0; i < file.lines.size(); i++) {
            std::string name, v;
            LineKind kind = Classify(file.lines[i].text, &name, &v);
            if (kind == L_SECTION) {
                if (inSection) {
                    sectionEnd = i;
                    break;
                }
                if (EqualNoCase(name, section)) {
                    inSection = true;
                    sectionHeader = i;
                }
            }
        }
        if (!inSection) {
            sectionEnd = file.lines.size();
        }
    }

    if (lpKeyName == NULL) {
        // Delete the section.
        if (sectionHeader == std::string::npos) {
            return TRUE;
        }
        file.lines.erase(file.lines.begin() + static_cast<std::ptrdiff_t>(sectionHeader),
            file.lines.begin() + static_cast<std::ptrdiff_t>(sectionEnd));
        return SaveFile(path, file) ? TRUE : FALSE;
    }

    if (sectionHeader != std::string::npos) {
        // Find the first matching key in the section.
        for (size_t i = sectionHeader + 1; i < sectionEnd; i++) {
            std::string name, v;
            if (Classify(file.lines[i].text, &name, &v) == L_KEY && EqualNoCase(name, lpKeyName)) {
                if (lpString == NULL) {
                    file.lines.erase(file.lines.begin() + static_cast<std::ptrdiff_t>(i));
                } else {
                    file.lines[i].text = std::string(lpKeyName) + "=" + lpString;
                    file.lines[i].eol = eol;
                }
                return SaveFile(path, file) ? TRUE : FALSE;
            }
        }
        if (lpString == NULL) {
            return TRUE; // nothing to delete
        }
        // Append at the end of the section (after its last key line, before
        // trailing blank lines).
        size_t at = sectionEnd;
        while (at > sectionHeader + 1 && Trim(file.lines[at - 1].text).empty()) {
            at--;
        }
        // The line before must end with a line terminator.
        if (at > 0 && file.lines[at - 1].eol.empty()) {
            file.lines[at - 1].eol = eol;
        }
        Line line;
        line.text = std::string(lpKeyName) + "=" + lpString;
        line.eol = eol;
        file.lines.insert(file.lines.begin() + static_cast<std::ptrdiff_t>(at), line);
        return SaveFile(path, file) ? TRUE : FALSE;
    }

    if (lpString == NULL) {
        return TRUE;
    }

    // New section at the end of the file.
    if (!file.lines.empty() && file.lines.back().eol.empty()) {
        file.lines.back().eol = eol;
    }
    Line header;
    header.text = "[" + section + "]";
    header.eol = eol;
    file.lines.push_back(header);
    Line line;
    line.text = std::string(lpKeyName) + "=" + lpString;
    line.eol = eol;
    file.lines.push_back(line);
    (void)existed;
    return SaveFile(path, file) ? TRUE : FALSE;
}

// ---------------------------------------------------------------------------
// Registry: there is none. Only CNetwork's modem lookup uses it and it copes
// with the failure.

LONG RegOpenKeyExA(HKEY, LPCSTR, DWORD, DWORD, PHKEY phkResult)
{
    if (phkResult != NULL) {
        *phkResult = NULL;
    }
    return ERROR_FILE_NOT_FOUND;
}

LONG RegCloseKey(HKEY)
{
    return ERROR_INVALID_HANDLE;
}

LONG RegQueryInfoKeyA(HKEY, LPSTR, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, LPDWORD, PFILETIME)
{
    return ERROR_INVALID_HANDLE;
}
