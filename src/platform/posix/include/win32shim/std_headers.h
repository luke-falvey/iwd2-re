#ifndef WIN32SHIM_STD_HEADERS_H_
#define WIN32SHIM_STD_HEADERS_H_

// Port: standard headers that must be included before the game's
// `#pragma pack(2)`. pack_prefix.h includes the system headers with the
// default packing and then switches to 2-byte packing; a standard header that
// is first included after that point lays its types out packed, which does
// not match the libc++ library (misaligned access, a crash in the first
// std::ostringstream). pack_prefix.h lists the ones the original needed; the
// ones below are the others that the game, the shim and the tests include.
// windows.h includes this through crt_msvc.h, which is before the pragma.

#ifdef __cplusplus
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <ciso646>
#include <cctype>
#include <cfloat>
#include <climits>
#include <condition_variable>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <istream>
#include <iterator>
#include <limits>
#include <memory>
#include <mutex>
#include <new>
#include <numeric>
#include <ostream>
#include <queue>
#include <random>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#endif

#endif /* WIN32SHIM_STD_HEADERS_H_ */
