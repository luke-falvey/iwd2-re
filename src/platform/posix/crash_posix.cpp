// The crash handler for non-Windows builds (src/support/CrashHandler.cpp
// uses dbghelp and is not built here).
//
// On a fatal signal the stack of the faulting thread is written to stderr
// and to iwd2re-stacks.txt in the current directory, as lines of
// "index module address symbol + offset". Resolve them to source lines with
// `atos -o <exe> -l <load address> <addresses>`; the load address is the
// first line of the file.

#include <execinfo.h>
#include <fcntl.h>
#include <mach-o/dyld.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void InstallCrashHandler();

namespace {

void Write(int fd, const char* text)
{
    (void)write(fd, text, strlen(text));
}

void OnFatalSignal(int sig, siginfo_t* info, void*)
{
    void* frames[64];
    int count = backtrace(frames, 64);

    char head[160];
    snprintf(head, sizeof(head), "\n*** fatal signal %d (%s), fault address %p, image base %p\n",
        sig, strsignal(sig), info != NULL ? info->si_addr : NULL,
        static_cast<const void*>(_dyld_get_image_header(0)));

    Write(STDERR_FILENO, head);
    backtrace_symbols_fd(frames, count, STDERR_FILENO);

    int fd = open("iwd2re-stacks.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd >= 0) {
        Write(fd, head);
        backtrace_symbols_fd(frames, count, fd);
        close(fd);
    }

    signal(sig, SIG_DFL);
    raise(sig);
}

} // namespace

void InstallCrashHandler()
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = OnFatalSignal;
    action.sa_flags = SA_SIGINFO | SA_RESETHAND;
    sigemptyset(&action.sa_mask);

    static const int signals[] = { SIGSEGV, SIGBUS, SIGILL, SIGFPE, SIGABRT, SIGTRAP };
    for (size_t k = 0; k < sizeof(signals) / sizeof(signals[0]); k++) {
        sigaction(signals[k], &action, NULL);
    }
}
