add_library(unicode_utils STATIC "src/unicode_utils.cpp")

target_include_directories (unicode_utils PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(unicode_utils PRIVATE cxx_std_17)
target_compile_definitions(unicode_utils PRIVATE
    "UNICODE;"
    "_UNICODE;"
    )