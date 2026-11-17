#ifndef IWD2RE_PLATFORM_POSIX_KOBJECT_H_
#define IWD2RE_PLATFORM_POSIX_KOBJECT_H_

// Kernel objects behind `HANDLE`: every HANDLE the shim hands out points to an
// object derived from KObject. CloseHandle releases one reference.

#include <win32shim/wintypes.h>

#include <atomic>
#include <string>
#include <vector>

namespace platform {

enum KKind { K_EVENT = 1, K_SEMAPHORE, K_MUTEX, K_THREAD, K_FILE, K_FIND, K_PROCESS };

struct KObject {
    static const uint32_t MAGIC = 0x4A42544Bu;

    uint32_t magic;
    KKind kind;
    std::atomic<int> refs;

    explicit KObject(KKind k) : magic(MAGIC), kind(k), refs(1) {}
    virtual ~KObject() { magic = 0; }

    // Waits for the object to become signalled; returns WAIT_OBJECT_0,
    // WAIT_TIMEOUT or WAIT_FAILED.
    virtual DWORD Wait(DWORD milliseconds);
};

// The object behind a handle, or NULL for NULL, INVALID_HANDLE_VALUE, the
// pseudo-handles and anything that is not one of ours.
KObject* ObjectFromHandle(HANDLE h, KKind kind = static_cast<KKind>(0));

void AddRef(KObject* obj);
void Release(KObject* obj);

struct FileObject : KObject {
    int fd;
    bool append;
    // For the share-mode check: which file, and what this handle allows.
    uint64_t dev;
    uint64_t ino;
    bool reads;
    bool writes;
    DWORD shareMode;
    bool registered;
    FileObject(int f) : KObject(K_FILE), fd(f), append(false), dev(0), ino(0), reads(false), writes(false), shareMode(0), registered(false) {}
    ~FileObject();
};

struct FindObject : KObject {
    struct Entry {
        std::string name;
        DWORD attributes;
        uint64_t size;
        uint64_t writeTime; // FILETIME ticks
        uint64_t createTime;
        uint64_t accessTime;
    };
    std::vector<Entry> entries;
    size_t next;
    FindObject() : KObject(K_FIND), next(0) {}
};

// Conversions between Unix time and FILETIME ticks (100 ns since 1601).
uint64_t UnixToFileTime(int64_t seconds, int64_t nanoseconds);

} // namespace platform

#endif /* IWD2RE_PLATFORM_POSIX_KOBJECT_H_ */
