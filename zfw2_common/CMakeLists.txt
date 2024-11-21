project(zfw2_common)

add_library(zfw2_common STATIC
    src/math.cpp

    include/zfw2_common/math.h
)

target_include_directories(zfw2_common PRIVATE include)
