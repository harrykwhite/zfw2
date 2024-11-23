project(zfw2)

find_package(glfw3 CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)

get_filename_component(PARENT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)

add_library(zfw2 STATIC
    src/game.cpp
    src/input.cpp
    src/assets.cpp
    src/graphics.cpp
    src/audio.cpp
    src/scenes.cpp
    src/utils.cpp
    ${PARENT_SRC_DIR}/vendor/glad/src/glad.c

    include/zfw2/game.h
    include/zfw2/input.h
    include/zfw2/assets.h
    include/zfw2/graphics.h
    include/zfw2/audio.h
    include/zfw2/scenes.h
    include/zfw2/utils.h
    ${PARENT_SRC_DIR}/vendor/glad/include/glad/glad.h
    ${PARENT_SRC_DIR}/vendor/glad/include/KHR/khrplatform.h
)

target_compile_definitions(zfw2 PRIVATE GLFW_INCLUDE_NONE)

target_include_directories(zfw2 PRIVATE
	include
	${PARENT_SRC_DIR}/zfw2_common/include
	${PARENT_SRC_DIR}/vendor/glad/include
)

target_link_libraries(zfw2 PRIVATE zfw2_common glfw OpenAL::OpenAL)
