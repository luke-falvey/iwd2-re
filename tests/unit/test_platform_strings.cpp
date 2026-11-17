// The Windows build gets the string table of test_strings.rc from the
// executable's resources; elsewhere LoadStringA reads a table that is filled
// at run time. Same ids and texts as test_strings.rc.

#include "test.h"

#ifndef _WIN32
#include "../../src/platform/posix/platform.h"

namespace {
struct RegisterTestStrings {
    RegisterTestStrings()
    {
        platform::RegisterStringResource(101, "Loaded from the string table.");
        platform::RegisterStringResource(102, "%s has %d items");
    }
} g_registerTestStrings;
} // namespace

TEST_CASE("LoadStringA reads the registered table")
{
    char buffer[64];
    CHECK(LoadStringA(NULL, 101, buffer, sizeof(buffer)) == 29);
    CHECK(std::string(buffer) == "Loaded from the string table.");
    CHECK(LoadStringA(NULL, 54321, buffer, sizeof(buffer)) == 0);
    // Cut to the buffer.
    CHECK(LoadStringA(NULL, 101, buffer, 6) == 5);
    CHECK(std::string(buffer) == "Loade");
}
#endif
