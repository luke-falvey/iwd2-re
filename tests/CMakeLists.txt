# T1/T2 unit tests. Link the game library and run without game data.

# Layout test: generated from header offset annotations at build time.
find_package(Python3 COMPONENTS Interpreter REQUIRED)
file(GLOB_RECURSE IWD2RE_HEADERS CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.h")
set(LAYOUT_TEST "${CMAKE_CURRENT_BINARY_DIR}/test_layout.cpp")
add_custom_command(
    OUTPUT "${LAYOUT_TEST}"
    COMMAND Python3::Interpreter -I "${PROJECT_SOURCE_DIR}/tools/gen_layout_asserts.py" --out "${LAYOUT_TEST}"
    DEPENDS
        "${PROJECT_SOURCE_DIR}/tools/gen_layout_asserts.py"
        "${CMAKE_CURRENT_SOURCE_DIR}/layout/known_mismatches.txt"
        ${IWD2RE_HEADERS}
    COMMENT "Generating layout test"
)

add_executable(iwd2-tests
    "unit/main.cpp"
    "unit/test.h"
    "unit/test_resref.cpp"
    "unit/test_vidpalette.cpp"
    "unit/test_aiscript.cpp"
    "unit/test_compat_string.cpp"
    "unit/test_compat_geometry.cpp"
    "unit/test_compat_collections.cpp"
    "unit/test_compat_file.cpp"
    "${LAYOUT_TEST}"
)
if(WIN32)
    target_sources(iwd2-tests PRIVATE "unit/test_strings.rc")
endif()
target_include_directories(iwd2-tests PRIVATE "${PROJECT_SOURCE_DIR}/third_party/doctest" "unit")
target_link_libraries(iwd2-tests PRIVATE iwd2-core)
if(WIN32)
    target_compile_options(iwd2-tests PRIVATE /bigobj)
else()
    # Port: test_strings.rc is Windows only. Object-only copy for compile-all.
    get_target_property(IWD2RE_TEST_SOURCES iwd2-tests SOURCES)
    add_library(iwd2-tests-objs OBJECT ${IWD2RE_TEST_SOURCES})
    target_include_directories(iwd2-tests-objs PRIVATE "${PROJECT_SOURCE_DIR}/third_party/doctest" "unit")
    target_link_libraries(iwd2-tests-objs PRIVATE iwd2-core)
endif()
add_test(NAME unit COMMAND iwd2-tests)

# The original was built with 2-byte struct packing (annotated offsets put
# ints on 2-byte boundaries). Measure offsets the same way. MFC and Windows
# headers set their own packing, so their types are unaffected.
# (The 2-byte packing now comes from pack_prefix.h, force-included through
# iwd2-core for every source file, tests included.)
