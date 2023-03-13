# vtkWasmBenchmark
[![Deploy WebAssembly binary](https://github.com/jspanchu/vtkWasmBenchmark/actions/workflows/build-wasm.yml/badge.svg)](https://github.com/jspanchu/vtkDearImGUIInjector/actions/workflows/build-wasm.yml)

Benchmarks VTK OpenGL 3D renderer in a browser through WebAssembly. The C++ code can also be compiled as a native desktop application.

The benchmark can be viewed at https://code.thepanchu.in/vtkWasmBenchmark/

Simple VTK meshes are rendered with `vtkCompositePolyDataMapper2`. The composite polydata mapper has recently implemented
workarounds [vtk/vtk!9916](https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9916) for lack of `PrimitiveID` and `TextureBuffer`
among other things. Features such as hardware picking, surface plus edges,
vertex visibility and flat shading were previously unsupported by the mapper on WebGL. This quick and simple application demonstrates
all those features.

It is possible to compile with `npm` or use your own `emsdk` installation.

## Compile for WebAssembly (with npm)
```
npm run build-wasm
npm run start
```

## Compile for WebAssembly (with npm and debug mode)
The source files referenced by the debugging symbols are actually from the [kitware/vtk-wasm](https://hub.docker.com/r/kitware/vtk-wasm) docker image. You may want to provide path substitutions if you intend to debug VTK C++ code. Check out this [kitware blog](https://www.kitware.com/how-to-debug-webassembly-pipelines-in-your-web-browser/) on how to set those in your browser.
```
npm run build-wasm:debug
npm run start
```

## Compile for WebAssembly (emsdk Release)
```
$ emcmake cmake -GNinja -S src -B build-emscripten -DCMAKE_BUILD_TYPE=Release -DDEBUGINFO=PROFILE -DOPTIMIZE=SMALLEST_WITH_CLOSURE -DVTK_DIR=/path/to/vtk/build-em-release
$ npm run start
```

## Compile for WebAssembly (emsdk in Debug)
This is the easiest way to debug VTK C++ code as it does not need path substitutions, because the source files that are referenced by the source map exist in the file system.
```
$ emcmake cmake -GNinja -S src -B build-emscripten -DCMAKE_BUILD_TYPE=Debug -DDEBUGINFO=DEBUG_NATIVE -DOPTIMIZE=NO_OPTIMIZATION -DVTK_DIR=/path/to/vtk/build-em-debug
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
