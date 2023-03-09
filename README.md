# vtkWasmBenchmark
[![Deploy WebAssembly binary](https://github.com/jspanchu/vtkWasmBenchmark/actions/workflows/build-wasm.yml/badge.svg)](https://github.com/jspanchu/vtkDearImGUIInjector/actions/workflows/build-wasm.yml)

Benchmarks VTK OpenGL 3D renderer in a browser through WebAssembly. The docker container [jspanchu/vtk-wasm-target](https://hub.docker.com/r/jspanchu/vtk-wasm-target) is used to provide
VTK over WASM. The C++ code can also be compiled as a native desktop application.

The benchmark is running live at https://code.thepanchu.in/vtkWasmBenchmark/

Simple VTK meshes are rendered with `vtkCompositePolyDataMapper2`. The composite polydata mapper has recently implemented
workarounds [vtk/vtk!9916](https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9916) for lack of `PrimitiveID` and `TextureBuffer`
among other things. Features such as hardware picking, surface plus edges,
vertex visibility and flat shading were previously unsupported by the mapper on WebGL. This quick and simple application demonstrates
all those features.

## Compile for WebAssembly (Debug)
```
$ emcmake cmake -GNinja -S src -B build-emscripten -DCMAKE_BUILD_TYPE=Debug -DDEBUGINFO=DEBUG_NATIVE -DOPTIMIZE=NO_OPTIMIZATION -DVTK_DIR=/path/to/vtk/build-em-debug
$ npm run start
```

## Compile for WebAssembly (Release)
```
$ emcmake cmake -GNinja -S src -B build-emscripten -DCMAKE_BUILD_TYPE=Release -DDEBUGINFO=PROFILE -DOPTIMIZE=SMALLEST_WITH_CLOSURE -DVTK_DIR=/path/to/vtk/build-em-release
$ npm run start
```

## Compile for desktop
```
$ cmake -GNinja -S . -B build -DVTK_DIR=/path/to/vtk/build
$ cd build
$ ninja
$ ./vtkwasmbenchmark --help
Usage: ./vtkwasmbenchmark OPTIONS
        -nx <number of objects along X direction> 
        -ny <number of objects along Y direction> 
        -r <representation: 0-Points, 1-Wireframe, 2-Surface, 3-Surface with edges> 
        -lw <line width> 
        -ps <point size> 
        --area-pick [Enables area picker. 'r' toggles rubberband]
        --hover-preselect [Highlights a mesh when mouse hovers above it]
```
