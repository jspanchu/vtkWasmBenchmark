cmake_minimum_required(VERSION 3.16)
project(vtkWasmBenchmark C CXX)

find_package(VTK
COMPONENTS
  FiltersSources
  FiltersGeneral
  RenderingOpenGL2
  RenderingUI
  InteractionStyle
  InteractionWidgets
)

if (NOT VTK_FOUND)
  message("Skipping example: ${VTK_NOT_FOUND_MESSAGE}")
  return ()
endif ()

add_executable(vtkwasmbenchmark 
  main.cpp 
  App.cpp
  HoverPickStyle.cpp
)

target_include_directories(vtkwasmbenchmark
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(vtkwasmbenchmark
  PRIVATE
  ${VTK_LIBRARIES}
)

vtk_module_autoinit(
  TARGETS  vtkwasmbenchmark
  MODULES  ${VTK_LIBRARIES}
)

if (EMSCRIPTEN)

  # -----------------------------------------------------------------------------
  # WebAssembly build options
  # -----------------------------------------------------------------------------
  set(emscripten_link_options)
  set(emscripten_compile_options)
  list(APPEND emscripten_compile_options
    "SHELL:-s USE_PTHREADS=0"
  )

  set(emscripten_debug_options)
  set(DEBUGINFO "PROFILE" CACHE STRING "Type of debug info")
  set_property(CACHE DEBUGINFO PROPERTY
    STRINGS
      NONE              # -g0
      READABLE_JS       # -g1
      PROFILE           # -g2
      DEBUG_NATIVE      # -g3
  )

  if(DEBUGINFO STREQUAL "NONE")
    list(APPEND emscripten_debug_options
      "-g0"
    )
  elseif(DEBUGINFO STREQUAL "READABLE_JS")
    list(APPEND emscripten_debug_options
      "-g1"
    )
  elseif(DEBUGINFO STREQUAL "PROFILE")
    list(APPEND emscripten_debug_options
      "-g2"
    )
  elseif(DEBUGINFO STREQUAL "DEBUG_NATIVE")
    list(APPEND emscripten_debug_options
      "-g3"
    )
  endif()

  list(APPEND emscripten_link_options
    "-lembind"
    "SHELL:-s EXPORT_NAME=vtkCPDMApp"
    "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    "SHELL:-s DEMANGLE_SUPPORT=1"
    "SHELL:-s EMULATE_FUNCTION_POINTER_CASTS=0"
    "SHELL:-s ERROR_ON_UNDEFINED_SYMBOLS=0"
    "SHELL:-s MODULARIZE=1"
    "SHELL:-s WASM=1"
  )

  # -----------------------------------------------------------------------------
  # Build options
  # -----------------------------------------------------------------------------
  set(emscripten_optimizations)
  set(OPTIMIZE "SMALLEST_WITH_CLOSURE" CACHE STRING "Emscripten optimization")
  set_property(CACHE OPTIMIZE PROPERTY
    STRINGS
      NO_OPTIMIZATION       # -O0
      LITTLE                # -O1
      MORE                  # -O2
      BEST                  # -O3
      SMALL                 # -Os
      SMALLEST              # -Oz
      SMALLEST_WITH_CLOSURE # -Oz --closure 1
  )

  if(OPTIMIZE STREQUAL "NO_OPTIMIZATION")
    list(APPEND emscripten_optimizations
      "-Oz"
    )
  elseif(OPTIMIZE STREQUAL "LITTLE")
    list(APPEND emscripten_optimizations
      "-O1"
    )
  elseif(OPTIMIZE STREQUAL "MORE")
    list(APPEND emscripten_optimizations
      "-O2"
    )
  elseif(OPTIMIZE STREQUAL "BEST")
    list(APPEND emscripten_optimizations
      "-O3"
    )
  elseif(OPTIMIZE STREQUAL "SMALL")
    list(APPEND emscripten_optimizations
      "-Os"
    )
  elseif(OPTIMIZE STREQUAL "SMALLEST")
    list(APPEND emscripten_optimizations
      "-Oz"
    )
  elseif(OPTIMIZE STREQUAL "SMALLEST_WITH_CLOSURE")
    list(APPEND emscripten_optimizations
      "-Oz"
    )
    list(APPEND emscripten_link_options
      "SHELL:--closure 1"
    )
  endif()

  target_compile_options(vtkwasmbenchmark
    PUBLIC
      ${emscripten_compile_options}
      ${emscripten_optimizations}
      ${emscripten_debug_options}
  )

  target_link_options(vtkwasmbenchmark
    PUBLIC
      ${emscripten_link_options}
      ${emscripten_optimizations}
      ${emscripten_debug_options}
  )

  # -----------------------------------------------------------------------------
  # Copy HTML to build directory
  # -----------------------------------------------------------------------------
  
  add_custom_command(
    TARGET vtkwasmbenchmark
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/index.html"
        "${CMAKE_CURRENT_SOURCE_DIR}/glmetrichooks.js"
        $<TARGET_FILE_DIR:vtkwasmbenchmark>
  )
endif ()