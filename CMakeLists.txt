cmake_minimum_required(VERSION 3.25)
project(slime_mold)

#   1.  edit cmake profiles -> -DCMAKE_TOOLCHAIN_FILE=/path/to/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
#   2.  in the terminal, from the repo root, emrun --browser chrome index.html
#
#   https://stackoverflow.com/questions/51868832/integrate-emscripten-in-clion
#   https://github.com/emscripten-core/emscripten/issues/11154

set(CMAKE_CXX_STANDARD 20)

add_executable(slime_mold
        main.cpp
        slime_mold.cpp
        random.cpp
        Soil.cpp
        SoilComponent.cpp
        SoilGUI.cpp
        wgpu_imshow.cpp
        #   imgui
        deps/imgui/imgui.cpp
        deps/imgui/imgui_demo.cpp
        deps/imgui/imgui_draw.cpp
        deps/imgui/imgui_tables.cpp
        deps/imgui/imgui_widgets.cpp
        deps/imgui/backends/imgui_impl_glfw.cpp
        deps/imgui/backends/imgui_impl_wgpu.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
        deps/imgui
        deps/imgui/backends
)

set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-s USE_GLFW=3 -s USE_WEBGPU=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=0 -s ASSERTIONS=1 --no-heap-copy --preload-file ${CMAKE_SOURCE_DIR}/deps/imgui/misc/fonts@/fonts")
