cmake_minimum_required(VERSION 3.23)
project(GaussianSplattingGL LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/W4 /permissive- /utf-8)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(OpenGL REQUIRED)

set(LIBS_DIR "${CMAKE_SOURCE_DIR}/libs")
set(GLFW_INCLUDE_DIR "${LIBS_DIR}/GLFW/include")
set(GLFW_LIB_DIR "${LIBS_DIR}/GLFW/lib")
set(GLAD_INCLUDE_DIR "${LIBS_DIR}/GLAD/include")
set(GLAD_SOURCE "${LIBS_DIR}/GLAD/src/glad.c")
set(GLM_INCLUDE_DIR "${LIBS_DIR}/glm")
set(STB_INCLUDE_DIR "${LIBS_DIR}/stb")

if(NOT EXISTS "${GLAD_SOURCE}")
  message(FATAL_ERROR "Missing GLAD source: ${GLAD_SOURCE}")
endif()

if(NOT EXISTS "${GLFW_INCLUDE_DIR}/GLFW/glfw3.h")
  message(FATAL_ERROR "Missing GLFW headers under: ${GLFW_INCLUDE_DIR}")
endif()

if(NOT EXISTS "${GLFW_LIB_DIR}/glfw3.lib")
  message(FATAL_ERROR "Missing GLFW import/static library: ${GLFW_LIB_DIR}/glfw3.lib")
endif()

add_executable(gaussian_splatting_gl
  ${GLAD_SOURCE}
  src/main.cpp
  
  src/core/ShaderProgram.cpp
  src/cache/SplatCacheReader.cpp
  src/runtime/RuntimeSplatAssetLoader.cpp
  src/scene/Camera.cpp
  src/render/GpuUploadBuffers.cpp
  src/render/ChunkSchedulerPipeline.cpp
  src/render/ScheduleCompactionPipeline.cpp
  src/render/ScheduleSortInitPipeline.cpp
  src/render/ViewDataPipeline.cpp
  src/render/GaussianRenderer.cpp
)

add_executable(gs_cache_builder
  src/import/PlySceneImporter.cpp
  src/io/PlyLoader.cpp
  src/render/GpuSplatPacking.cpp
  src/cache/SplatCacheWriter.cpp
  src/cache/SplatCacheReader.cpp
  src/runtime/RuntimeSplatAssetLoader.cpp
  tools/gs_cache_builder/main.cpp
)

target_include_directories(gaussian_splatting_gl PRIVATE
  src
  "${GLAD_INCLUDE_DIR}"
  "${GLFW_INCLUDE_DIR}"
  "${GLM_INCLUDE_DIR}"
  "${STB_INCLUDE_DIR}"
)

target_link_libraries(gaussian_splatting_gl
  PRIVATE
    OpenGL::GL
    "${GLFW_LIB_DIR}/glfw3.lib"
    user32
    gdi32
    shell32
    advapi32
    winmm
    ole32
)

target_include_directories(gs_cache_builder PRIVATE
  src
  "${GLM_INCLUDE_DIR}"
)

if(EXISTS "${GLFW_LIB_DIR}/glfw3.dll")
  add_custom_command(TARGET gaussian_splatting_gl POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${GLFW_LIB_DIR}/glfw3.dll"
    "$<TARGET_FILE_DIR:gaussian_splatting_gl>/glfw3.dll"
  )
endif()

add_custom_command(TARGET gaussian_splatting_gl POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_SOURCE_DIR}/assets"
  "$<TARGET_FILE_DIR:gaussian_splatting_gl>/assets"
)

# =============================================================================
# Test Infrastructure (optional, for validation of upcoming changes)
# =============================================================================
option(BUILD_TESTS "Build unit tests for validation" OFF)

if(BUILD_TESTS)
  message(STATUS "Building tests")
  
  # Test: Chunk Scheduler (Change 1: Tunable thresholds)
  add_executable(chunk_scheduler_test
    tests/ChunkSchedulerTest.cpp
  )
  target_include_directories(chunk_scheduler_test PRIVATE 
    src
    tests
    "${GLM_INCLUDE_DIR}"
  )
  
  # Test: Chunk Dispatch (Change 3: Chunk-driven dispatch)
  add_executable(chunk_dispatch_test
    tests/ChunkDispatchTest.cpp
  )
  target_include_directories(chunk_dispatch_test PRIVATE 
    src
    tests
    "${GLM_INCLUDE_DIR}"
  )
  
  # More tests can be added here as needed
  
endif()
