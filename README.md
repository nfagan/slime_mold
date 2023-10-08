# slime mold

This repository is a first-exploration of [WebGPU](https://developer.chrome.com/blog/webgpu-cross-platform/); it is a port of the slime mold simulation from [grove](https://github.com/nfagan/grove-public) and builds directly off of the emscripten IMGUI [example](https://github.com/ocornut/imgui/tree/master/examples/example_emscripten_wgpu).

The ideas behind the particle simulation are detailed in the following references. 
* [1] https://sagejenson.com/physarum
* [2] https://uwe-repository.worktribe.com/output/980579
* [3] https://www.youtube.com/watch?v=X-iSQQgOd1A&t=923s

# demo

TODO

# build

Note that the following assumes access to CLion.

1. Download and install [emscripten](https://emscripten.org/index.html), following the instructions for your platform.
2. Make sure that python is accessible on the system path. 
   1. On Windows, start a command prompt: hold `win + r` and type `cmd.exe`. Type `python` and hit enter; if this brings up a prompt to install Python, do so (otherwise, you should be dropped into the Python interpreter, which you can close out of).
3. Clone this repository: `git clone https://github.com/nfagan/slime_mold`
4. Open CLion and select file -> open, then select this repository folder.
5. Click the "Debug" button and select "Edit CMake Profiles". Click the "+" button several times until a profile named "Default" appears. Edit this profile as follows:
   1. Change the name to "web" (actually, you can name it whatever you want).
   2. Locate the path to the emscripten repository you installed earlier. Copy the full path to the `Emscripten.cmake` toolchain file within the repository. This should be located in `/path/to/emsdk/upstream/emscripten/Modules/Platform`
   2. In the cmake options field, enter the following (replacing the path with the one you just copied).

```
-DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/path/to/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
```

6. Click OK. Then activate the "web" cmake profile that you just created and build the program.
7. Open an [emsdk command prompt](https://emscripten.org/docs/getting_started/Tutorial.html#general-tips-and-next-steps). Navigate to the repository folder and run `emrun --browser chrome index.html`. 
