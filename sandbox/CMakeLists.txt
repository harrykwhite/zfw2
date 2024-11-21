cmake_minimum_required(VERSION 3.30)

project(sandbox)

find_package(glfw3 CONFIG REQUIRED)

get_filename_component(PARENT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR} PATH)

add_executable(sandbox
	src/entry.cpp
	src/game.cpp
	src/game.h
	${PARENT_SRC_DIR}/vendor/glad/src/glad.c
)

target_include_directories(sandbox PRIVATE
    ${PARENT_SRC_DIR}/zfw2/include
    ${PARENT_SRC_DIR}/zfw2_common/include
    ${PARENT_SRC_DIR}/vendor/glad/include
)

target_link_libraries(sandbox PRIVATE glfw zfw2 zfw2_common)
