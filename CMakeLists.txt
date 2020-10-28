cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
set(NAME OptiX-test)
project(${NAME})

if(POLICY CMP0077)  # see notes/issues/cmake-3.13.4-FindCUDA-warnings.rst
    cmake_policy(SET CMP0077 OLD)
endif()

set(CMAKE_CXX_STANDARD 14) 
set(CMAKE_CXX_STANDARD_REQUIRED on) 

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" "${OptiX_INSTALL_DIR}/SDK/CMake")
message("CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH})
include("nvcuda_compile_ptx")
configure_file(config.h.in config.h)

# find_package(OpenGL REQUIRED)
# find_package(GLFW REQUIRED)
# find_package(GLEW REQUIRED)

find_package(OptiX REQUIRED)
add_library(ImportOptiX INTERFACE IMPORTED) 
set_target_properties(ImportOptiX PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${OptiX_INCLUDE}"
    INTERFACE_LINK_LIBRARIES "optix;optixu;optix_prime"
)

find_package(CUDA 10.0 REQUIRED) 
add_library(ImportCUDA INTERFACE IMPORTED)
set_target_properties(ImportCUDA  PROPERTIES 
    INTERFACE_INCLUDE_DIRECTORIES "${CUDA_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${CUDA_cudart_static_LIBRARY};${CUDA_curand_LIBRARY}"
)

set(COMPUTE_CAPABILITY "30" CACHE STRING "Compute capability")
message(STATUS "COMPUTE_CAPABILITY = ${COMPUTE_CAPABILITY}")

# set(CUDA_NVCC_FLAGS)
# list(APPEND CUDA_NVCC_FLAGS "-Xcompiler -fPIC")
# list(APPEND CUDA_NVCC_FLAGS "-gencode=arch=compute_${COMPUTE_CAPABILITY},code=sm_${COMPUTE_CAPABILITY}")
# list(APPEND CUDA_NVCC_FLAGS "-O2")
# list(APPEND CUDA_NVCC_FLAGS "--use_fast_math")
# message("CUDA_NVCC_FLAGS: " ${CUDA_NVCC_FLAGS})

set(HEADERS
  include/geo_configure.h
  include/error_check.h
  include/read_PTX.h
)

set(SOURCES
  src/error_check.cc
  src/main.cc
  src/read_PTX.cc
)

set(CUDA_SOURCES
  cuda/point_source.cu
  cuda/simple_dom.cu
  cuda/sphere.cu
)

set(CUDA_HEADERS
  cuda/PerRayData_pathtrace.h
  cuda/random.h
)

NVCUDA_COMPILE_PTX(SOURCES ${CUDA_SOURCES}
                  DEPENDENCIES ${CUDA_HEADERS}
                  TARGET_PATH "${CMAKE_CURRENT_BINARY_DIR}/ptx"
                  GENERATED_FILES PTX_SOURCES
                  NVCC_OPTIONS "--gpu-architecture=compute_${COMPUTE_CAPABILITY}" "--use_fast_math" "--relocatable-device-code=true" "--generate-line-info" "-Wno-deprecated-gpu-targets" "-I${OptiX_INCLUDE}" "-I${CMAKE_CURRENT_SOURCE_DIR}/cuda" "-I${CUDA_INCLUDE_DIRS}"
                  )

include_directories(
  "include"
  ${OptiX_INCLUDE}
  ${PROJECT_BINARY_DIR}
)

message("PTX_SOURCES: " ${PTX_SOURCES})

add_executable(${NAME}
  ${SOURCES}
  ${HEADERS}
  ${CUDA_SOURCES}
  ${CUDA_HEADERS}
  ${PTX_SOURCES}
)

target_link_libraries(${NAME} PUBLIC
  ImportOptiX
  ImportCUDA
)

install(FILES ${PTX_SOURCES} DESTINATION ptx)
install(TARGETS ${name} DESTINATION bin)
