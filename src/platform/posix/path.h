#ifndef IWD2RE_PLATFORM_POSIX_PATH_H_
#define IWD2RE_PLATFORM_POSIX_PATH_H_

#include <string>

namespace platform {

// Turns a path as the game writes it (backslashes, optional drive letter,
// any letter case) into a path that exists on a case-sensitive POSIX volume.
//
// - `\` becomes `/`.
// - A leading drive letter `X:` is dropped if what follows it exists as an
//   absolute path; otherwise it is kept as given.
// - Each component that does not exist as written is looked up in its parent
//   directory ignoring ASCII case. Components that do not exist at all are
//   kept as given, so new files can be created.
std::string TranslatePath(const char* winPath);

// Lexical absolute path: relative paths get the current directory in front,
// `.` and `..` are folded, both slashes accepted, `/` is used in the result.
std::string AbsolutePath(const char* path);

// The current directory, named the way the program named it (no symbolic
// links resolved, unlike getcwd on macOS), and chdir that keeps that name.
std::string CurrentDirectory();
int ChangeDirectory(const std::string& posixPath);

} // namespace platform

#endif /* IWD2RE_PLATFORM_POSIX_PATH_H_ */
