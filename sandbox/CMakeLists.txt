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

target_compile_definitions(sandbox PRIVATE GLFW_INCLUDE_NONE)

target_include_directories(sandbox PRIVATE
    ${PARENT_SRC_DIR}/zfw2/include
    ${PARENT_SRC_DIR}/zfw2_common/include
    ${PARENT_SRC_DIR}/vendor/glad/include
)

target_link_libraries(sandbox PRIVATE glfw zfw2 zfw2_common)

add_dependencies(sandbox zfw2_asset_packer)

add_custom_command(TARGET sandbox POST_BUILD
	COMMAND $<TARGET_FILE:zfw2_asset_packer> ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:sandbox>
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:sandbox>/assets.zfw2dat ${CMAKE_CURRENT_BINARY_DIR}/assets.zfw2dat
)
