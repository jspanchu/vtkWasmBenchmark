# vtkWasmBenchmark
[![Deploy WebAssembly binary](https://github.com/jspanchu/vtkWasmBenchmark/actions/workflows/build-wasm.yml/badge.svg)](https://github.com/jspanchu/vtkDearImGUIInjector/actions/workflows/build-wasm.yml)

This application benchmarks VTK OpenGL 3D renderer in a browser through WebAssembly. The docker container [jspanchu/vtk-wasm-target](https://hub.docker.com/r/jspanchu/vtk-wasm-target) is used to provide
VTK over WASM.

A total of 3072 meshes are rendered with `vtkCompositePolyDataMapper2`. The composite polydata mapper has recently implemented
workarounds [vtk/vtk!9916](https://gitlab.kitware.com/vtk/vtk/-/merge_requests/9916) for lack of `PrimitiveID` and `TextureBuffer`
among other things. Features such as hardware picking, surface plus edges,
vertex visibility and flat shading were previously unsupported by the mapper on WebGL. This quick and simple application demonstrates
all those features.
