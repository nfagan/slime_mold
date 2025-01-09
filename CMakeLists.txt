cmake_minimum_required(VERSION 3.25)
project(slime_mold)

#   1.  edit cmake profiles -> -DCMAKE_TOOLCHAIN_FILE=/path/to/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
#   2.  in the terminal, from the repo root, emrun --browser chrome index.html
#
#   https://stackoverflow.com/questions/51868832/integrate-emscripten-in-clion
#   https://github.com/emscripten-core/emscripten/issues/11154

set(CMAKE_CXX_STANDARD 20)
set(COMMON_SOURCES
        main.cpp
        slime_mold.cpp
        base_math.hpp
        base_math.cpp
        image_manip.cpp
        gui.cpp
        slime_mold_component.cpp
        text_rasterizer.cpp
        font.cpp
        font_env.cpp
        util.cpp
        #   imgui
        deps/imgui/imgui.cpp
        deps/imgui/imgui_demo.cpp
        deps/imgui/imgui_draw.cpp
        deps/imgui/imgui_tables.cpp
        deps/imgui/imgui_widgets.cpp
        deps/imgui/backends/imgui_impl_glfw.cpp
)

set(COMMON_INCLUDES
        deps/imgui
        deps/imgui/backends
        deps/stb
)

#   webgpu / public
add_executable(slime_mold ${COMMON_SOURCES} wgpu_imshow.cpp deps/imgui/backends/imgui_impl_wgpu.cpp)
target_include_directories(slime_mold PRIVATE ${COMMON_INCLUDES})

set_target_properties(slime_mold PROPERTIES LINK_FLAGS "-s USE_GLFW=3 -s USE_WEBGPU=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=0 -s ASSERTIONS=1 --no-heap-copy --preload-file ${CMAKE_SOURCE_DIR}/deps/imgui/misc/fonts@/fonts")
target_compile_definitions(slime_mold PRIVATE SM_IS_WGPU)

#   opengl / local
if (NOT EMSCRIPTEN)
    add_subdirectory(deps/glfw-3.4)
endif()
add_executable(slime_mold_local ${COMMON_SOURCES} opengl_imshow.cpp deps/imgui/backends/imgui_impl_opengl3.cpp deps/glad/src/glad.c)
target_include_directories(slime_mold_local PRIVATE ${COMMON_INCLUDES} deps/glad/include deps/stb)
target_compile_definitions(slime_mold_local PRIVATE SM_IS_OPENGL)
target_link_libraries(slime_mold_local PUBLIC glfw)