project(zfw2_asset_packer)

find_package(Freetype REQUIRED)

add_executable(zfw2_asset_packer
	src/main.cpp
	src/textures.cpp
	src/shader_progs.cpp
	src/fonts.cpp
	src/sounds_and_music.cpp
	${CMAKE_SOURCE_DIR}/vendor/stb_image/src/stb_image.c

	src/common.h
	${CMAKE_SOURCE_DIR}/vendor/audio_file/include/AudioFile.h
)

target_include_directories(zfw2_asset_packer PRIVATE
	${CMAKE_SOURCE_DIR}/zfw2_common/include
	${CMAKE_SOURCE_DIR}/vendor/stb_image/include
	${CMAKE_SOURCE_DIR}/vendor/nlohmann/include
	${CMAKE_SOURCE_DIR}/vendor/audio_file/include
)

target_link_libraries(zfw2_asset_packer PRIVATE Freetype::Freetype zfw2_common)
