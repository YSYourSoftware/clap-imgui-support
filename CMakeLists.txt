cmake_minimum_required(VERSION 3.17)
project(imgui-clap-support C CXX)

if(NOT IMGUI_SOURCE_DIR)
	message(FATAL "You must define IMGUI_SOURCE_DIR before including these extensions")
endif()

add_library(${PROJECT_NAME} STATIC
	${IMGUI_SOURCE_DIR}/imgui.cpp
	${IMGUI_SOURCE_DIR}/imgui_tables.cpp
	${IMGUI_SOURCE_DIR}/imgui_widgets.cpp
	${IMGUI_SOURCE_DIR}/imgui_draw.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PUBLIC ${IMGUI_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC clap-core)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

option(CLAP_IMGUI_FORCE_METAL OFF "Forces the Metal backend to be built, regardless of the platform.")
option(CLAP_IMGUI_FORCE_DX12 OFF "Forces the DirectX12 backend to be built, regardless of the platform.")
option(CLAP_IMGUI_FORCE_SDL3_OPENGL3 OFF "Forces the SDL3+OpenGL3 backend to be built, regardless of the platform.")

set(DO_PLATFORM_SELECT ON)
if(CLAP_IMGUI_FORCE_METAL OR CLAP_IMGUI_FORCE_DX12 OR CLAP_IMGUI_FORCE_SDL3_OPENGL3)
	set(DO_PLATFORM_SELECT OFF)
endif()

if((APPLE AND DO_PLATFORM_SELECT) OR CLAP_IMGUI_FORCE_METAL)
	target_sources(${PROJECT_NAME} PRIVATE
		src/macos-osxnative-metal.mm
		${IMGUI_SOURCE_DIR}/backends/imgui_impl_metal.mm
	)

	target_link_libraries(${PROJECT_NAME} PUBLIC "-framework CoreFoundation" "-framework GameController" "-framework Cocoa" "-framework Metal" "-framework MetalKit")
elseif((WIN32 AND DO_PLATFORM_SELECT) OR CLAP_IMGUI_FORCE_DX12)
	target_sources(${PROJECT_NAME} PRIVATE
		src/win-dx12.cpp
		${IMGUI_SOURCE_DIR}/backends/imgui_impl_win32.cpp
		${IMGUI_SOURCE_DIR}/backends/imgui_impl_dx12.cpp
	)
else()
	if(NOT SDL_LIBRARY)
		message(FATAL "You must define SDL_LIBRARY before including these extensions")
	endif()

	target_sources(${PROJECT_NAME} PRIVATE
		src/other-sdl3-opengl3.cpp
		${IMGUI_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
		${IMGUI_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
	)

	target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL_LIBRARY})
endif()
